#ifndef OPENRGBHARDWARESYNCPLUGIN_H
#define OPENRGBHARDWARESYNCPLUGIN_H

#include "OpenRGBPluginInterface.h"
#include "ResourceManager.h"
#include "HardwareMeasure.h"
#include "HardwareSyncMainPage.h"

#include <QObject>
#include <QString>
#include <QtPlugin>
#include <QWidget>

class OpenRGBHardwareSyncPlugin : public QObject, public OpenRGBPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID OpenRGBPluginInterface_IID)
    Q_INTERFACES(OpenRGBPluginInterface)

public:
    ~OpenRGBHardwareSyncPlugin();

    OpenRGBPluginInfo   GetPluginInfo() override;
    unsigned int        GetPluginAPIVersion() override;

    void                Load(bool dark_theme, ResourceManager* resource_manager_ptr) override;
    QWidget*            GetWidget() override;
    QMenu*              GetTrayMenu() override;
    void                Unload() override;

    static bool             DarkTheme;
    static ResourceManager* RMPointer;
    static HardwareMeasure* hm;

private:

    static void DeviceListChangedCallback(void*);

    HardwareSyncMainPage* ui;

    bool can_load = true;
};

#endif // OPENRGBHARDWARESYNCPLUGIN_H
