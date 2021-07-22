# Hardware Sync

[![pipeline status](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/badges/master/pipeline.svg)](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/commits/master)

## What is this?

This is a plugin for [OpenRGB](https://gitlab.com/CalcProgrammer1/OpenRGB) that allows you sync your ARGB devices with hardware measures (CPU temperature, RAM usage, fans speed, etc...)

## How do I install it?

### **windows**

1. download the latest [Windows pipeline](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/artifacts/master/download?job=Windows%2064)

2. copy ``OpenRGBHardwareSyncPlugin.dll`` from the 7z file to  ``C:\Users\Username\appdata\roaming\OpenRGB\plugins``

3. download [lhwm-wrapper.dll](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/raw/master/dependencies/lhwm-cpp-wrapper/x64/Release/lhwm-wrapper.dll?inline=false)

4. Place `lhwm-wrapper.dll` in the directory of `OpenRGB.exe`.

### **linux**

You need `lm-sensors` installed.

1. download the latest [linux pipeline](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/artifacts/master/download?job=Linux%2064)

2. Copy ``libOpenRGBHardwareSyncPlugin.so`` from the 7z to ``~/.config/OpenRGB/plugins``

