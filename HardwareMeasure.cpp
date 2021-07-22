#include "HardwareMeasure.h"

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

    for (auto const& [key, val] : hs_map)
    {
        Hardware hw;
        hw.TrackDeviceName = key;

        for(auto const& tup : val)
        {
            HardwareFeature feature;
            feature.Name = std::get<0>(tup) + " (" + std::get<1>(tup) + ")";
            feature.ValueType = HWTYPE_TEMP_VALUE;
            feature.Identifier = std::get<2>(tup);
            feature.PrimaryFeatureIndex = 0;
            feature.SubFeatureIndex = 0;

            hw.features.push_back(feature);
        }

        HardwareList.push_back(hw);
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

#elif __linux__

HardwareMeasure::HardwareMeasure()
{
    chips = get_detected_chips();

    for (int ChipID = 0; ChipID < (int)chips.size(); ChipID++)
    {
        Hardware NewDev;
        NewDev.TrackDeviceName = chips[ChipID].name();
        NewDev.DeviceIndex = ChipID;

        chip_name chip = chips[ChipID];
        for (int ChipFeatureID = 0; ChipFeatureID < (int)chip.features().size(); ChipFeatureID++)
        {
            feature FTR = chip.features()[ChipFeatureID];
            std::string BaseFeatureName = (std::string)FTR.name();

            for(int SubFeatureID = 0; SubFeatureID < (int)FTR.subfeatures().size(); SubFeatureID++)
            {
                subfeature SubFTR = FTR.subfeatures()[SubFeatureID];
                if(SubFTR.type() == subfeature_type::input)
                {
                    std::string FTRName = (BaseFeatureName + std::string(SubFTR.name()).c_str());
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
}

double HardwareMeasure::GetMeasure(Hardware Dev, HardwareFeature FTR)
{
    if(FTR.ValueType == HWTYPE_TEMP_VALUE)
    {
        return GetTemperature(Dev, FTR);
    }

    if(FTR.ValueType == HWTYPE_USAGE_PERCENT)
    {
        return GetRamUsagePercent();
    }

    return 0.0f;
}

double HardwareMeasure::GetTemperature(Hardware Dev, HardwareFeature FTR)
{
    return chips[Dev.DeviceIndex].features()[FTR.PrimaryFeatureIndex].subfeatures()[FTR.SubFeatureIndex].read();
}

double HardwareMeasure::GetRamUsagePercent()
{
    sysinfo(&info);
    return 100.0f * (info.totalram - info.freeram) / info.totalram;
}

#endif
