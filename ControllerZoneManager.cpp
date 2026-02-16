#include "OpenRGBHardwareSyncPlugin.h"
#include "ControllerZoneManager.h"
#include "ControllerZone.h"

ControllerZoneManager::ControllerZoneManager()
{

}

std::vector<ControllerZone*> ControllerZoneManager::GetControllerZones()
{
    std::vector<ControllerZone*> controller_zones;

    for(RGBControllerInterface* controller: OpenRGBHardwareSyncPlugin::api->GetRGBControllers())
    {
        for(unsigned int i = 0; i < controller->GetZoneCount(); i++)
        {
            controller_zones.push_back(new ControllerZone(controller, i, false, false, 100, false, false));

            for(unsigned int s = 0; s < controller->GetZoneSegmentCount(i); s++)
            {
                controller_zones.push_back(new ControllerZone(controller, i, false, false, 100, false, true, s));
            }
        }
    }

    return controller_zones;
}

bool ControllerZoneManager::SupportsDirectMode(RGBControllerInterface* controller)
{
    for(unsigned int i = 0; i < controller->GetModeCount(); i++)
    {
        if(controller->GetModeName(i) == "Direct")
        {
            return true;
        }
    }

    return false;
}
