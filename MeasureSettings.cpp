#include "MeasureSettings.h"
#include <fstream>
#include <iostream>
#include "filesystem.h"
#include "OpenRGBHardwareSyncPlugin.h"

const std::string MeasureSettings::SettingsFolder = "/plugins/settings/";
const std::string MeasureSettings::SettingsFileName = "MeasureSettings.json";

void MeasureSettings::Save(json Settings)
{
    if(!CreateSettingsDirectory())
    {
        printf("[OpenRGBHardwareSyncPlugin] Cannot create settings directory.\n");
        return;
    }

    std::ofstream File((OpenRGBHardwareSyncPlugin::RMPointer->GetConfigurationDirectory() + SettingsFolder + SettingsFileName), std::ios::out | std::ios::binary);

    if(File)
    {
        try{
            File << Settings.dump(4);
        }
        catch(const std::exception& e)
        {
            printf("[OpenRGBHardwareSyncPlugin] Cannot write settings: %s\n", e.what());
        }
        File.close();
    }
}

json MeasureSettings::Load()
{
    json Settings;

    std::ifstream SFile(OpenRGBHardwareSyncPlugin::RMPointer->GetConfigurationDirectory() + SettingsFolder + SettingsFileName, std::ios::in | std::ios::binary);

    if(SFile)
    {
        try
        {
            SFile >> Settings;
            SFile.close();
        }
        catch(const std::exception& e)
        {
            printf("[OpenRGBHardwareSyncPlugin] Cannot read settings: %s\n", e.what());
        }
    }

    return Settings;
}

bool MeasureSettings::CreateSettingsDirectory()
{
    std::string directory = OpenRGBHardwareSyncPlugin::RMPointer->GetConfigurationDirectory() + SettingsFolder;

    if(filesystem::exists(directory))
    {
        return true;
    }

    return filesystem::create_directory(directory);
}
