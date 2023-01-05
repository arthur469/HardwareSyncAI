#include "OpenRGBHardwareSyncPlugin.h"
#include <QHBoxLayout>

bool OpenRGBHardwareSyncPlugin::DarkTheme               = false;
ResourceManager* OpenRGBHardwareSyncPlugin::RMPointer   = nullptr;
HardwareMeasure* OpenRGBHardwareSyncPlugin::hm          = nullptr;

OpenRGBPluginInfo OpenRGBHardwareSyncPlugin::GetPluginInfo()
{
    OpenRGBPluginInfo info;

    info.Name           = "Hardware Sync";
    info.Description    = "Sync colors with hardware measures";
    info.Version        = VERSION_STRING;
    info.Commit         = GIT_COMMIT_ID;
    info.URL            = "https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin";
    info.Location       =  OPENRGB_PLUGIN_LOCATION_TOP;
    info.Label          =  "Hardware Sync";
    info.TabIconString  =  "Hardware Sync";

    info.TabIcon.load(":/OpenRGBHardwareSyncPlugin.png");
    info.Icon.load(":/OpenRGBHardwareSyncPlugin.png");

    return info;
}

unsigned int OpenRGBHardwareSyncPlugin::GetPluginAPIVersion()
{
    return OPENRGB_PLUGIN_API_VERSION;
}

void OpenRGBHardwareSyncPlugin::Load(bool dark_theme, ResourceManager* resource_manager_ptr)
{
    RMPointer = resource_manager_ptr;
    DarkTheme = dark_theme;

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
        RMPointer->WaitForDeviceDetection();

        ui = new HardwareSyncMainPage(nullptr);

        RMPointer->RegisterDeviceListChangeCallback(DeviceListChangedCallback, ui);
        RMPointer->RegisterDeviceListChangeCallback(DeviceListChangedCallback, ui);

        return ui;
    }
    else
    {
        QLabel* label = new QLabel(
            "<h1>Cannot load the plugin.</h1>"
            "<p>Make sure you downloaded <a href=\"https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/raw/master/dependencies/lhwm-cpp-wrapper/x64/Release/lhwm-wrapper.dll\">lhwm-wrapper.dll</a></p>"
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
    printf("[OpenRGBHardwareSyncPlugin] Unload.\n");

    ui->StopAll();

    RMPointer->UnregisterDeviceListChangeCallback(DeviceListChangedCallback, ui);
    RMPointer->UnregisterDeviceListChangeCallback(DeviceListChangedCallback, ui);
}

void OpenRGBHardwareSyncPlugin::DeviceListChangedCallback(void* o)
{
    printf("[OpenRGBHardwareSyncPlugin] DeviceListChangedCallback\n");
    ((HardwareSyncMainPage *)o)->StopAll();
    QMetaObject::invokeMethod((HardwareSyncMainPage *)o, "Clear", Qt::QueuedConnection);
    QMetaObject::invokeMethod((HardwareSyncMainPage *)o, "InitDevicesList", Qt::QueuedConnection);
}

OpenRGBHardwareSyncPlugin::~OpenRGBHardwareSyncPlugin()
{
    printf("[OpenRGBHardwareSyncPlugin] Destructor\n");

    if(hm)
    {
        delete hm;
    }
}
