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

#include <glibtop.h>
#include <glibtop/sysinfo.h>
#endif


enum TrackingTypes
{
    HWTYPE_USAGE_PERCENT,
    HWTYPE_TEMP_VALUE,
    HWTYPE_CPU_RATE,
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

#ifdef _WIN32
    int GetHardwareIndex(std::string identifier);
    int GetHardwareFeatureIndex(std::string identifier);
#endif

private:
    std::vector<Hardware> HardwareList;

    double GetTemperature(Hardware, HardwareFeature);
    double GetRamUsagePercent();
    double GetCPURate();

    #ifdef _WIN32
    std::map<std::string, int> IdentifierToHardwareDropdownIdx;
    std::map<std::string, int> IdentifierToFeatureDropdownIdx;

    #elif __linux__
    std::vector<chip_name> chips;
    std::vector<subfeature> subfeatures;
    struct sysinfo info;

    #endif

};

#endif // HARDWAREMEASURE
