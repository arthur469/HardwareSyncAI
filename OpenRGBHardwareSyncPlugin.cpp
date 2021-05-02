#include "OpenRGBHardwareSyncPlugin.h"
#include <QHBoxLayout>
#include "HardwareSyncMainPage.h"

bool OpenRGBHardwareSyncPlugin::DarkTheme = false;
ResourceManager* OpenRGBHardwareSyncPlugin::RMPointer = nullptr;
std::vector<ControllerZone*> OpenRGBHardwareSyncPlugin::controller_zones;

OpenRGBPluginInfo OpenRGBHardwareSyncPlugin::Initialize(bool Dt, ResourceManager *RM)
{
    PInfo.PluginName         = "Hardware Sync";
    PInfo.PluginDescription  = "Sync colors with hardware measures";
    PInfo.PluginLocation     = "TopTabBar";
    PInfo.HasCustom          = true;
    PInfo.PluginLabel        = new QLabel("Hardware Sync");

    RMPointer                = RM;
    DarkTheme                = Dt;

    return PInfo;
}

QWidget* OpenRGBHardwareSyncPlugin::CreateGUI(QWidget*)
{
    RMPointer->WaitForDeviceDetection();

    ResetControllers();

    HardwareSyncMainPage* ui = new HardwareSyncMainPage(nullptr);

    RMPointer->RegisterDeviceListChangeCallback(DeviceListChangedCallback, this);
    RMPointer->RegisterDetectionProgressCallback(DeviceListChangedCallback, this);

    return new HardwareSyncMainPage(ui);
}

void OpenRGBHardwareSyncPlugin::DeviceListChangedCallback(void* o)
{
    ResetControllers();
}

void OpenRGBHardwareSyncPlugin::ResetControllers()
{
    controller_zones.clear();

    std::vector<RGBController*> controllers = OpenRGBHardwareSyncPlugin::RMPointer->GetRGBControllers();

    for(RGBController* controller: controllers)
    {
        for(unsigned int i = 0; i < controller->zones.size(); i++)
        {
            controller_zones.push_back(new ControllerZone(controller, i));
        }
    }
}
