#include "HardwareMeasure.h"
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <limits>
#include <algorithm>

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
            return std::numeric_limits<double>::quiet_NaN();
        }
    }

    return std::numeric_limits<double>::quiet_NaN();
}

#if defined(_WIN32) || defined(HARDWARE_SYNC_AI_COLLECTOR)
#include "TelemetryRuntime.h"
#include <limits>
HardwareMeasure::HardwareMeasure(){
    const std::vector<std::tuple<std::string,std::string,std::string>> features={
      {"CPU","Demand (%), package power or load fallback","cpu_demand_percent"},{"CPU","Usage (%)","cpu_usage_percent"},{"CPU","Package power (W), if available","cpu_power_w"},{"CPU","Temperature (C)","cpu_temp_c"},
      {"GPU","Power (W)","gpu_power_w"},{"GPU","Temperature (C)","gpu_temp_c"},{"GPU","Usage (%)","gpu_usage_percent"},
      {"RAM","Usage (%)","ram_usage_percent"},{"Fans","Case speed (rpm)","case_rpm"},{"Fans","Auxiliary fan speed (rpm), if configured","sl_rpm"},
      {"Thermal","Case thermal demand (%)","case_thermal_percent"},{"GPU","Power relative to limit (%)","gpu_power_percent"},
      {"Local AI (Ollama)","Model in GPU memory (0/1)","gpu_model"},{"Local AI (Ollama)","Model host allocation estimate (0/1)","ram_model"},
      {"Local AI (Ollama)","Model CPU/partial allocation estimate (0/1)","cpu_model"},{"Local AI (Ollama)","Loaded model count","model_count"},
      {"GPU","Memory used (MiB)","gpu_memory_mib"},{"GPU","Memory total (MiB)","gpu_memory_total_mib"},{"GPU","Fan speed (%)","gpu_fan_percent"}
    };
    for(const auto& row:features){
      auto name=std::get<0>(row);int h=-1;
      for(unsigned i=0;i<HardwareList.size();i++)if(HardwareList[i].TrackDeviceName==name)h=i;
      if(h<0){Hardware hw;hw.TrackDeviceName=name;hw.DeviceIndex=HardwareList.size();HardwareList.push_back(hw);h=HardwareList.size()-1;}
      const auto key=std::get<2>(row);HardwareFeature feature;feature.Name=std::get<1>(row);feature.Identifier=key;
      feature.GetMeasure=[key](){return telemetryRuntime().value(key);};
      IdentifierToHardwareDropdownIdx[key]=h;IdentifierToFeatureDropdownIdx[key]=HardwareList[h].features.size();HardwareList[h].features.push_back(feature);
    }
#ifdef __linux__
    AppendNativeSensors();
#endif
}
double HardwareMeasure::GetRamUsagePercent(){return telemetryRuntime().value("ram_usage_percent");}
double HardwareMeasure::GetCPURate(){return telemetryRuntime().value("cpu_usage_percent");}
int HardwareMeasure::GetHardwareIndex(std::string key){auto i=IdentifierToHardwareDropdownIdx.find(key);return i==IdentifierToHardwareDropdownIdx.end()?-1:i->second;}
int HardwareMeasure::GetHardwareFeatureIndex(std::string key){auto i=IdentifierToFeatureDropdownIdx.find(key);return i==IdentifierToFeatureDropdownIdx.end()?-1:i->second;}
#elif __linux__

