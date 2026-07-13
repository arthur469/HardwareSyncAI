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
    OpenRGBPluginInfo info;

    info.Name           = PROJECT_NAME;
    info.Description    = PROJECT_DESC;
    info.Version        = VERSION_STRING;
    info.Commit         = GIT_COMMIT_ID;
    info.URL            = PROJECT_URL;
    info.Location       = OPENRGB_PLUGIN_LOCATION_TOP;
    info.Label          = "Hardware Sync";
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

#ifdef _WIN32
    can_load = QFile(QCoreApplication::applicationDirPath()+"\\lhwm-wrapper.dll").exists();
#endif

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

        ui = new HardwareSyncMainPage(nullptr);

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
    ui->StopAll();

    LOG_INFO("[OpenRGBHardwareSyncPlugin] Unloaded.");
}

void OpenRGBHardwareSyncPlugin::OnProfileAboutToLoad()
{
    if(QThread::currentThread() == this->thread())
    {
        ui->Clear();
    }
    else
    {
        QMetaObject::invokeMethod(ui, [=](){ui->Clear();}, Qt::BlockingQueuedConnection);
    }
}

void OpenRGBHardwareSyncPlugin::OnProfileLoad(nlohmann::json profile_data)
{
    if(profile_data.contains("measures"))
    {
        json measures_data = profile_data["measures"];

        if(QThread::currentThread() == this->thread())
        {
            ui->LoadMeasures(measures_data);
        }
        else
        {
            QMetaObject::invokeMethod(ui, [=](){ui->LoadMeasures(measures_data);}, Qt::QueuedConnection);
        }
    }
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

unsigned char* OpenRGBHardwareSyncPlugin::OnSDKCommand(unsigned int /*pkt_id*/, unsigned char * /*pkt_data*/, unsigned int * /*pkt_size*/)
{
    return(NULL);
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
            ui->StopAll();
            QMetaObject::invokeMethod(ui, "Clear", Qt::BlockingQueuedConnection);
            QMetaObject::invokeMethod(ui, "InitDevicesList", Qt::BlockingQueuedConnection);
            break;
    }
}

void OpenRGBHardwareSyncPlugin::SettingsManagerUpdated(unsigned int /*update_reason*/)
{

}