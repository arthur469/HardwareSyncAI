#include "HardwareMeasure.h"

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
    sysinfo(&info);
    return 100.0f * (info.totalram - info.freeram) / info.totalram;
}

#endif
