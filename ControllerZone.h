#ifndef CONTROLLERZONE_H
#define CONTROLLERZONE_H

#include <nlohmann/json.hpp>
#include "RGBControllerInterface.h"

using json = nlohmann::json;

class ControllerZone
{
public:
    ControllerZone(RGBControllerInterface* controller,
                   unsigned int zone_idx,
                   bool enabled,
                   bool is_percentage_fill,
                   int brightness,
                   bool reverse,
                   bool is_segment,
                   int segment_idx = -1):
        controller(controller),
        zone_idx(zone_idx),
        enabled(enabled),
        is_percentage_fill(is_percentage_fill),
        brightness(brightness),
        reverse(reverse),
        is_segment(is_segment),
        segment_idx(segment_idx)
    {};

    RGBControllerInterface* controller;
    unsigned int zone_idx;
    bool enabled;
    bool is_percentage_fill;
    int brightness;
    bool reverse;
    bool is_segment;
    int segment_idx;

    zone_type type()
    {
        return is_segment ? controller->GetZoneSegmentType(zone_idx, segment_idx) : controller->GetZoneType(zone_idx);
    }

    unsigned int start_idx()
    {
        return is_segment ? controller->GetZoneSegmentStartIndex(zone_idx, segment_idx) : controller->GetZoneStartIndex(zone_idx);
    }

    unsigned int leds_count()
    {
        return is_segment ? controller->GetZoneSegmentLEDsCount(zone_idx, segment_idx) : controller->GetZoneLEDsCount(zone_idx);
    }

    unsigned int matrix_map_width()
    {
        return controller->GetZoneMatrixMapWidth(zone_idx);
    }

    unsigned int matrix_map_height()
    {
        return controller->GetZoneMatrixMapHeight(zone_idx);
    }

    const unsigned int* map()
    {
        return controller->GetZoneMatrixMapData(zone_idx);
    }

    bool operator==(ControllerZone const & rhs) const
    {
        if (is_segment)
        {
            return this->controller == rhs.controller && this->zone_idx == rhs.zone_idx && this->segment_idx == rhs.segment_idx;
        }
        else
        {
            return this->controller == rhs.controller && this->zone_idx == rhs.zone_idx;
        }
    }

    bool operator<(ControllerZone const & rhs) const
    {
        if (is_segment)
        {
            return this->controller != rhs.controller || this->zone_idx != rhs.zone_idx || this->segment_idx != rhs.segment_idx;
        }
        else
        {
            // whatever
            return this->controller != rhs.controller || this->zone_idx != rhs.zone_idx;
        }
    }

    std::string display_name()
    {
        return controller->GetName() + ": " +  controller->GetZoneName(zone_idx) + (is_segment ? (" - " + controller->GetZoneSegmentName(zone_idx, segment_idx)) : "");
    }

    json to_json()
    {
        json j;
        j["zone_idx"]           = zone_idx;
        j["name"]               = controller->GetName();
        j["is_percentage_fill"] = is_percentage_fill;
        j["brightness"]         = brightness;
        j["reverse"]            = reverse;
        j["is_segment"]         = is_segment;
        j["segment_idx"]        = segment_idx;
        j["location"]           = controller->GetLocation();
        j["serial"]             = controller->GetSerial();
        j["description"]        = controller->GetDescription();
        j["version"]            = controller->GetVersion();
        j["vendor"]             = controller->GetVendor();
        return j;
    }
};

#endif // CONTROLLERZONE_H
