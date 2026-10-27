#include "TelemetryRuntime.h"
#include "OpenRGBHardwareSyncPlugin.h"
#include "TelemetryMath.h"
#include <QDateTime>
#include <QCoreApplication>
#include <QFile>
#include <QSaveFile>
#include <QDir>
#include <QUuid>
#include <QStandardPaths>
#include <limits>
#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
TelemetryRuntime& telemetryRuntime(){static TelemetryRuntime instance;return instance;}
TelemetryRuntime::TelemetryRuntime(QObject* parent):QObject(parent){
#ifdef _WIN32
 process.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments* a){a->flags|=CREATE_NO_WINDOW;});
#endif
 connect(&process,&QProcess::readyReadStandardOutput,this,[this](){
  buffer+=process.readAllStandardOutput();
  if(buffer.size()>1000000){buffer.clear();collectorError="Collector response too large";process.kill();return;}
  while(buffer.contains('\n')){
   auto line=buffer.left(buffer.indexOf('\n'));buffer.remove(0,line.size()+1);
   try{auto candidate=json::parse(line.constData(),line.constData()+line.size());if(!candidate.is_object()||candidate.value("schema",0)!=1||(!candidate.contains("telemetry")||!candidate["telemetry"].is_object()))throw std::runtime_error("schema");snapshot=candidate["telemetry"];received=QDateTime::currentMSecsSinceEpoch();collectorError="";}
   catch(...){collectorError="Invalid collector response";}
  }
 });
 connect(&process,&QProcess::readyReadStandardError,this,[this](){process.readAllStandardError();});
 connect(&process,&QProcess::errorOccurred,this,[this](QProcess::ProcessError){collectorError="Collector unavailable";});
 connect(&process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int,QProcess::ExitStatus){collectorError="Collector stopped; telemetry frozen";});
 watchdog.setInterval(1000);connect(&watchdog,&QTimer::timeout,this,[this](){
 readControl();
 auto path=configuration.value("status_file",std::string());
 if(!path.empty()){
  auto state=status();state["timestamp"]=QDateTime::currentMSecsSinceEpoch()/1000.0;state["pid"]=QCoreApplication::applicationPid();state["running"]=true;state["backend"]="plugin";state["write_count"]=writes;state["active_writes"]=!paused;state["targets"]=json::array();
  for(const auto& item:states)state["targets"].push_back({{"name",item.first},{"state",item.second}});
  // Telemetry status is durable but must never fsync on the GUI thread.
  // At most one write is in flight; a slow disk drops intermediate samples.
  if(!statusWrite.valid() || statusWrite.wait_for(std::chrono::seconds(0))==std::future_status::ready){
   if(statusWrite.valid())statusWrite.get();
   auto text=state.dump();
   statusWrite=std::async(std::launch::async,[path,text](){
    QSaveFile file(QString::fromStdString(path));
    if(file.open(QIODevice::WriteOnly)){file.write(text.data(),text.size());file.commit();}
   });
  }
 }
});
}
TelemetryRuntime::~TelemetryRuntime(){shutdown();}
void TelemetryRuntime::start(){
 if(configured)return;configured=true;
 try{configuration=OpenRGBHardwareSyncPlugin::api->GetSettings("RGBTelemetryPlugin");}catch(...){configuration=json::object();}
 if(!configuration.is_object())configuration=json::object();
 const auto directory=QString::fromStdString((OpenRGBHardwareSyncPlugin::api->GetConfigurationDirectory()/"HardwareSyncAI").string());
 if(!configuration.contains("control_file")) {
  if(QDir().mkpath(directory)) {
   QFile::setPermissions(directory,QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner);
   const auto path=directory+"/control.json";
   configuration["control_file"]=path.toStdString();
   if(!QFile::exists(path)) {
    QSaveFile initial(path);const auto data=json({{"paused",true},{"generation","fresh-install"},{"reason","Fresh installation; explicit resume required"}}).dump();
    if(initial.open(QIODevice::WriteOnly)){initial.setPermissions(QFile::ReadOwner|QFile::WriteOwner);initial.write(data.data(),data.size());initial.commit();}
   }
  }
 }
 if(!configuration.contains("control_lock"))configuration["control_lock"]=(directory+"/control.lock").toStdString();
 if(!configuration.contains("status_file"))configuration["status_file"]=(directory+"/status.json").toStdString();
 readControl();watchdog.start();
 auto python=QString::fromStdString(configuration.value("collector_python",std::string()));
 auto script=QString::fromStdString(configuration.value("collector_script",std::string()));
 auto library=QString::fromStdString(configuration.value("library_path",std::string()));
 if(configuration.contains("collector_python") || configuration.contains("collector_script") || configuration.contains("library_path")) {
  // Explicit integrations keep their existing settings and fail closed if incomplete.
  if(!QFile::exists(python)||!QFile::exists(script)||!QDir(library).exists()){collectorError="Configured collector paths missing";return;}
  process.start(python,{"-u",script,"--library",library});
 } else {
  python=QStandardPaths::findExecutable("python3");
  QFile source(":/standalone_collector.py");
  if(python.isEmpty()||!source.open(QIODevice::ReadOnly)){collectorError="Install Python 3 for the bundled read-only collector";return;}
  process.start(python,{"-u","-c",QString::fromUtf8(source.readAll())});
 }
}
void TelemetryRuntime::shutdown(){
 watchdog.stop();configured=false;
 if(process.state()!=QProcess::NotRunning){process.write("stop\n");process.waitForBytesWritten(200);if(!process.waitForFinished(5000)){process.kill();process.waitForFinished(1000);}}
 endFrame();
}
double TelemetryRuntime::value(const std::string& key)const{
 const double unknown=std::numeric_limits<double>::quiet_NaN();
 if(!received||QDateTime::currentMSecsSinceEpoch()-received>8000||!snapshot.is_object()||!snapshot.contains(key))return unknown;
 const auto& v=snapshot[key];if(v.is_boolean())return v.get<bool>()?1:0;
 if(!v.is_number())return unknown;
 double number=v.get<double>();return std::isfinite(number)?number:unknown;
}
bool TelemetryRuntime::modelKnown(const std::string& key)const{return std::isfinite(value(key));}
bool TelemetryRuntime::readControl(){
 try{
  const auto path=configuration.value("control_file",std::string());if(path.empty())throw std::runtime_error("No control path");
  QFile file(QString::fromStdString(path));if(!file.open(QIODevice::ReadOnly)||file.size()>65536)throw std::runtime_error("Control unavailable");
  auto bytes=file.readAll();auto state=json::parse(bytes.constData(),bytes.constData()+bytes.size());
  if(!state.contains("paused")||!state["paused"].is_boolean()||!state.contains("generation")||!state["generation"].is_string())throw std::runtime_error("Invalid control state");
  auto gen=state["generation"].get<std::string>();if(gen!=generation){generation=gen;expected.clear();manual.clear();}
  paused=state["paused"].get<bool>();pauseReason=state.value("reason",std::string());return true;
 }catch(...){paused=true;pauseReason="Control unavailable or invalid; writes inhibited";return false;}
}
bool TelemetryRuntime::beginFrame(){
 if(lockHandle)return false;
#ifdef _WIN32
 const auto path=QString::fromStdString(configuration.value("control_lock",std::string()));if(path.isEmpty())return false;
 HANDLE handle=CreateFileW(reinterpret_cast<LPCWSTR>(path.utf16()),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(handle==INVALID_HANDLE_VALUE)return false;
 OVERLAPPED ov={};if(!LockFileEx(handle,LOCKFILE_EXCLUSIVE_LOCK|LOCKFILE_FAIL_IMMEDIATELY,0,1,0,&ov)){CloseHandle(handle);return false;}lockHandle=handle;
#else
 const auto path=configuration.value("control_lock",std::string());if(path.empty())return false;
 int fd=::open(path.c_str(),O_RDWR|O_CREAT|O_CLOEXEC|O_NOFOLLOW,0600);if(fd<0)return false;
 struct flock guard={};guard.l_type=F_WRLCK;guard.l_whence=SEEK_SET;guard.l_len=1;
 if(::fcntl(fd,F_SETLK,&guard)<0){::close(fd);return false;}
 lockHandle=reinterpret_cast<void*>(static_cast<intptr_t>(fd)+1);
#endif
 if(!readControl()){endFrame();return false;}return true;
}
void TelemetryRuntime::endFrame(){
#ifdef _WIN32
 if(lockHandle){OVERLAPPED ov={};UnlockFileEx(static_cast<HANDLE>(lockHandle),0,1,0,&ov);CloseHandle(static_cast<HANDLE>(lockHandle));lockHandle=nullptr;}
#else
 if(lockHandle){int fd=static_cast<int>(reinterpret_cast<intptr_t>(lockHandle)-1);
 struct flock guard={};guard.l_type=F_UNLCK;guard.l_whence=SEEK_SET;guard.l_len=1;
 ::fcntl(fd,F_SETLK,&guard);::close(fd);lockHandle=nullptr;}
#endif
}
bool TelemetryRuntime::setPause(bool pause,const std::string& reason){
 if(!beginFrame())return false;
 json state={{"paused",pause},{"reason",reason},{"generation",QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString()},{"updated",QDateTime::currentMSecsSinceEpoch()/1000.0}};
 QSaveFile file(QString::fromStdString(configuration.value("control_file",std::string())));auto text=state.dump(2);
 bool ok=file.open(QIODevice::WriteOnly)&&file.write(text.data(),text.size())==static_cast<qint64>(text.size())&&file.commit();
 if(ok)readControl();endFrame();return ok;
}
void TelemetryRuntime::clearControllerState(){expected.clear();manual.clear();owners.clear();assignments.clear();}
bool TelemetryRuntime::eligible(RGBControllerInterface* c,bool native){
 if(paused)return false;
 int m=c->GetActiveMode();if(m<0||static_cast<unsigned>(m)>=c->GetModeCount())return false;
 if(c->GetModeName(m)=="Off" || c->GetModeName(m)=="OFF")return false;
 if(!native && !TelemetryMath::supportedMode(c->GetModeName(m),c->GetModeFlags(m)))return false;
 if((c->GetModeFlags(m)&MODE_FLAG_HAS_BRIGHTNESS)&&c->GetModeBrightness(m)==0)return false;
 for(unsigned z=0;z<c->GetZoneCount();z++){bool black=c->GetZoneLEDsCount(z)>0;for(unsigned i=0;i<c->GetZoneLEDsCount(z);i++)if(c->GetZoneColor(z,i)!=0){black=false;break;}if(black){expected.erase(c);return false;}}
 if(manual.count(c))return false;
 std::vector<RGBColor> current;for(unsigned i=0;i<c->GetLEDCount();i++)current.push_back(c->GetColor(i));current.push_back(c->GetActiveMode());current.push_back(c->GetModeBrightness(c->GetActiveMode()));
 auto it=expected.find(c);if(it!=expected.end()&&current!=it->second){manual.insert(c);expected.erase(c);return false;}
 return true;
}
void TelemetryRuntime::remember(RGBControllerInterface* c,bool wrote){auto& colors=expected[c];colors.clear();for(unsigned i=0;i<c->GetLEDCount();i++)colors.push_back(c->GetColor(i));colors.push_back(c->GetActiveMode());colors.push_back(c->GetModeBrightness(c->GetActiveMode()));if(wrote)writes++;}
void TelemetryRuntime::assign(const void* owner,const std::string& name,const std::vector<std::pair<RGBControllerInterface*,unsigned>>& zones){assignments[owner]={name,zones};}
std::string TelemetryRuntime::conflict(RGBControllerInterface* c,unsigned z,const void* owner)const{for(const auto& entry:assignments)if(entry.first!=owner)for(auto zone:entry.second.second)if(zone==std::make_pair(c,z))return entry.second.first;return "";}
bool TelemetryRuntime::claim(RGBControllerInterface* c,unsigned z,const void* owner){return conflict(c,z,owner).empty();}
void TelemetryRuntime::release(const void* owner){assignments.erase(owner);for(auto it=owners.begin();it!=owners.end();)if(it->second==owner)it=owners.erase(it);else ++it;}
std::string TelemetryRuntime::state(const std::string& name)const{auto it=states.find(name);return it==states.end()?"Waiting for first frame":it->second;}
void TelemetryRuntime::record(const std::string& name,const std::string& state){states[name]=state;}
json TelemetryRuntime::status()const{return {{"plugin","Hardware Sync AI"},{"version","1.1.0"},{"paused",paused},{"reason",pauseReason},{"writes",writes},{"collector_error",collectorError},{"telemetry_age_seconds",received?(QDateTime::currentMSecsSinceEpoch()-received)/1000.0:-1.0},{"measures",states},{"physical_verified",false},{"telemetry",snapshot}};}
