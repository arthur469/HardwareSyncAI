#include "HardwareMeasure.h"
#include <stdio.h>
#include <string.h>
#include <iostream>

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

double HardwareMeasure::GetMeasure(const Hardware& /*Dev*/, const HardwareFeature& FTR)
{
    if (FTR.GetMeasure != nullptr)
    {
        try
        {
            return FTR.GetMeasure();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return 0.0;
        }
    }

    return 0.0f;
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
            const auto identifier = std::get<2>(tup);
            feature.Identifier = identifier;
            feature.GetMeasure = [identifier](){return LHWM::GetSensorValue(identifier);};

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
    printf("## HardwareMeasure chips: %d\n", (int)detected_chips.size());

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
                    feature.GetMeasure = [this, chip, ChipFeatureID, SubFeatureID]()
                    {
                        try
                        {
                            return chip.features()[ChipFeatureID].subfeatures()[SubFeatureID].read();
                        }
                        catch (const sensors::io_error &ex)
                        {
                            printf("##### sensors::io_error while reading value: %s\n", ex.what());
                            return 0.;
                        }
                    };

                    NewDev.features.push_back(feature);
                }
            }
        }

        if (NewDev.features.size() > 0)
        {
            HardwareList.push_back(NewDev);
        }
    }

    Hardware RamDev;
    RamDev.TrackDeviceName = "Ram usage";
    RamDev.DeviceIndex = 0;

    HardwareFeature feature;
    feature.Name = "Percent usage";
    feature.GetMeasure = [this](){return GetRamUsagePercent();};
    RamDev.features.push_back(feature);

    HardwareList.push_back(RamDev);

    Hardware CPU;
    CPU.TrackDeviceName = "CPU";
    CPU.DeviceIndex = 0;

    HardwareFeature cpu_rate_feature;
    cpu_rate_feature.Name = "CPU rate";
    cpu_rate_feature.GetMeasure = [this](){return GetCPURate();};

    CPU.features.push_back(cpu_rate_feature);

    HardwareList.push_back(CPU);

    AddNvidiaGPUMeasurements();
    AddLoadAverages();
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

void HardwareMeasure::AddNvidiaGPUMeasurements()
{
    char line[128];
    std::string result;
    std::string cmd = "nvidia-smi --query-gpu=gpu_name,index --format=csv,noheader";

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        return;
    }

    while (!feof(pipe))
    {
        if (fgets(line, 128, pipe) != NULL)
        {
            result += line;
        }
    }

    pclose(pipe);

    if (result.empty())
    {
        printf("GPU not found\n");
        return;
    }
    else
    {
        printf("GPU found: %s\n", result.c_str());
    }

    // extract GPU name and index
    std::string gpu_name = result.substr(0, result.find(","));
    std::string gpu_index = result.substr(result.find(",") + 1, result.length());

    Hardware GPU;
    GPU.TrackDeviceName = gpu_name;
    GPU.DeviceIndex = std::stoi(gpu_index);

    // create list of features and query tags
    std::vector<std::string> features = {"temperature.gpu", "utilization.gpu", "utilization.memory", "memory.used", "memory.free"};
    
    for (auto feature : features)
    {
        HardwareFeature gpu_feature;
        gpu_feature.Name = feature;

        gpu_feature.GetMeasure = [gpu_index, feature]()
        {
            char line[128];
            std::string result;
            std::string cmd = "nvidia-smi --query-gpu=" + feature + " --format=csv,noheader,nounits -i " + gpu_index;

            FILE *pipe = popen(cmd.c_str(), "r");
            if (!pipe)
                return 0.0;

            while (!feof(pipe))
            {
                if (fgets(line, 128, pipe) != NULL)
                {
                    result += line;
                }
            }

            pclose(pipe);

            if (result.empty())
            {
                // printf("GPU temperature not found\n");
                return 0.0;
            }
            else
            {
                // printf("GPU temperature found: %s\n", result.c_str());
            }

            return std::stod(result);
        };

        GPU.features.push_back(gpu_feature);
    }

    HardwareList.push_back(GPU);
}

void HardwareMeasure::AddLoadAverages()
{
    Hardware LoadAverages;
    LoadAverages.TrackDeviceName = "Load Averages";
    LoadAverages.DeviceIndex = 0;

    HardwareFeature load_1min_feature;
    load_1min_feature.Name = "Load 1min";
    load_1min_feature.GetMeasure = [this](){return GetLoadAverage(0);};

    HardwareFeature load_5min_feature;
    load_5min_feature.Name = "Load 5min";
    load_5min_feature.GetMeasure = [this](){return GetLoadAverage(1);};

    HardwareFeature load_15min_feature;
    load_15min_feature.Name = "Load 15min";
    load_15min_feature.GetMeasure = [this](){return GetLoadAverage(2);};

    LoadAverages.features.push_back(load_1min_feature);
    LoadAverages.features.push_back(load_5min_feature);
    LoadAverages.features.push_back(load_15min_feature);

    HardwareList.push_back(LoadAverages);
}

double HardwareMeasure::GetLoadAverage(int index)
{
    double loadavg[3];
    if (getloadavg(loadavg, 3) != -1)
    {
        return loadavg[index];
    }
    else
    {
        return 0.0;
    }
}

#endif
