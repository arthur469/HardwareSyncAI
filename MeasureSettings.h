#ifndef MEASURESETTINGS_H
#define MEASURESETTINGS_H

#include "json.hpp"

using json = nlohmann::json;

class MeasureSettings
{
public:
    static bool Save(json);
    static json Load();

private:
    static bool CreateSettingsDirectory();

    static std::string folder_separator();
    static std::string SettingsFolder();

    static bool create_dir(std::string);
    static json load_json_file(std::string);
    static bool write_file(std::string, json);
};

#endif // MEASURESETTINGS_H
