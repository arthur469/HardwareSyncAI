#include "HardwareMeasure.h"
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include "error.h"
#endif

HardwareMeasure::~HardwareMeasure()
{

}

std::vector<Hardware> HardwareMeasure::GetAvailableDevices()
{
    return HardwareList;
}

#ifdef _WIN32

HardwareMeasure::HardwareMeasure()
{    
    std::map<std::string, std::vector<std::tuple<std::string, std::string, std::string>>>
            hs_map = LHWM::GetHardwareSensorMap();

    int hardwareIdx = 0;
    for (auto const& [key, val] : hs_map)
    {
        Hardware hw;
        hw.TrackDeviceName = key;

        int featureIdx = 0;

        for(auto const& tup : val)
        {
            HardwareFeature feature;
            feature.Name = std::get<0>(tup) + " (" + std::get<1>(tup) + ")";
            feature.ValueType = HWTYPE_TEMP_VALUE;
            feature.Identifier = std::get<2>(tup);
            feature.PrimaryFeatureIndex = 0;
            feature.SubFeatureIndex = 0;

            hw.features.push_back(feature);

            IdentifierToHardwareDropdownIdx[feature.Identifier] = hardwareIdx;
            IdentifierToFeatureDropdownIdx[feature.Identifier] = featureIdx;
            featureIdx++;
        }

        if (hw.features.size() > 0)
        {
            HardwareList.push_back(hw);
            hardwareIdx++;
        }
    }

}

double HardwareMeasure::GetMeasure(Hardware Dev, HardwareFeature FTR)
{
    return LHWM::GetSensorValue(FTR.Identifier);
}

double HardwareMeasure::GetTemperature(Hardware Dev, HardwareFeature Feature)
{
    return 0.0f;
}

double HardwareMeasure::GetRamUsagePercent()
{
    return 0.0f;
}

double HardwareMeasure::GetCPURate()
{
    return 0.0f;
}

int HardwareMeasure::GetHardwareIndex(std::string identifier)
{
    return IdentifierToHardwareDropdownIdx[identifier];
}

int HardwareMeasure::GetHardwareFeatureIndex(std::string identifier)
{
    return IdentifierToFeatureDropdownIdx[identifier];
}

#elif __linux__

HardwareMeasure::HardwareMeasure()
{
    std::vector<chip_name> detected_chips = get_detected_chips();

    chips.clear();

    printf("## HardwareMeasure chips: %d\n", (int)chips.size());

    for (unsigned int ChipID = 0; ChipID < detected_chips.size(); ChipID++)
    {
        Hardware NewDev;

        try
        {
            NewDev.TrackDeviceName = detected_chips[ChipID].name();
        }
        catch(const sensors::io_error& ex)
        {
            printf("##### sensors::io_error for chip[%d]: %s\n", ChipID, ex.what());
            continue;
        }

        NewDev.DeviceIndex = ChipID;

        const chip_name& chip = detected_chips[ChipID];

        for (unsigned int ChipFeatureID = 0; ChipFeatureID < chip.features().size(); ChipFeatureID++)
        {
            feature FTR = chip.features()[ChipFeatureID];
            std::string BaseFeatureName = (std::string)FTR.label();

            for(int SubFeatureID = 0; SubFeatureID < (int)FTR.subfeatures().size(); SubFeatureID++)
            {
                subfeature SubFTR = FTR.subfeatures()[SubFeatureID];

                if(SubFTR.type() == subfeature_type::input)
                {
                    std::string FTRName;
                    if (SubFeatureID == 0)
                        FTRName = (BaseFeatureName);
                    else
                        FTRName = (BaseFeatureName + " "+ std::string(SubFTR.name().c_str()));
                    HardwareFeature feature;
                    feature.Name = FTRName;
                    feature.PrimaryFeatureIndex = ChipFeatureID;
                    feature.SubFeatureIndex = SubFeatureID;
                    feature.ValueType = HWTYPE_TEMP_VALUE;

                    NewDev.features.push_back(feature);
                }
            }
        }

        if (NewDev.features.size() > 0)
        {
            HardwareList.push_back(NewDev);
            chips.push_back(detected_chips[ChipID]);
        }
    }


    Hardware RamDev;
    RamDev.TrackDeviceName = "Ram usage";
    RamDev.DeviceIndex = 0;

    HardwareFeature feature;
    feature.Name = "Percent usage";
    feature.ValueType = HWTYPE_USAGE_PERCENT;
    feature.PrimaryFeatureIndex = 0;
    feature.SubFeatureIndex = 0;
    RamDev.features.push_back(feature);

    HardwareList.push_back(RamDev);

    Hardware CPU;
    CPU.TrackDeviceName = "CPU";
    CPU.DeviceIndex = 0;

    HardwareFeature cpu_rate_feature;
    cpu_rate_feature.Name = "CPU rate";
    cpu_rate_feature.ValueType = HWTYPE_CPU_RATE;
    cpu_rate_feature.PrimaryFeatureIndex = 0;
    cpu_rate_feature.SubFeatureIndex = 0;

    CPU.features.push_back(cpu_rate_feature);

    HardwareList.push_back(CPU);
}

