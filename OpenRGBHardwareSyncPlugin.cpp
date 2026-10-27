/*---------------------------------------------------------*\
| OpenRGBHardwareSyncPlugin.cpp                             |
|                                                           |
|   OpenRGB Hardware Sync Plugin                            |
|                                                           |
|   This file is part of the OpenRGB Hardware Sync Plugin   |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <QHBoxLayout>
#include "TelemetryRuntime.h"
#include <QFile>
#include <QTabWidget>
#include <QThread>
#include "LogManager.h"
#include "OpenRGBHardwareSyncPlugin.h"
#include "ResourceManagerCallback.h"
#include "MeasureEntry.h"

OpenRGBPluginAPIInterface*  OpenRGBHardwareSyncPlugin::api  = nullptr;
HardwareMeasure*            OpenRGBHardwareSyncPlugin::hm   = nullptr;

OpenRGBHardwareSyncPlugin::~OpenRGBHardwareSyncPlugin()
{
    if(hm)
    {
        delete hm;
    }
}

/*---------------------------------------------------------*\
| Plugin Information                                        |
\*---------------------------------------------------------*/
OpenRGBPluginInfo OpenRGBHardwareSyncPlugin::GetPluginInfo()
{
    OpenRGBPluginInfo info{};

    info.Name           = PROJECT_NAME;
    info.Description    = PROJECT_DESC;
    info.Version        = VERSION_STRING;
    info.Commit         = GIT_COMMIT_ID;
    info.URL            = PROJECT_URL;
    info.Location       = OPENRGB_PLUGIN_LOCATION_TOP;
    info.Label          = "Hardware Sync AI";
    info.ProtocolVersion = 1;
    info.TabIconString  = "Hardware Sync";

    info.TabIcon.load(":/OpenRGBHardwareSyncPlugin.png");
    info.Icon.load(":/OpenRGBHardwareSyncPlugin.png");

    return info;
}

unsigned int OpenRGBHardwareSyncPlugin::GetPluginAPIVersion()
{
    return OPENRGB_PLUGIN_API_VERSION;
}

/*---------------------------------------------------------*\
| Plugin Functionality                                      |
\*---------------------------------------------------------*/
void OpenRGBHardwareSyncPlugin::Load(OpenRGBPluginAPIInterface* api_interface_ptr)
{
    /*-----------------------------------------------------*\
    | Store API interface pointer                           |
    \*-----------------------------------------------------*/
    api = api_interface_ptr;

    /*-----------------------------------------------------*\
    | Log initial messages                                  |
    \*-----------------------------------------------------*/
    LOG_INFO("[OpenRGBHardwareSyncPlugin] version %s (%s), build date %s", VERSION_STRING, GIT_COMMIT_ID, GIT_COMMIT_DATE);

    can_load = true;

    if(can_load)
    {
        hm = new HardwareMeasure();
    }
}

QWidget* OpenRGBHardwareSyncPlugin::GetWidget()
{
    if(can_load)
    {
        api->WaitForDetection();

        telemetryRuntime().start();
        ui = new HardwareSyncMainPage(nullptr);
        auto stored=api->GetSettings("RGBTelemetryPlugin");
        if(stored.is_object()&&stored.contains("measures"))ui->LoadMeasures(stored["measures"]);

        return ui;
    }
    else
    {
        QLabel* label = new QLabel(
            "<h1>Cannot load the plugin.</h1>"
            "<p>Make sure you downloaded <a href=\"https://gitlab.com/api/v4/projects/25437611/jobs/artifacts/master/raw/x64/Release/lhwm-wrapper.dll?job=main_build\">lhwm-wrapper.dll</a></p>"
            "<p>Place this DLL inside  <b>" + QCoreApplication::applicationDirPath() +  "</b> and restart OpenRGB.</p>"
        );

        label->setTextFormat(Qt::RichText);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        label->setOpenExternalLinks(true);

        return label;
    }
}

QMenu* OpenRGBHardwareSyncPlugin::GetTrayMenu()
{
    return nullptr;
}

void OpenRGBHardwareSyncPlugin::Unload()
{
    if(ui)ui->StopAll();
    telemetryRuntime().shutdown();

    LOG_INFO("[OpenRGBHardwareSyncPlugin] Unloaded.");
}

void OpenRGBHardwareSyncPlugin::OnProfileAboutToLoad(){
    if(!ui)return;
    auto prepare=[this](){telemetryRuntime().readControl();resumeInitialProfile=initialProfile && !telemetryRuntime().paused;if(!telemetryRuntime().paused)telemetryRuntime().setPause(true,"Profile change; explicit resume required");};
    if(QThread::currentThread()==ui->thread())prepare();else QMetaObject::invokeMethod(ui,prepare,Qt::BlockingQueuedConnection);
}
void OpenRGBHardwareSyncPlugin::OnProfileLoad(nlohmann::json profile_data){
    if(!ui||!profile_data.is_object()||!profile_data.contains("measures"))return;
    auto load=[this,profile_data](){
       // Settings are the latest manually edited configuration. Startup profile
       // data can be older; subsequent explicit profile loads still take effect.
       if(initialProfile){initialProfile=false;if(!ui->GetMeasures().empty()){if(resumeInitialProfile)telemetryRuntime().setPause(false,"Startup profile loaded; prior enabled state retained, black/OFF remains protected");return;}}
       try{if(!profile_data["measures"].is_array()||profile_data["measures"].size()>32)throw std::runtime_error("Invalid measures");ui->Clear();ui->LoadMeasures(profile_data["measures"]);}
       catch(...){telemetryRuntime().setPause(true,"Invalid profile; output inhibited");}
    };
    if(QThread::currentThread()==ui->thread())load();else QMetaObject::invokeMethod(ui,load,Qt::BlockingQueuedConnection);
}

