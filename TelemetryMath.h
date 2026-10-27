#pragma once
#include <cmath>
#include <algorithm>
#include <string>
namespace TelemetryMath {
inline double envelope(const std::string& effect,double seconds,double period,double floor) {
    if(!std::isfinite(seconds)||!std::isfinite(period)||period<2||period>30||!std::isfinite(floor)||floor<.1||floor>1) return 1;
    const double phase=seconds/period-std::floor(seconds/period);
    if(effect=="breathing") return floor+(1-floor)*(.5-.5*std::cos(phase*6.283185307179586));
    if(effect=="pulse") return floor+(1-floor)*std::exp(-7*phase);
    return 1;
}
inline bool supportedMode(const std::string& name,unsigned flags) {
    return name=="Direct" || (name=="Custom" && (flags&0x20));
}
}
