#ifndef MEASURESETTINGS_H
#define MEASURESETTINGS_H

#include <nlohmann/json.hpp>
#include "filesystem.h"

using json = nlohmann::json;

class MeasureSettings
{
public:
    static bool Save(json);
    static json Load();

private:
    static bool CreateSettingsDirectory();
    static filesystem::path SettingsFolder();

    static bool create_dir(filesystem::path);
    static json load_json_file(filesystem::path);
    static bool write_file(filesystem::path, json);
};

#endif // MEASURESETTINGS_H
