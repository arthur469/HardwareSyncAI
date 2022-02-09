#include "MeasureSettings.h"
#include <fstream>
#include <iostream>
#include <QFile>
#include <QString>
#include <QDir>
#include "OpenRGBHardwareSyncPlugin.h"

bool MeasureSettings::Save(json j)
{
    printf("[OpenRGBHardwareSyncPlugin] Saving file.\n");

    if(!CreateSettingsDirectory())
    {
        printf("[OpenRGBHardwareSyncPlugin] Cannot create settings directory.\n");
        return false;
    }

    return write_file(SettingsFolder() + folder_separator() + "MeasureSettings.json", j);
}

json MeasureSettings::Load()
{
    printf("[OpenRGBHardwareSyncPlugin] Loading file.\n");

    json Settings;

    if(!CreateSettingsDirectory())
    {
        return Settings;
    }

    return load_json_file(SettingsFolder() + folder_separator() + "MeasureSettings.json");
}


std::string MeasureSettings::folder_separator()
{
#if defined(WIN32) || defined(_WIN32)
    return "\\";
#else
    return "/";
#endif
}

bool MeasureSettings::write_file(std::string file_name, json j)
{
    std::ofstream file(file_name, std::ios::out | std::ios::binary);

    if(file)
    {
        try
        {
            file << j.dump(4);
            file.close();
        }
        catch(const std::exception& e)
        {
            printf("[OpenRGBHardwareSyncPlugin] Cannot write file: %s\n", e.what());
            return false;
        }
    }

    return true;
}

json MeasureSettings::load_json_file(std::string file_name)
{
    json j;

    std::ifstream file(file_name);

    if(file)
    {
        try
        {
            file >> j;
            file.close();
        }
        catch(const std::exception& e)
        {
             printf("[OpenRGBHardwareSyncPlugin] Cannot read file: %s\n", e.what());
        }
    }

    return j;
}

bool MeasureSettings::create_dir(std::string directory)
{
    QDir dir(QString::fromStdString(directory));

    if(dir.exists())
    {
        return true;
    }

    return QDir().mkpath(dir.path());
}


std::string MeasureSettings::SettingsFolder()
{
    return OpenRGBHardwareSyncPlugin::RMPointer->GetConfigurationDirectory() + "plugins" + folder_separator() + "settings";
}

bool MeasureSettings::CreateSettingsDirectory()
{
     return create_dir(SettingsFolder());
}
