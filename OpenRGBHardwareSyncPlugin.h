#ifndef OPENRGBHARDWARESYNCPLUGIN_H
#define OPENRGBHARDWARESYNCPLUGIN_H

#include "OpenRGBPluginInterface.h"
#include "ResourceManagerInterface.h"
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

    void                Load(ResourceManagerInterface* resource_manager_ptr) override;
    QWidget*            GetWidget() override;
    QMenu*              GetTrayMenu() override;
    void                Unload() override;

    static HardwareMeasure* hm;
    static ResourceManagerInterface* RMPointer;

private:

    static void DeviceListChangedCallback(void*);

    HardwareSyncMainPage* ui;

    bool can_load = true;
};

#endif // OPENRGBHARDWARESYNCPLUGIN_H
