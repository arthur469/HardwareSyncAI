#ifndef CONTROLLERZONE_H
#define CONTROLLERZONE_H

#include <nlohmann/json.hpp>
#include "RGBController.h"

using json = nlohmann::json;

class ControllerZone
{
public:
    ControllerZone(RGBController* controller,
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

    RGBController* controller;
    unsigned int zone_idx;
    bool enabled;
    bool is_percentage_fill;
    int brightness;
    bool reverse;
    bool is_segment;
    int segment_idx;

    zone_type type()
    {
        return is_segment ? controller->zones[zone_idx].segments[segment_idx].type : controller->zones[zone_idx].type;
    }

    unsigned int start_idx()
    {
        return is_segment ? controller->zones[zone_idx].segments[segment_idx].start_idx : controller->zones[zone_idx].start_idx;
    }

    unsigned int leds_count()
    {
        return is_segment ? controller->zones[zone_idx].segments[segment_idx].leds_count : controller->zones[zone_idx].leds_count;
    }

    unsigned int matrix_map_width()
    {
        return controller->zones[zone_idx].matrix_map->width;
    }

    unsigned int matrix_map_height()
    {
        return controller->zones[zone_idx].matrix_map->height;
    }

    unsigned int* map()
    {
        return controller->zones[zone_idx].matrix_map->map;
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
        return controller->name + ": " +  controller->zones[zone_idx].name + (is_segment ? (" - " + controller->zones[zone_idx].segments[segment_idx].name) : "");
    }

    json to_json()
    {
        json j;
        j["zone_idx"] = zone_idx;
        j["name"] = controller->name;
        j["is_percentage_fill"] = is_percentage_fill;
        j["brightness"] = brightness;
        j["reverse"] = reverse;
        j["is_segment"] = is_segment;
        j["segment_idx"] = segment_idx;
        j["location"] = controller->location;
        j["serial"] = controller->serial;
        j["description"] = controller->description;
        j["version"] = controller->version;
        j["vendor"] = controller->vendor;
        return j;
    }
};

#endif // CONTROLLERZONE_H
