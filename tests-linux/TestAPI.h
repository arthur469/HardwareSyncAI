#pragma once
#include "OpenRGBPluginInterface.h"
class TestAPI: public OpenRGBPluginAPIInterface {public: filesystem::path configDirectory; nlohmann::json settings; std::vector<RGBControllerInterface*> controllers;
    void LogEntry(const char* filename, int line, unsigned int level, const char* fmt, ...) override {return;}
    RGBControllerInterface* CreateVirtualRGBController(RGBController_Setup* setup) override {return {};}
    void DeleteVirtualRGBController(RGBControllerInterface* rgb_controller) override {return;}
    void RegisterVirtualRGBController(RGBControllerInterface* rgb_controller) override {return;}
    void RegisterVirtualRGBControllerInThread(RGBControllerInterface* rgb_controller) override {return;}
    void UnregisterVirtualRGBController(RGBControllerInterface* rgb_controller) override {return;}
    void UpdateVirtualRGBController(RGBControllerInterface* rgb_controller, RGBController_Setup* setup) override {return;}
    void UnregisterVirtualRGBControllerInThread(RGBControllerInterface* rgb_controller) override {return;}
    void ClearActiveProfile() override {return;}
    std::vector<std::string> GetProfileList() override {return {};}
    bool LoadProfile(std::string profile_name) override {return {};}
    bool SaveProfileFromPlugin(std::string profile_name, std::string plugin_name, nlohmann::json plugin_data) override {return {};}
    filesystem::path GetConfigurationDirectory() override {return configDirectory;}
    bool GetDetectionEnabled() override {return {};}
    unsigned int GetDetectionPercent() override {return {};}
    std::string GetDetectionString() override {return {};}
    void RescanDevices() override {return;}
    void WaitForDetection() override {return;}
    std::vector<RGBControllerInterface*> GetRGBControllers() override {return controllers;}
    nlohmann::json GetDeviceDescriptionJSON(RGBControllerInterface* controller) override {return {};}
    nlohmann::json GetLEDDescriptionJSON(led led) override {return {};}
    nlohmann::json GetMatrixMapDescriptionJSON(matrix_map_type matrix_map) override {return {};}
    nlohmann::json GetModeDescriptionJSON(mode mode) override {return {};}
    nlohmann::json GetSegmentDescriptionJSON(segment segment) override {return {};}
    nlohmann::json GetZoneDescriptionJSON(zone zone) override {return {};}
    RGBControllerInterface* SetDeviceDescriptionJSON(nlohmann::json controller_json) override {return {};}
    led SetLEDDescriptionJSON(nlohmann::json led_json) override {return {};}
    matrix_map_type SetMatrixMapDescriptionJSON(nlohmann::json matrix_map_json) override {return {};}
    mode SetModeDescriptionJSON(nlohmann::json mode_json) override {return {};}
    segment SetSegmentDescriptionJSON(nlohmann::json segment_json) override {return {};}
    zone SetZoneDescriptionJSON(nlohmann::json zone_json) override {return {};}
    bool CompareControllers(RGBControllerInterface* controller_1, RGBControllerInterface* controller_2) override {return {};}
    std::string DeviceTypeToString(device_type type) override {return {};}
    bool SetModeValuesFromMode(mode& destination, mode& source) override {return {};}
    nlohmann::json GetSettings(std::string settings_key) override {return settings;}
    void SaveSettings() override {return;}
    void SetSettings(std::string settings_key, nlohmann::json new_settings) override {settings=new_settings;}
};
