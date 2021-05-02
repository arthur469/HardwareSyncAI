#ifndef HARDWAREMEASURE_H
#define HARDWAREMEASURE_H

#pragma once

#include <vector>
#include <string>
#include "RGBController.h"
#include <map>

#ifdef _WIN32
#include <lhwm-cpp-wrapper.h>
#include <QString>
#include <QFile>
#include <QCoreApplication>
#elif __linux__

#include <sys/sysinfo.h>
#include "sensors.h"

using namespace sensors;
#endif


enum TrackingTypes
{
    HWTYPE_USAGE_PERCENT,
    HWTYPE_TEMP_VALUE,
};

struct HardwareFeature
{
    std::string     Name;
    TrackingTypes   ValueType;
    int PrimaryFeatureIndex;
    int SubFeatureIndex;
    std::string Identifier;
};

struct Hardware
{
    std::string TrackDeviceName;
    std::vector<HardwareFeature> features;
    int DeviceIndex;
};

class HardwareMeasure
{
public:

    HardwareMeasure();
    ~HardwareMeasure();

    std::vector<Hardware> GetAvailableDevices();
    double GetMeasure(Hardware, HardwareFeature);

private:
    std::vector<Hardware> HardwareList;

    double GetTemperature(Hardware, HardwareFeature);
    double GetRamUsagePercent();

    #ifdef _WIN32

    #elif __linux__
    std::vector<chip_name> chips;
    std::vector<subfeature> subfeatures;
    std::map<int , chip_name , std::vector<feature>> FTRList;
    struct sysinfo info;

    #endif

};

#endif // HARDWAREMEASURE