double HardwareMeasure::GetMeasure(Hardware Dev, HardwareFeature FTR)
{
    switch (FTR.ValueType) {

    case HWTYPE_TEMP_VALUE:
            return GetTemperature(Dev, FTR);
        break;

    case HWTYPE_USAGE_PERCENT:
            return GetRamUsagePercent();
        break;

    case HWTYPE_CPU_RATE:
            return GetCPURate();
        break;

    default:
        break;
    }

    return 0.0f;
}

double HardwareMeasure::GetTemperature(Hardware Dev, HardwareFeature FTR)
{
    try
    {
        return chips[Dev.DeviceIndex].features()[FTR.PrimaryFeatureIndex].subfeatures()[FTR.SubFeatureIndex].read();
    }
    catch(const sensors::io_error& ex)
    {
        printf("##### sensors::io_error while reading value: %s\n", ex.what());
        return 0.;
    }
}

double HardwareMeasure::GetRamUsagePercent()
{

    int MemTotal = 0;
    int MemAvailable = 0;

    int ret = read_meminfo(&MemTotal, &MemAvailable);

    if(ret == 0)
    {
        if(MemTotal > 0)
        {
            return 100.0f * (MemTotal - MemAvailable) / MemTotal;
        }

        return 0.;
    }
    else
    {
        sysinfo(&info);
        return 100.0f * (info.totalram - info.freeram) / info.totalram;
    }
}

int HardwareMeasure::read_meminfo(int *MemTotal, int *MemAvailable)
{
    FILE *file_handle;
    const char * meminfo_file ="/proc/meminfo";
    char buffer[128];
    int len = 0;
    char *p1;
    file_handle = fopen(meminfo_file,"r");

    if (file_handle == NULL)
    {
        return -1;
    }

    while ( fgets(buffer,sizeof(buffer) - 1, file_handle) != NULL)
    {
        p1 = strchr(buffer, ':');

        if (! strncmp(buffer,"MemTotal", p1 - buffer))
        {
            char mem_total[256];
            sscanf(p1+1,"%s%*s",mem_total);
            len = strlen(mem_total);
            *(mem_total+len) = ' ';
            sscanf(p1+1,"%*s%s",mem_total+len+1);
            sscanf(mem_total, "%d kB", MemTotal);
        }
        else if (! strncmp(buffer,"MemAvailable", p1 - buffer))
        {
            char mem_available[256];
            sscanf(p1,"%*s%s",mem_available);
            len = strlen(mem_available);
            *(mem_available+len) = ' ';
            sscanf(p1+1,"%*s%s",mem_available+len+1);
            sscanf(mem_available, "%d kB", MemAvailable);
            break;
        }
    }
    fclose(file_handle);
    return 0;
}

static float last_total = 0, last_used = 0;

double HardwareMeasure::GetCPURate()
{
    glibtop_cpu cpu;

    unsigned long int used = 0, dt = 0;

    double cpu_rate = 0;

    glibtop_get_cpu(&cpu);

    used = cpu.user + cpu.nice + cpu.sys;

    dt = cpu.total - last_total;

    if (dt)
        cpu_rate = 100.0 * (used - last_used) / dt;
    else
        cpu_rate = 0;

    last_used = used;
    last_total = cpu.total;

    return cpu_rate;
}

#endif
