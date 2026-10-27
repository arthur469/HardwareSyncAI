#pragma once
#include "RGBControllerInterface.h"
class TestController: public RGBControllerInterface {public: unsigned active=0,light=100,updates=0,modeChanges=0;std::vector<unsigned> counts={0,2,2};std::vector<RGBColor> colors={0xFF00,0xFF00,0xFF00,0xFF00};
    bool hidden=false;std::string alias;std::string serial="test",deviceName="Test Controller",vendor="Test",description="Inert test controller",version="1";std::vector<std::string> zoneNames;std::vector<std::string> modeNames={"Direct","Off","Static","Custom"};std::vector<std::string> zoneAliases;std::vector<std::vector<std::pair<std::string,unsigned>>> segments;
    std::string GetName() override {return deviceName;}
    std::string GetVendor() override {return vendor;}
    std::string GetDescription() override {return description;}
    std::string GetVersion() override {return version;}
    std::string GetSerial() override {return serial;}
    std::string location="test-location";
    std::string GetLocation() override {return location;}
    std::string GetDisplayName() override {return alias;}
    device_type GetDeviceType() override {return {};}
    controller_flags GetFlags() override {return {};}
    bool GetHidden() override {return hidden;}
    void SetHidden(bool hidden) override {return;}
    zone GetZone(unsigned int zone_idx) override {return {};}
    int GetZoneActiveMode(unsigned int zone) override {return {};}
    RGBColor GetZoneColor(unsigned int zone, unsigned int color_index) override {return colors.at(GetZoneStartIndex(zone)+color_index);}
    RGBColor* GetZoneColorsPointer(unsigned int zone) override {return {};}
    unsigned int GetZoneCount() override {return counts.size();}
    std::string GetZoneDisplayName(unsigned int zone) override {return zone<zoneAliases.size()?zoneAliases[zone]:std::string();}
    zone_flags GetZoneFlags(unsigned int zone) override {return {};}
    unsigned int GetZoneLEDsCount(unsigned int zone) override {return counts.at(zone);}
    unsigned int GetZoneLEDsMax(unsigned int zone) override {return {};}
    unsigned int GetZoneLEDsMin(unsigned int zone) override {return {};}
    matrix_map_type GetZoneMatrixMap(unsigned int zone) override {return {};}
    const unsigned int* GetZoneMatrixMapData(unsigned int zone) override {return {};}
    unsigned int GetZoneMatrixMapHeight(unsigned int zone) override {return {};}
    unsigned int GetZoneMatrixMapWidth(unsigned int zone) override {return {};}
    unsigned int GetZoneModeCount(unsigned int zone) override {return {};}
    unsigned int GetZoneModeBrightness(unsigned int zone, unsigned int mode) override {return {};}
    unsigned int GetZoneModeBrightnessMax(unsigned int zone, unsigned int mode) override {return {};}
    unsigned int GetZoneModeBrightnessMin(unsigned int zone, unsigned int mode) override {return {};}
    RGBColor GetZoneModeColor(unsigned int zone, unsigned int mode, unsigned int color_index) override {return {};}
    unsigned int GetZoneModeColorMode(unsigned int zone, unsigned int mode) override {return {};}
    unsigned int GetZoneModeColorsCount(unsigned int zone, unsigned int mode) override {return {};}
    unsigned int GetZoneModeColorsMax(unsigned int zone, unsigned int mode) override {return {};}
    unsigned int GetZoneModeColorsMin(unsigned int zone, unsigned int mode) override {return {};}
    unsigned int GetZoneModeDirection(unsigned int zone, unsigned int mode) override {return {};}
    unsigned int GetZoneModeFlags(unsigned int zone, unsigned int mode) override {return {};}
    std::string GetZoneModeName(unsigned int zone, unsigned int mode) override {return {};}
    unsigned int GetZoneModeSpeed(unsigned int zone, unsigned int mode) override {return {};}
    unsigned int GetZoneModeSpeedMax(unsigned int zone, unsigned int mode) override {return {};}
    unsigned int GetZoneModeSpeedMin(unsigned int zone, unsigned int mode) override {return {};}
    std::string GetZoneName(unsigned int zone) override {return zone<zoneNames.size()?zoneNames[zone]:"Zone "+std::to_string(zone);}
    unsigned int GetZoneSegmentCount(unsigned int zone) override {return zone<segments.size()?segments[zone].size():0;}
    segment_flags GetZoneSegmentFlags(unsigned int zone, unsigned int segment) override {return {};}
    unsigned int GetZoneSegmentLEDsCount(unsigned int zone, unsigned int segment) override {return segments.at(zone).at(segment).second;}
    matrix_map_type GetZoneSegmentMatrixMap(unsigned int zone, unsigned int segment) override {return {};}
    const unsigned int * GetZoneSegmentMatrixMapData(unsigned int zone, unsigned int segment) override {return {};}
    unsigned int GetZoneSegmentMatrixMapHeight(unsigned int zone, unsigned int segment) override {return {};}
    unsigned int GetZoneSegmentMatrixMapWidth(unsigned int zone, unsigned int segment) override {return {};}
    std::string GetZoneSegmentName(unsigned int zone, unsigned int segment) override {return segments.at(zone).at(segment).first;}
    unsigned int GetZoneSegmentStartIndex(unsigned int zone, unsigned int segment) override {unsigned start=GetZoneStartIndex(zone);for(unsigned i=0;i<segment;i++)start+=segments.at(zone).at(i).second;return start;}
    unsigned int GetZoneSegmentType(unsigned int zone, unsigned int segment) override {return {};}
    unsigned int GetZoneStartIndex(unsigned int zone) override {unsigned start=0;for(unsigned i=0;i<zone;i++)start+=counts[i];return start;}
    zone_type GetZoneType(unsigned int zone) override {return ZONE_TYPE_LINEAR;}
    unsigned int GetLEDsInZone(unsigned int zone) override {return {};}
    void SetZoneActiveMode(unsigned int zone, int mode) override {return;}
    void SetZoneColor(unsigned int zone, unsigned int color_index, RGBColor color) override {colors.at(GetZoneStartIndex(zone)+color_index)=color;}
    void SetZoneModeBrightness(unsigned int zone, unsigned int mode, unsigned int brightness) override {return;}
    void SetZoneModeColor(unsigned int zone, unsigned int mode, unsigned int color_index, RGBColor color) override {return;}
    void SetZoneModeColorMode(unsigned int zone, unsigned int mode, unsigned int color_mode) override {return;}
    void SetZoneModeColorsCount(unsigned int zone, unsigned int mode, unsigned int count) override {return;}
    void SetZoneModeDirection(unsigned int zone, unsigned int mode, unsigned int direction) override {return;}
    void SetZoneModeSpeed(unsigned int zone, unsigned int mode, unsigned int speed) override {return;}
    bool SupportsPerZoneModes() override {return {};}
    unsigned int GetModeCount() override {return modeNames.size();}
    unsigned int GetModeBrightness(unsigned int mode) override {return light;}
    unsigned int GetModeBrightnessMax(unsigned int mode) override {return 100;}
    unsigned int GetModeBrightnessMin(unsigned int mode) override {return {};}
    RGBColor GetModeColor(unsigned int mode, unsigned int color_index) override {return {};}
    unsigned int GetModeColorMode(unsigned int mode) override {return {};}
    unsigned int GetModeColorsCount(unsigned int mode) override {return {};}
    unsigned int GetModeColorsMax(unsigned int mode) override {return {};}
    unsigned int GetModeColorsMin(unsigned int mode) override {return {};}
    unsigned int GetModeDirection(unsigned int mode) override {return {};}
    unsigned int GetModeFlags(unsigned int mode) override {return MODE_FLAG_HAS_BRIGHTNESS|MODE_FLAG_HAS_PER_LED_COLOR;}
    std::string GetModeName(unsigned int mode) override {return modeNames.at(mode);}
    unsigned int GetModeSpeed(unsigned int mode) override {return {};}
    unsigned int GetModeSpeedMax(unsigned int mode) override {return {};}
    unsigned int GetModeSpeedMin(unsigned int mode) override {return {};}
    void SetModeBrightness(unsigned int mode, unsigned int brightness) override {return;}
    void SetModeColor(unsigned int mode, unsigned int color_index, RGBColor color) override {return;}
    void SetModeColorMode(unsigned int mode, unsigned int color_mode) override {return;}
    void SetModeColorsCount(unsigned int mode, unsigned int count) override {return;}
    void SetModeDirection(unsigned int mode, unsigned int direction) override {return;}
    void SetModeSpeed(unsigned int mode, unsigned int speed) override {return;}
    int GetActiveMode() override {return active;}
    void SetActiveMode(int mode) override {active=mode;modeChanges++;}
    void SetCustomMode() override {modeChanges++;}
    unsigned int GetLEDCount() override {return colors.size();}
    std::string GetLEDName(unsigned int led) override {return {};}
    std::string GetLEDDisplayName(unsigned int led) override {return {};}
    RGBColor GetColor(unsigned int led) override {return colors.at(led);}
    RGBColor* GetColorsPointer() override {return {};}
    void SetColor(unsigned int led, RGBColor color) override {return;}
    void SetAllColors(RGBColor color) override {return;}
    void SetAllZoneColors(int zone, RGBColor color) override {for(unsigned i=0;i<GetZoneLEDsCount(zone);i++)colors[GetZoneStartIndex(zone)+i]=color;}
    nlohmann::json GetDeviceSpecificConfigurationSchema() override {return {};}
    nlohmann::json GetDeviceSpecificConfiguration() override {return {};}
    void SetDeviceSpecificConfiguration(nlohmann::json configuration_json) override {return;}
    nlohmann::json GetDeviceSpecificZoneConfigurationSchema(int zone) override {return {};}
    nlohmann::json GetDeviceSpecificZoneConfiguration(int zone) override {return {};}
    void SetDeviceSpecificZoneConfiguration(int zone, nlohmann::json configuration_json) override {return;}
    void RegisterUpdateCallback(RGBControllerCallback new_callback, void * new_callback_arg) override {return;}
    void UnregisterUpdateCallback(void * callback_arg) override {return;}
    void ClearCallbacks() override {return;}
    void SignalUpdate(unsigned int update_reason) override {return;}
    void UpdateLEDs() override {updates++;}
    void UpdateZoneLEDs(int zone) override {return;}
    void UpdateSingleLED(int led) override {return;}
    void UpdateMode() override {return;}
    void UpdateZoneMode(int zone) override {return;}
    void SaveMode() override {return;}
    void ClearSegments(int zone) override {return;}
    void AddSegment(int zone, segment new_segment) override {return;}
    void ConfigureZone(int zone_idx, zone new_zone) override {return;}
    void ResizeZone(int zone, int new_size) override {return;}
    void ConfigureDevice(controller_flags new_flags, std::string new_name) override {return;}
};
