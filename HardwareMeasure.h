#ifndef HARDWAREMEASURE_H
#define HARDWAREMEASURE_H

#pragma once

#include <vector>
#include <string>
#include "RGBControllerInterface.h"
#include <map>
#include <functional>

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


struct HardwareFeature
{
    std::string Name;
    std::string Identifier;
    std::function<double()> GetMeasure = nullptr;
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
    double GetMeasure(const Hardware&, const HardwareFeature&);

#ifdef _WIN32
    int GetHardwareIndex(std::string identifier);
    int GetHardwareFeatureIndex(std::string identifier);
#endif

private:
    std::vector<Hardware> HardwareList;

    double GetRamUsagePercent();
    double GetCPURate();

    #ifdef _WIN32
    std::map<std::string, int> IdentifierToHardwareDropdownIdx;
    std::map<std::string, int> IdentifierToFeatureDropdownIdx;

    #elif __linux__
    int read_meminfo(int*, int*);
    struct sysinfo info;

    // Deteect GPU and add it to the list if present
    void AddNvidiaGPUMeasurements();

    // Add system load averages
    void AddLoadAverages();
    double GetLoadAverage(int);
    #endif
};

#endif // HARDWAREMEASURE
