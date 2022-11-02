#ifndef CONTROLLERZONEMANAGER_H
#define CONTROLLERZONEMANAGER_H

#include "ControllerZone.h"

class ControllerZoneManager
{
public:
    ControllerZoneManager();
    static std::vector<ControllerZone*> GetControllerZones();
    static bool SupportsDirectMode(RGBController*);

};

#endif // CONTROLLERZONEMANAGER_H
