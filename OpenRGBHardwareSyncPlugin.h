#ifndef OPENRGBHARDWARESYNCPLUGIN_H
#define OPENRGBHARDWARESYNCPLUGIN_H

#include "OpenRGBPluginInterface.h"
#include "ResourceManager.h"
#include "ControllerZone.h"

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
    ~OpenRGBHardwareSyncPlugin() {};

    OpenRGBPluginInfo       PInfo;
    OpenRGBPluginInfo       Initialize(bool, ResourceManager*)   override;
    QWidget*                CreateGUI(QWidget *Parent)           override;
    static bool             DarkTheme;
    static ResourceManager* RMPointer;

    static std::vector<ControllerZone*> controller_zones;

private:
    static void DeviceListChangedCallback(void* ptr);
    static void ResetControllers();
};

#endif // OPENRGBHARDWARESYNCPLUGIN_H
