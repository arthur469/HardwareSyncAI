#include "OpenRGBHardwareSyncPlugin.h"
#include "ControllerZoneManager.h"
#include "ControllerZone.h"

ControllerZoneManager::ControllerZoneManager()
{

}

std::vector<ControllerZone*> ControllerZoneManager::GetControllerZones()
{
    std::vector<ControllerZone*> controller_zones;

    for(RGBController* controller: OpenRGBHardwareSyncPlugin::RMPointer->GetRGBControllers())
    {
        for(unsigned int i = 0; i < controller->zones.size(); i++)
        {
            controller_zones.push_back(new ControllerZone(controller, i, false, false, 100));
        }
    }

    return controller_zones;
}

bool ControllerZoneManager::SupportsDirectMode(RGBController* controller)
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
