#include <QThread>
#include <QElapsedTimer>
#include <QApplication>
#include <QFontDatabase>
#include <QPluginLoader>
#include <QTemporaryDir>
#include <QFile>
#include <QTimer>
#include <QEventLoop>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPixmap>
#include <QTreeWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <iostream>
#include <stdexcept>
#include "TestAPI.h"
#include "ResourceManagerCallback.h"
#include "TestController.h"
#include "TelemetryMath.h"
using json=nlohmann::json;
void need(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void write(const QString& path,const json& value){QFile f(path);need(f.open(QIODevice::WriteOnly|QIODevice::Truncate),"write test data");f.write(value.dump().c_str());}
void wait(int ms){QEventLoop loop;QTimer::singleShot(ms,&loop,&QEventLoop::quit);loop.exec();}
json command(OpenRGBPluginInterface* p,std::string cmd){auto text=json({{"command",cmd}}).dump();unsigned size=text.size();auto result=p->OnSDKCommand(1,(unsigned char*)text.data(),&size);need(result!=nullptr,"SDK result");auto value=json::parse(result,result+size);delete[] result;return value;}
int main(int argc,char** argv){QApplication app(argc,argv);app.setFont(QFont("sans-serif",10));unsigned checks=0;
try{
 need(argc==5,"args DLL python fixtureScript imagePath");
 for(int i=0;i<1000;i++){double v=TelemetryMath::envelope("breathing",i*.01,5,.3);need(v>=.3&&v<=1,"envelope bounds");}checks++;
 need(TelemetryMath::supportedMode("Custom",MODE_FLAG_HAS_PER_LED_COLOR)&&!TelemetryMath::supportedMode("Custom",0)&&!TelemetryMath::supportedMode("Off",0),"mode eligibility");checks++;
 QTemporaryDir temporary;need(temporary.isValid(),"temporary directory");auto dir=temporary.path();auto control=dir+"/control.json";
 write(control,{{"paused",true},{"generation","initial"},{"reason","test"}});
 write(dir+"/fixture.json",{{"gpu_power_percent",50},{"gpu_model",true},{"cpu_model",false},{"ram_model",false}});
 QElapsedTimer heartbeatClock;heartbeatClock.start();qint64 lastHeartbeat=0,maxHeartbeat=0;QTimer heartbeat;QObject::connect(&heartbeat,&QTimer::timeout,[&](){auto now=heartbeatClock.elapsed();if(lastHeartbeat)maxHeartbeat=std::max(maxHeartbeat,now-lastHeartbeat);lastHeartbeat=now;});heartbeat.start(20);
 TestController controller;TestAPI api;
 api.controllers={&controller};
 if(qEnvironmentVariableIsSet("HSAI_ABSENT_START"))api.controllers.clear();
 json zone={{"name",controller.GetName()},{"location",controller.GetLocation()},{"serial",controller.GetSerial()},{"description",controller.GetDescription()},{"version",controller.GetVersion()},{"vendor",controller.GetVendor()},{"zone_idx",1},{"is_segment",false},{"segment_idx",-1},{"brightness",22},{"reverse",false},{"is_percentage_fill",false},{"zone_name","Zone 1"},{"leds_count",2}};
 json measure={{"name","GPU test"},{"hardware_identifier","gpu_power_percent"},{"measure_color",{{"color_stops",{{0,0xFF0000},{100,0x0000FF}}}}},{"devices",{zone}},{"running",true},{"fps",5},{"brightness",100},{"effect",{{"mode","breathing"},{"trigger","gpu_model"},{"period",5},{"floor",.3}}}};
 api.settings={{"status_file",(dir+"/status.json").toStdString()},{"collector_python",argv[2]},{"collector_script",argv[3]},{"library_path",dir.toStdString()},{"control_file",control.toStdString()},{"control_lock",(dir+"/control.lock").toStdString()},{"measures",{measure}}};
 QPluginLoader loader(argv[1]);auto object=loader.instance();need(object!=nullptr,qPrintable(loader.errorString()));auto plugin=qobject_cast<OpenRGBPluginInterface*>(object);need(plugin&&plugin->GetPluginAPIVersion()==5,"plugin API");checks++;
 QThread loaderAffinity;object->moveToThread(&loaderAffinity);
 plugin->Load(&api);auto widget=plugin->GetWidget();need(widget!=nullptr,"widget");widget->resize(1080,800);widget->show();wait(1200);
 if(qEnvironmentVariableIsSet("HSAI_ABSENT_START")) {
   wait(1900);need(api.settings["measures"][0]["devices"].size()==1,"cold start absent target remains persisted");checks++;
   api.controllers={&controller};plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);wait(250);
   need(command(plugin,"status")["status"]["rules"][0]["assignments"][0]["state"]=="resolved","cold start target returns");checks++;
 }
 plugin->OnProfileLoad({{"measures",json::array()}});
 need(command(plugin,"status")["status"]["rules"].size()==1,"startup profile preserves latest settings");checks++;
 need(controller.updates==0&&controller.modeChanges==0,"paused load has zero writes and mode changes");checks++;
 need(widget->findChild<QComboBox*>("effectMode")&&widget->findChild<QDoubleSpinBox*>("effectPeriod"),"effect controls");checks++;
 widget->grab().save(argv[4]);
 need(command(plugin,"resume")["ok"].get<bool>(),"resume");wait(650);need(controller.updates>0,"active rendering");need(controller.modeChanges==0,"no forced direct");checks++;
 auto before=controller.updates;command(plugin,"pause");wait(450);need(controller.updates==before,"pause stops writes");checks++;
 command(plugin,"resume");controller.colors.assign(4,0);before=controller.updates;wait(450);need(controller.updates==before,"black survives resume");checks++;
 controller.colors.assign(4,0xFF00);wait(450);need(controller.updates>before,"external nonblack rearms black protection");checks++;
 controller.colors[0]=0xABCDEF;before=controller.updates;wait(450);need(controller.updates==before,"manual change latches");command(plugin,"resume");wait(450);need(controller.updates>before,"explicit resume clears manual latch");checks++;
 controller.active=1;before=controller.updates;command(plugin,"resume");wait(450);need(controller.updates==before&&controller.active==1,"OFF cannot be overridden");checks++;
 controller.active=3;command(plugin,"resume");wait(450);need(controller.updates>before&&controller.modeChanges==0,"Custom per-LED supported without mode switch");checks++;
 write(dir+"/fixture.json",{{"gpu_power_percent",nullptr},{"gpu_model",nullptr}});wait(350);before=controller.updates;wait(450);need(controller.updates==before,"unknown telemetry freezes");checks++;
 need(!command(plugin,"execute arbitrary") ["ok"].get<bool>(),"unknown command rejected");checks++;
 write(control,{{"paused","false"},{"generation","bad"}});wait(300);need(command(plugin,"status")["status"]["paused"].get<bool>(),"corrupt control fails closed");checks++;
 write(control,{{"paused",false},{"generation","new"},{"reason","test"}});plugin->OnProfileAboutToLoad();need(command(plugin,"status")["status"]["paused"].get<bool>(),"profile cannot auto resume");checks++;
 auto tree=widget->findChild<QTreeWidget*>("targetTree");need(tree&&tree->topLevelItemCount()==1&&tree->topLevelItem(0)->childCount()==3,"device grouping uses detected controller zones");checks++;
 auto empty=tree->itemWidget(tree->topLevelItem(0)->child(0),0)->findChild<QCheckBox*>("enabled");need(empty&&!empty->isEnabled()&&!empty->isChecked(),"empty zone disabled");checks++;
 tree->topLevelItem(0)->setCheckState(0,Qt::Checked);need(command(plugin,"status")["status"]["rules"][0]["configuration"]["devices"].size()==2,"whole-device selection includes only populated zones");checks++;
 wait(1900);need(api.settings["measures"][0]["devices"].size()==2,"edited device selections autosave");checks++;
 tree->topLevelItem(0)->setCheckState(0,Qt::Unchecked);need(command(plugin,"status")["status"]["rules"][0]["configuration"]["devices"].empty(),"whole-device deselection");checks++;
 auto solid=measure;solid["brightness"]=50;solid["devices"][0]["brightness"]=100;solid["effect"]={{"mode","steady"},{"trigger","always"},{"period",5},{"floor",.3},{"smoothing",.1}};solid["measure_color"]["color_stops"]={{0,0x00FF00}};
 write(dir+"/fixture.json",{{"gpu_power_percent",50},{"gpu_model",true},{"cpu_model",false},{"ram_model",false}});
 controller.active=0;controller.colors.assign(4,0xFF00);plugin->OnProfileAboutToLoad();plugin->OnProfileLoad({{"measures",{solid}}});wait(250);
 auto removeButtons=widget->findChildren<QPushButton*>("remove_button");need(removeButtons.size()==1&&removeButtons[0]->text()=="Remove"&&removeButtons[0]->minimumWidth()>=70&&!removeButtons[0]->isEnabled(),"readable remove action preserves last stop");checks++;
 before=controller.updates;wait(250);auto output=widget->findChild<QLabel*>("outputDetails");need(output&&output->text().contains("#008000")&&controller.updates==before,"single-stop paused preview includes brightness without writes");checks++;
 command(plugin,"resume");wait(450);need(controller.colors[0]==0x8000&&controller.colors[1]==0x8000&&controller.colors[2]==0xFF00&&controller.colors[3]==0xFF00&&controller.updates>before,"single green stop is exact half-brightness; unselected zone preserved");checks++;
 command(plugin,"pause");auto two=solid;two["measure_color"]["color_stops"]={{0,0xFF0000},{100,0x0000FF}};plugin->OnProfileAboutToLoad();plugin->OnProfileLoad({{"measures",{two}}});wait(250);
 removeButtons=widget->findChildren<QPushButton*>("remove_button");need(removeButtons.size()==2&&removeButtons[0]->isEnabled(),"two stops removable");removeButtons[0]->click();wait(250);
 need(command(plugin,"status")["status"]["rules"][0]["configuration"]["measure_color"]["color_stops"].size()==1&&widget->findChild<QLabel*>("outputDetails")->text().contains("#800000"),"remove stop updates solid preview without black fallback");checks++;
 plugin->OnProfileAboutToLoad();two["brightness"]=100;two["measure_color"]["color_stops"]={{0,0xFF0000},{100,0x0000FF}};plugin->OnProfileLoad({{"measures",{two}}});write(dir+"/fixture.json",{{"gpu_power_percent",0},{"gpu_model",false}});command(plugin,"resume");wait(450);need(controller.colors[0]==0xFF0000,"gradient blue endpoint exact");checks++;
 write(dir+"/fixture.json",{{"gpu_power_percent",100},{"gpu_model",false}});wait(450);need(controller.colors[0]==0x0000FF,"gradient red endpoint exact");checks++;
 auto competing=solid;competing["name"]="Competing rule";plugin->OnProfileAboutToLoad();plugin->OnProfileLoad({{"measures",{solid,competing}}});controller.colors.assign(4,0xFF00);command(plugin,"resume");before=controller.updates;wait(450);
 auto diagnostic=command(plugin,"status")["status"]["rules"];need(controller.updates==before&&diagnostic[0]["state"].get<std::string>().find("Conflict")==0&&diagnostic[1]["state"].get<std::string>().find("Conflict")==0,"conflicting rules both inhibited");checks++;
 auto editors=widget->findChildren<QWidget*>("measureEditor");need(editors.size()==2,"two editors");tree=editors[0]->findChild<QTreeWidget*>("targetTree");tree->topLevelItem(0)->setCheckState(0,Qt::Unchecked);wait(450);need(controller.updates>before,"deselect releases ownership without stop/restart");checks++;
 command(plugin,"pause");plugin->OnProfileAboutToLoad();controller.counts={0,0,48,48,0,0,0,0};controller.colors.assign(96,0xFF00);controller.alias="SL Infinity (simulated)";controller.zoneAliases={"","","Fans","Infinity"};
 auto sl=solid;sl["name"]="SL grouping test";sl["devices"]=json::array();for(unsigned i:{2u,3u}){auto z=zone;z["zone_idx"]=i;z["zone_name"]="Zone "+std::to_string(i);z["leds_count"]=48;z["brightness"]=100;sl["devices"].push_back(z);}
 TestController hidden;hidden.hidden=true;api.controllers.push_back(&hidden);plugin->OnProfileLoad({{"measures",{sl}}});wait(250);
 tree=widget->findChild<QTreeWidget*>("targetTree");need(tree->topLevelItemCount()==1&&tree->topLevelItem(0)->text(0)=="SL Infinity (simulated)"&&tree->topLevelItem(0)->childCount()==8,"same detected inventory; hidden controller excluded; eight SL channels grouped");checks++;
 unsigned enabledChannels=0;for(int i=0;i<8;i++)if(tree->itemWidget(tree->topLevelItem(0)->child(i),0)->findChild<QCheckBox*>("enabled")->isEnabled())enabledChannels++;
 need(enabledChannels==2&&tree->itemWidget(tree->topLevelItem(0)->child(2),0)->findChild<QLabel*>("zone_name")->text().contains("Fans"),"only populated SL channels selectable; display aliases reused");checks++;
 widget->resize(1180,900);wait(1100);widget->grab().save(argv[4]);
 command(plugin,"pause");plugin->OnProfileAboutToLoad();
 controller.location="HID: /dev/hidraw117";
 for(auto& target:sl["devices"])target["location"]="HID: /dev/hidraw9";
 plugin->OnProfileLoad({{"measures",{sl}}});wait(250);
 need(command(plugin,"status")["status"]["rules"][0]["configuration"]["devices"].size()==2,"Linux hidraw renumbering preserves unique identities and zone sizes");checks++;
 // A transient empty topology must not erase the durable associations.
 api.controllers.clear();plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);wait(1900);
 need(command(plugin,"status")["status"]["rules"][0]["configuration"]["devices"].size()==2,"rescan retains absent targets before autosave");checks++;
 need(api.settings["measures"][0]["devices"].size()==2,"autosave retains absent targets");checks++;
 api.controllers={&controller};plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);wait(250);
 need(command(plugin,"status")["status"]["rules"][0]["configuration"]["devices"].size()==2,"rescan restores returning targets");checks++;
 auto config=[&](){return command(plugin,"status")["status"]["rules"][0]["configuration"];};
 auto states=[&](){return command(plugin,"status")["status"]["rules"][0]["assignments"];};
 need(states().size()==2&&states()[0]["state"]=="resolved"&&states()[1]["state"]=="resolved","returning zones really rebound");checks++;
 auto durable=config();auto persistedBeforeRescans=api.settings;
 for(int i=0;i<3;i++)plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
 wait(1900);need(config()==durable&&api.settings==persistedBeforeRescans,"identical repeated rescans preserve full rule through autosave");checks++;
 TestController unrelated;unrelated.deviceName="Other inert model";
 api.controllers={&unrelated,&controller};plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
 need(config()==durable&&states()[0]["state"]=="resolved","controller order change does not change assignments");checks++;
 controller.location="new-port";controller.version="firmware-2";controller.description="Updated description";
 plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
 need(states()[0]["state"]=="resolved"&&config()==durable,"unique serial survives port and metadata changes without rewriting choices");checks++;
 controller.zoneNames={"Zero","One","Zone 3","Zone 2"};
 plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
 need(states()[0]["state"]=="resolved"&&states()[1]["state"]=="resolved","unique named zones survive index reordering");checks++;
 controller.counts[2]=24;controller.colors.resize(72,0xFF00);
 plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
 need(states()[1]["state"]=="zone_unavailable"&&config()==durable,"changed LED count remains pending rather than assigned unsafely");checks++;
 controller.counts[2]=48;controller.colors.resize(96,0xFF00);
 TestController twin;twin.counts=controller.counts;twin.zoneNames=controller.zoneNames;twin.colors=controller.colors;twin.location="other-port";
 api.controllers={&twin,&controller};plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
 need(states()[0]["state"]=="device_ambiguous"&&config()==durable,"duplicate serial cannot claim an arbitrary twin");checks++;
 api.controllers={&controller};plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
 auto noSerial=durable;for(auto& row:noSerial["devices"]){row["serial"]="";row["location"]="HID: /dev/hidraw4";}
 controller.serial="";controller.location="HID: /dev/hidraw8";
 plugin->OnProfileLoad({{"measures",{noSerial}}});
 need(states()[0]["state"]=="resolved","serial-less unique normalized path supported");checks++;
 twin.serial="";twin.location="HID: /dev/hidraw18";api.controllers={&controller,&twin};
 plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
 need(states()[0]["state"]=="device_ambiguous"&&config()==noSerial,"serial-less normalized collision preserves choices without guessing");checks++;
 api.controllers={&controller};plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
 tree=widget->findChild<QTreeWidget*>("targetTree");tree->topLevelItem(0)->setCheckState(0,Qt::Unchecked);
 need(config()["devices"].empty(),"explicit deselection removes the remembered associations");checks++;
 plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
 need(config()["devices"].empty(),"rescan never resurrects deliberately deselected targets");checks++;
 plugin->OnProfileLoad({{"measures",{noSerial}}});api.controllers.clear();
 plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
 auto forget=widget->findChild<QPushButton*>("forgetUnavailableTargets");need(forget&&!forget->isHidden(),"unavailable targets can be deliberately forgotten");forget->click();
 need(config()["devices"].empty(),"explicit forget removes only unavailable choices");checks++;
 api.controllers={&controller};plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
 need(config()["devices"].empty(),"forgotten device does not return selected");checks++;
 need(command(plugin,"status")["status"]["paused"].get<bool>(),"topology update preserves explicit-resume protection");checks++;
 controller.segments.resize(8);controller.segments[2]={{"Outer",24},{"Inner",24}};
 auto segmented=noSerial;segmented["devices"]=json::array({noSerial["devices"][0]});auto& segment=segmented["devices"][0];segment["is_segment"]=true;segment["segment_idx"]=0;segment["segment_name"]="Outer";segment["zone_name"]=controller.GetZoneName(2);segment["zone_idx"]=2;segment["leds_count"]=24;
 plugin->OnProfileLoad({{"measures",{segmented}}});need(states()[0]["state"]=="resolved","segment configuration restored");checks++;
 std::swap(controller.segments[2][0],controller.segments[2][1]);plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
 need(states()[0]["state"]=="resolved"&&config()==segmented,"unique segment names survive index changes");checks++;
 controller.segments[2][0].first="Outer";plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
 need(states()[0]["state"]=="zone_ambiguous","ambiguous segment remains pending");checks++;
 auto temporaryController=new TestController;api.controllers={temporaryController};auto temporaryRule=solid;
 plugin->OnProfileLoad({{"measures",{temporaryRule}}});need(states()[0]["state"]=="resolved","temporary controller bound");
 auto rememberedTemporary=config();api.controllers.clear();delete temporaryController;
 plugin->ResourceManagerUpdated(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
 need(config()==rememberedTemporary&&states()[0]["state"]=="device_absent","rescan never dereferences destroyed previous controller");checks++;
 need(maxHeartbeat<500,"slow status disk must not block GUI heartbeat");checks++;
 plugin->Unload();delete widget;
 loaderAffinity.start();QMetaObject::invokeMethod(object,[&](){object->moveToThread(app.thread());},Qt::BlockingQueuedConnection);loaderAffinity.quit();loaderAffinity.wait();
 need(loader.unload(),"unload");checks++;
 std::cout<<json({{"passed",true},{"checks",checks},{"hardware_writes",0},{"controller_simulated",true}}).dump()<<std::endl;return 0;
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<std::endl;return 1;}}
