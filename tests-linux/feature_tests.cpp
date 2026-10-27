// SPDX-License-Identifier: GPL-2.0-or-later
#include <QApplication>
#include <QPluginLoader>
#include <QTemporaryDir>
#include <QTimer>
#include <QEventLoop>
#include <QFile>
#include <QComboBox>
#include <iostream>
#include <stdexcept>
#include "TestAPI.h"
#include "TestController.h"
#include "ResourceManagerCallback.h"
using json=nlohmann::json;
void need(bool v,const char* m){if(!v)throw std::runtime_error(m);}
void wait(int ms){QEventLoop e;QTimer::singleShot(ms,&e,&QEventLoop::quit);e.exec();}
void write(const QString& p,const json& j){QFile f(p);need(f.open(QIODevice::WriteOnly|QIODevice::Truncate),"fixture write");f.write(j.dump().c_str());}
json command(OpenRGBPluginInterface* p,const std::string& name){auto text=json({{"command",name}}).dump();unsigned size=text.size();auto data=p->OnSDKCommand(1,(unsigned char*)text.data(),&size);need(data,"SDK reply");auto result=json::parse(data,data+size);delete[] data;return result;}
int main(int argc,char** argv){QApplication app(argc,argv);unsigned checks=0;try{
 need(argc==4,"args plugin.so python fixture.py");QTemporaryDir dir;TestAPI api;TestController controller;controller.modeNames.push_back("Breathing");api.controllers={&controller};api.configDirectory=dir.path().toStdString();
 auto control=dir.path()+"/control.json", fixture=dir.path()+"/fixture.json";
 write(control,{{"paused",true},{"generation","initial"}});write(fixture,{{"gpu_power_percent",50},{"gpu_model",true},{"model_count",1}});
 json zone={{"name",controller.GetName()},{"location",controller.GetLocation()},{"serial",controller.GetSerial()},{"vendor",controller.GetVendor()},{"description",controller.GetDescription()},{"version",controller.GetVersion()},{"zone_name","Zone 1"},{"zone_idx",1},{"leds_count",2},{"is_segment",false},{"segment_idx",-1},{"brightness",100}};
 json rule={{"name","Inert overlay test"},{"hardware_identifier","gpu_power_percent"},{"devices",json::array({zone})},{"measure_color",{{"color_stops",{{0,0x00FF00}}}}},{"brightness",50},{"fps",5},{"running",true},{"effect",{{"mode","steady"},{"trigger","gpu_model"},{"smoothing",.1},{"deadband",1},{"period",2},{"floor",.3}}}};
 api.settings={{"control_file",control.toStdString()},{"control_lock",(dir.path()+"/lock").toStdString()},{"status_file",(dir.path()+"/status").toStdString()},{"collector_python",argv[2]},{"collector_script",argv[3]},{"library_path",dir.path().toStdString()},{"measures",json::array({rule})}};
 QPluginLoader loader(argv[1]);auto object=loader.instance();need(object,qPrintable(loader.errorString()));auto plugin=qobject_cast<OpenRGBPluginInterface*>(object);plugin->Load(&api);auto widget=plugin->GetWidget();wait(300);plugin->OnProfileLoad({{"measures",json::array()}});
 auto state=[&](){return command(plugin,"status")["status"]["rules"][0];};
 command(plugin,"resume");wait(450);need(controller.colors[0]==0x8000,"No effect retains measurement color regardless of loaded model");checks++;
 auto unchanged=controller.updates;wait(300);need(controller.updates==unchanged,"identical frames never trigger redundant RGB writes");checks++;
 auto mode=widget->findChild<QComboBox*>("effectMode");auto condition=widget->findChild<QComboBox*>("effectTrigger");need(mode->itemText(0)=="No effect"&&condition&&condition->count()==4&&condition->itemText(1)=="Model GPU"&&condition->itemText(2)=="Model RAM"&&condition->itemText(3)=="Model CPU"&&!condition->itemData(2,Qt::ToolTipRole).toString().isEmpty(),"short labels, four conditions and detailed tooltips");checks++;need(mode->findData("native:breathing")<0,"partial device selection never offers whole-device effect");checks++;
 auto breathing=rule;breathing["effect"]["mode"]="breathing";plugin->OnProfileLoad({{"measures",json::array({breathing})}});command(plugin,"resume");auto before=controller.updates;write(fixture,{{"gpu_power_percent",50},{"gpu_model",true},{"model_count",1}});wait(1000);
 need(controller.updates-before>=20&&state()["frame_interval_ms"].get<int>()<=34,"software breathing rendered at least 30fps independent of 5fps measurement setting");checks++;
 auto palette=state()["palette_target"];write(fixture,{{"gpu_power_percent",50.2},{"gpu_model",true},{"model_count",1}});wait(300);
 need(state()["palette_target"]==palette,"tiny telemetry changes do not move the palette");checks++;
 write(fixture,{{"gpu_power_percent",53},{"gpu_model",true},{"model_count",1}});wait(300);need(state()["palette_target"]==53,"meaningful telemetry changes still update the palette");checks++;
 command(plugin,"pause");auto full=rule;auto second=zone;second["zone_idx"]=2;second["zone_name"]="Zone 2";full["devices"].push_back(second);full["effect"]["mode"]="native:breathing";full["effect"]["trigger"]="gpu_model";
 controller.colors.assign(4,0xFF00);plugin->OnProfileLoad({{"measures",json::array({full})}});mode=widget->findChild<QComboBox*>("effectMode");need(mode->findData("native:breathing")>=0,"common native effect offered for entire device");checks++;
 command(plugin,"resume");wait(300);need(controller.active==4,"native effect activates for loaded GPU model");checks++;
 before=controller.modeChanges;wait(600);need(controller.modeChanges==before,"native effect not restarted on every frame");checks++;
 write(fixture,{{"gpu_power_percent",53},{"gpu_model",false},{"model_count",0}});wait(450);need(controller.active==0,"condition false restores preceding mode and suppresses native overlay");checks++;
 TestController other;other.serial="other";api.controllers.push_back(&other);auto third=zone;third["serial"]="other";third["zone_idx"]=1;auto fourth=third;fourth["zone_idx"]=2;fourth["zone_name"]="Zone 2";full["devices"].push_back(third);full["devices"].push_back(fourth);
 command(plugin,"pause");plugin->OnProfileLoad({{"measures",json::array({full})}});controller.colors.assign(4,0xFF00);other.colors.assign(4,0xFF00);before=controller.modeChanges;
 command(plugin,"resume");write(fixture,{{"gpu_power_percent",53},{"gpu_model",true},{"model_count",1}});wait(450);
 need(controller.modeChanges==before&&other.modeChanges==0,"unsupported effect on one selected device inhibits entire native effect");checks++;
 plugin->Unload();delete widget;need(loader.unload(),"unload");std::cout<<json({{"passed",true},{"checks",checks},{"controller_simulated",true},{"hardware_writes",0}}).dump()<<std::endl;return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<std::endl;return 1;}}
