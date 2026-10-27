// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <algorithm>
#include <cctype>
#include <regex>
#include <string>
#include <nlohmann/json.hpp>
namespace TargetIdentity {
using json=nlohmann::json;
inline std::string location(std::string value) {
#ifdef __linux__
    value=std::regex_replace(value,std::regex("/dev/hidraw[0-9]+"),"/dev/hidraw*");
    value=std::regex_replace(value,std::regex("/dev/i2c-[0-9]+"),"/dev/i2c-*");
#endif
    return value;
}
inline bool hasSerial(const json& value) {
    auto serial=value.value("serial",std::string());
    std::transform(serial.begin(),serial.end(),serial.begin(),[](unsigned char c){return std::tolower(c);});
    return !serial.empty() && serial!="unknown" && serial!="n/a" && serial!="none" &&
        serial!="default" && serial!="0" && serial.find_first_not_of("0 -:")!=std::string::npos;
}
inline bool sameDevice(const json& saved,const json& live) {
    if(saved.value("name",std::string())!=live.value("name",std::string()) ||
       saved.value("vendor",std::string())!=live.value("vendor",std::string())) return false;
    if(hasSerial(saved) || hasSerial(live))
        return hasSerial(saved) && hasSerial(live) && saved["serial"]==live["serial"];
    auto path=location(saved.value("location",std::string()));
    return !path.empty() && path==location(live.value("location",std::string()));
}
inline bool sameZone(const json& saved,const json& live) {
    if(saved.value("is_segment",false)!=live.value("is_segment",false))return false;
    if(saved.contains("zone_name") && !saved["zone_name"].get<std::string>().empty()) {
        if(saved["zone_name"]!=live["zone_name"])return false;
    } else if(saved.value("zone_idx",-1)!=live.value("zone_idx",-1))return false;
    if(saved.contains("leds_count") && saved["leds_count"]!=live["leds_count"])return false;
    if(saved.value("is_segment",false)) {
        if(saved.contains("segment_name") && !saved["segment_name"].get<std::string>().empty())
            return saved["segment_name"]==live.value("segment_name",std::string());
        return saved.value("segment_idx",-1)==live.value("segment_idx",-1);
    }
    return true;
}
}