HardwareMeasure::HardwareMeasure()
{
    std::vector<chip_name> detected_chips;

    try
    {
        detected_chips = get_detected_chips();
    }
    catch(const sensors::error& ex)
    {
        printf("[OpenRGBHardwareSyncPlugin] sensors::error during initialization: %s\n", ex.what());
        printf("[OpenRGBHardwareSyncPlugin] Hardware monitoring via sensors will be disabled.\n");
        return;
    }
    catch(const std::exception& ex)
    {
        printf("[OpenRGBHardwareSyncPlugin] std::exception during sensors initialization: %s\n", ex.what());
        printf("[OpenRGBHardwareSyncPlugin] Hardware monitoring via sensors will be disabled.\n");
        return;
    }

    printf("[OpenRGBHardwareSyncPlugin] detected chips: %d\n", (int)detected_chips.size());

    // it seems that devices are detected in random order, as a result on each load there is a shift in measures
    // so sort by name assuming constant hardware names
    std::sort(detected_chips.begin(), detected_chips.end(), 
        [](const chip_name &a, const chip_name &b)
              { return a.name() < b.name(); }
    );

    for (unsigned int ChipID = 0; ChipID < detected_chips.size(); ChipID++)
    {
        Hardware NewDev;

        try
        {
            NewDev.TrackDeviceName = detected_chips[ChipID].name();
        }
        catch(const sensors::io_error& ex)
        {
            printf("[OpenRGBHardwareSyncPlugin] sensors::io_error for chip[%d]: %s\n", ChipID, ex.what());
            continue;
        }

        NewDev.DeviceIndex = ChipID;

        const chip_name& chip = detected_chips[ChipID];

        try
        {
            const auto& features = chip.features();
            for (unsigned int ChipFeatureID = 0; ChipFeatureID < features.size(); ChipFeatureID++)
            {
                feature FTR = features[ChipFeatureID];
                std::string BaseFeatureName = (std::string)FTR.label();

                const auto& subfeatures = FTR.subfeatures();
                for(int SubFeatureID = 0; SubFeatureID < (int)subfeatures.size(); SubFeatureID++)
                {
                    subfeature SubFTR = subfeatures[SubFeatureID];

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
                                printf("[OpenRGBHardwareSyncPlugin] sensors::io_error while reading value: %s\n", ex.what());
                                return 0.;
                            }
                        };

                        NewDev.features.push_back(feature);
                    }
                }
            }
        }
        catch(const sensors::error& ex)
        {
            printf("[OpenRGBHardwareSyncPlugin] sensors::error while processing chip[%d]: %s\n", ChipID, ex.what());
            continue;
        }
        catch(const std::exception& ex)
        {
            printf("[OpenRGBHardwareSyncPlugin] std::exception while processing chip[%d]: %s\n", ChipID, ex.what());
            continue;
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
        printf("[OpenRGBHardwareSyncPlugin] GPU not found\n");
        return;
    }
    else
    {
        printf("[OpenRGBHardwareSyncPlugin] GPU found: %s\n", result.c_str());
    }

    // extract GPU name and index
    std::string gpu_name = result.substr(0, result.find(","));
    std::string gpu_index = result.substr(result.find(",") + 1, result.length());

    printf("[OpenRGBHardwareSyncPlugin] gpu_index: %s\n", gpu_index.c_str());

    Hardware GPU;

    GPU.TrackDeviceName = gpu_name;

    try
    {
        GPU.DeviceIndex = std::stoi(gpu_index);
    }
    catch(const std::invalid_argument& e)
    {
        printf("[OpenRGBHardwareSyncPlugin] ERROR: %s\n", e.what());
        return;
    }

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

#if defined(__linux__) && defined(HARDWARE_SYNC_AI_COLLECTOR)
#include "sensors.h"
#include <cstdlib>
void HardwareMeasure::AppendNativeSensors()
{
    // Preserve upstream's generic libsensors discovery alongside the AI extension.
    try {
        auto chips=sensors::get_detected_chips();
        std::sort(chips.begin(),chips.end(),[](const sensors::chip_name& a,const sensors::chip_name& b){return a.name()<b.name();});
        for(const auto& chip:chips) {
            Hardware hardware;hardware.TrackDeviceName=chip.name();hardware.DeviceIndex=HardwareList.size();
            for(const auto& feature:chip.features())for(const auto& input:feature.subfeatures()) {
                if(input.type()!=sensors::subfeature_type::input)continue;
                HardwareFeature metric;metric.Name=feature.label()+" ("+input.name()+")";
                metric.Identifier="sensor:"+chip.name()+"/"+input.name();
                metric.GetMeasure=[input](){try{return input.read();}catch(...){return std::numeric_limits<double>::quiet_NaN();}};
                IdentifierToHardwareDropdownIdx[metric.Identifier]=HardwareList.size();
                IdentifierToFeatureDropdownIdx[metric.Identifier]=hardware.features.size();hardware.features.push_back(metric);
            }
            if(!hardware.features.empty())HardwareList.push_back(hardware);
        }
    } catch(const std::exception&) { /* Other system/AI metrics remain available. */ }
    Hardware load;load.TrackDeviceName="Load Averages";load.DeviceIndex=HardwareList.size();
    for(int i=0;i<3;++i) {
        HardwareFeature feature;feature.Name=std::string("Load ")+(i==0?"1":i==1?"5":"15")+" min";
        feature.Identifier="load_average_"+std::to_string(i);
        feature.GetMeasure=[i](){double values[3];return getloadavg(values,3)==3?values[i]:std::numeric_limits<double>::quiet_NaN();};
        IdentifierToHardwareDropdownIdx[feature.Identifier]=HardwareList.size();IdentifierToFeatureDropdownIdx[feature.Identifier]=load.features.size();load.features.push_back(feature);
    }
    HardwareList.push_back(load);
}
#endif