nlohmann::json OpenRGBHardwareSyncPlugin::OnProfileSave()
{
    nlohmann::json profile_data;

    std::vector<nlohmann::json> v;

    for(MeasureEntry* measure: ui->GetMeasures())
    {
        v.push_back(measure->ToJson());
    }

    profile_data["measures"] = v;

    return(profile_data);
}

unsigned char* OpenRGBHardwareSyncPlugin::OnSDKCommand(unsigned int pkt_id,unsigned char* pkt_data,unsigned int* pkt_size){
    json response;
    if(pkt_id!=1||!pkt_size||*pkt_size==0||*pkt_size>65536){response={{"ok",false},{"error","Invalid bounded request"}};}
    else {
      try{
        auto request=json::parse(pkt_data,pkt_data+*pkt_size);
        if(!request.is_object()||!request.contains("command")||!request["command"].is_string())throw std::runtime_error("Missing command");
        for(auto it=request.begin();it!=request.end();++it)if(it.key()!="command")throw std::runtime_error("Unknown request field");
        const auto command=request["command"].get<std::string>();
        if(!ui)throw std::runtime_error("Plugin UI not ready");
        auto execute=[&](){
          if(command=="status"){
            telemetryRuntime().readControl();auto state=telemetryRuntime().status();state["detection_percent"]=api->GetDetectionPercent();if(ui)state["rules"]=ui->Diagnostics();state["controllers"]=json::array();
            for(auto c:api->GetRGBControllers()){json zones=json::array();for(unsigned z=0;z<c->GetZoneCount();z++)zones.push_back({{"index",z},{"name",c->GetZoneName(z)},{"display_name",c->GetZoneDisplayName(z)},{"leds_count",c->GetZoneLEDsCount(z)}});state["controllers"].push_back({{"name",c->GetName()},{"display_name",c->GetDisplayName()},{"hidden",c->GetHidden()},{"zones",zones}});}
            response={{"ok",true},{"status",state}};
          }
          else if(command=="show"){
            if(!ui)throw std::runtime_error("Plugin UI unavailable");
            QWidget* child=ui;
            for(QWidget* parent=ui->parentWidget();parent;child=parent,parent=parent->parentWidget())if(auto tabs=qobject_cast<QTabWidget*>(parent)){int index=tabs->indexOf(child);if(index>=0)tabs->setCurrentIndex(index);}
            ui->window()->showNormal();ui->window()->raise();ui->window()->activateWindow();response={{"ok",true}};
          }
          else if(command=="pause"||command=="resume"){
            bool ok=telemetryRuntime().setPause(command=="pause",command=="pause"?"Pause requested by plugin SDK":"Resume requested by plugin SDK; black/OFF protection remains");
            response={{"ok",ok},{"status",telemetryRuntime().status()}};
          }else throw std::runtime_error("Unsupported command");
        };
        if(QThread::currentThread()==ui->thread())execute();else QMetaObject::invokeMethod(ui,[&](){try{execute();}catch(const std::exception& e){response={{"ok",false},{"error",e.what()}};}},Qt::BlockingQueuedConnection);
      }catch(const std::exception& e){response={{"ok",false},{"error",e.what()}};}
    }
    auto text=response.dump();*pkt_size=static_cast<unsigned>(text.size());auto out=new unsigned char[*pkt_size];memcpy(out,text.data(),*pkt_size);return out;
}

/*---------------------------------------------------------*\
| Update Signals                                            |
\*---------------------------------------------------------*/
void OpenRGBHardwareSyncPlugin::ProfileManagerUpdated(unsigned int /*update_reason*/)
{

}

void OpenRGBHardwareSyncPlugin::ResourceManagerUpdated(unsigned int update_reason)
{
    switch(update_reason)
    {
        case RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED:
            if(ui){
                auto refresh=[this](){telemetryRuntime().setPause(true,"Device topology changed; audit and explicit resume required");telemetryRuntime().clearControllerState();ui->RefreshDevicesLists();};
                if(QThread::currentThread()==ui->thread())refresh();else QMetaObject::invokeMethod(ui,refresh,Qt::BlockingQueuedConnection);
            }
            break;
    }
}

void OpenRGBHardwareSyncPlugin::SettingsManagerUpdated(unsigned int /*update_reason*/)
{

}
