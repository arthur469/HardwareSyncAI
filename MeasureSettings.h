#ifndef MEASURESETTINGS_H
#define MEASURESETTINGS_H

#include "json.hpp"

using json = nlohmann::json;

class MeasureSettings
{
public:
    static void Save(json);
    static json Load();

private:
    static bool CreateSettingsDirectory();

    static const std::string SettingsFolder;
    static const std::string SettingsFileName;
};

#endif // MEASURESETTINGS_H
