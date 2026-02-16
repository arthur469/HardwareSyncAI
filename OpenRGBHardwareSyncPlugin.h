/*---------------------------------------------------------*\
| OpenRGBHardwareSyncPlugin.h                               |
|                                                           |
|   OpenRGB Hardware Sync Plugin                            |
|                                                           |
|   This file is part of the OpenRGB Hardware Sync Plugin   |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <QtPlugin>
#include <QObject>
#include <QString>
#include <QWidget>
#include "HardwareMeasure.h"
#include "HardwareSyncMainPage.h"
#include "OpenRGBPluginInterface.h"

class OpenRGBHardwareSyncPlugin : public QObject, public OpenRGBPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID OpenRGBPluginInterface_IID FILE "OpenRGBHardwareSyncPlugin.json")
    Q_INTERFACES(OpenRGBPluginInterface)

public:
    ~OpenRGBHardwareSyncPlugin();

    /*-----------------------------------------------------*\
    | Plugin Information                                    |
    \*-----------------------------------------------------*/
    OpenRGBPluginInfo               GetPluginInfo();
    unsigned int                    GetPluginAPIVersion();

    /*-----------------------------------------------------*\
    | Plugin Functionality                                  |
    \*-----------------------------------------------------*/
    void                            Load(OpenRGBPluginAPIInterface* plugin_api_ptr);
    QWidget*                        GetWidget();
    QMenu*                          GetTrayMenu();
    void                            Unload();
    void                            OnProfileAboutToLoad();
    void                            OnProfileLoad(nlohmann::json profile_data);
    nlohmann::json                  OnProfileSave();
    unsigned char*                  OnSDKCommand(unsigned int pkt_id, unsigned char * pkt_data, unsigned int *pkt_size);

    /*-----------------------------------------------------*\
    | Update Signals                                        |
    \*-----------------------------------------------------*/
    void                            ProfileManagerUpdated(unsigned int update_reason);
    void                            ResourceManagerUpdated(unsigned int update_reason);
    void                            SettingsManagerUpdated(unsigned int update_reason);

private:
    /*-----------------------------------------------------*\
    | User interface widget                                 |
    \*-----------------------------------------------------*/
    HardwareSyncMainPage* ui;
    bool can_load = true;

public:
    /*-----------------------------------------------------*\
    | Plugin Global Variables                               |
    \*-----------------------------------------------------*/
    static OpenRGBPluginAPIInterface *  api;
    static HardwareMeasure*             hm;
};

/*---------------------------------------------------------*\
| LogManager logging macros                                 |
\*---------------------------------------------------------*/
#undef  LogAppend
#define LogAppend(level, ...)   OpenRGBHardwareSyncPlugin::api->LogEntry(__FILE__, __LINE__, level, __VA_ARGS__)
