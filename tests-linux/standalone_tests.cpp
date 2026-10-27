// SPDX-License-Identifier: GPL-2.0-or-later
#include <QApplication>
#include <QPluginLoader>
#include <QTemporaryDir>
#include <QTimer>
#include <QEventLoop>
#include <QFile>
#include <iostream>
#include <stdexcept>
#include "TestAPI.h"
#include "TestController.h"
using json=nlohmann::json;
void need(bool v,const char* m){if(!v)throw std::runtime_error(m);}
void wait(int ms){QEventLoop e;QTimer::singleShot(ms,&e,&QEventLoop::quit);e.exec();}
json command(OpenRGBPluginInterface* p,const std::string& name){auto text=json({{"command",name}}).dump();unsigned size=text.size();auto data=p->OnSDKCommand(1,(unsigned char*)text.data(),&size);need(data,"SDK reply");auto result=json::parse(data,data+size);delete[] data;return result;}
int main(int argc,char** argv){QApplication app(argc,argv);try{
 need(argc==2,"args plugin.so");QTemporaryDir dir;need(dir.isValid(),"isolated config");TestAPI api;TestController controller;api.controllers={&controller};api.configDirectory=dir.path().toStdString();api.settings=json::object();
 QPluginLoader loader(argv[1]);auto object=loader.instance();need(object,qPrintable(loader.errorString()));auto plugin=qobject_cast<OpenRGBPluginInterface*>(object);need(plugin,"plugin interface");plugin->Load(&api);auto widget=plugin->GetWidget();need(widget,"widget");wait(4500);
 auto status=command(plugin,"status")["status"];
 need(status["paused"].get<bool>(),"fresh install must remain paused");need(status["collector_error"]=="","bundled collector running without private library");need(status["telemetry"].contains("cpu_usage_percent")&&status["telemetry"]["ram_usage_percent"].is_number(),"CPU/RAM read-only telemetry");need(controller.updates==0&&controller.modeChanges==0,"no simulated RGB writes on fresh install");
 need(QFile::exists(dir.path()+"/HardwareSyncAI/control.json"),"portable default configuration");
 plugin->Unload();delete widget;need(loader.unload(),"unload");std::cout<<json({{"passed",true},{"checks",6},{"hardware_writes",0},{"fresh_install",true},{"no_personal_library",true}}).dump()<<std::endl;return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<std::endl;return 1;}}
