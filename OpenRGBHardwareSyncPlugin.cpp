#include "OpenRGBHardwareSyncPlugin.h"
#include <QHBoxLayout>
#include "HardwareSyncMainPage.h"

bool OpenRGBHardwareSyncPlugin::DarkTheme = false;
ResourceManager* OpenRGBHardwareSyncPlugin::RMPointer = nullptr;
std::vector<ControllerZone*> OpenRGBHardwareSyncPlugin::controller_zones;
HardwareMeasure* OpenRGBHardwareSyncPlugin::hm = nullptr;


OpenRGBPluginInfo OpenRGBHardwareSyncPlugin::GetPluginInfo()
{
    OpenRGBPluginInfo info;
    info.Name         = "Hardware Sync";
    info.Description  = "Sync colors with hardware measures";
    info.Version  = VERSION_STRING;
    info.Commit  = GIT_COMMIT_ID;
    info.URL  = "https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin";
    info.Icon.load(":/OpenRGBHardwareSyncPlugin.png");

    info.Location     =  OPENRGB_PLUGIN_LOCATION_TOP;
    info.Label        =  "Hardware Sync";
    info.TabIconString        =  "Hardware Sync";
    info.TabIcon.load(":/OpenRGBHardwareSyncPlugin.png");

    return info;
}

unsigned int OpenRGBHardwareSyncPlugin::GetPluginAPIVersion()
{
    return 1;
}

void OpenRGBHardwareSyncPlugin::Load(bool dark_theme, ResourceManager* resource_manager_ptr)
{
    RMPointer                = resource_manager_ptr;
    DarkTheme                = dark_theme;
    hm = new HardwareMeasure();
}


QWidget* OpenRGBHardwareSyncPlugin::GetWidget()
{
    RMPointer->WaitForDeviceDetection();

    ResetControllers();

    HardwareSyncMainPage* ui = new HardwareSyncMainPage(nullptr);

    RMPointer->RegisterDetectionStartCallback(DetectionStart, ui);
    RMPointer->RegisterDetectionEndCallback(DetectionEnd, ui);

    return ui;
}

QMenu* OpenRGBHardwareSyncPlugin::GetTrayMenu()
{
    QMenu* menu = new QMenu("Hardware sync");

    return menu;
}

void OpenRGBHardwareSyncPlugin::Unload()
{
    printf("Time to call some cleaning stuff.\n");
}

void OpenRGBHardwareSyncPlugin::ResetControllers()
{
    controller_zones.clear();

    std::vector<RGBController*> controllers = OpenRGBHardwareSyncPlugin::RMPointer->GetRGBControllers();

    for(RGBController* controller: controllers)
    {
        if(!SupportsDirectMode(controller))
        {
            continue;
        }

        for(unsigned int i = 0; i < controller->zones.size(); i++)
        {
            controller_zones.push_back(new ControllerZone(controller, i));
        }
    }
}

bool OpenRGBHardwareSyncPlugin::SupportsDirectMode(RGBController* controller)
{
    for(unsigned int i = 0; i < controller->modes.size(); i++)
    {
        if(controller->modes[i].name == "Direct")
        {
            return true;
        }
    }

    return false;
}

void OpenRGBHardwareSyncPlugin::DetectionStart(void* o)
{
    printf("[OpenRGBHardwareSyncPlugin] DetectionStart\n");

    controller_zones.clear();

    QMetaObject::invokeMethod((HardwareSyncMainPage *)o, "Clear", Qt::QueuedConnection);
}

void OpenRGBHardwareSyncPlugin::DetectionEnd(void*)
{
    printf("[OpenRGBHardwareSyncPlugin] DetectionEnd\n");

    ResetControllers();
}

OpenRGBHardwareSyncPlugin::~OpenRGBHardwareSyncPlugin()
{
    printf("[OpenRGBHardwareSyncPlugin] DetectionEnd\n");

    if(hm)
    {
        delete hm;
    }
}
