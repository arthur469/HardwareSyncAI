# Hardware Sync

[![pipeline status](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/badges/master/pipeline.svg)](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/commits/master)

## What is this?

This is a plugin for [OpenRGB](https://gitlab.com/CalcProgrammer1/OpenRGB) that allows you sync your ARGB devices with hardware measures (CPU temperature, RAM usage, fans speed, etc...)

## Downloads

* [Windows 64](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/artifacts/master/download?job=Windows%2064)
* [Linux 64 Stretch](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/artifacts/master/download?job=Linux%2064%20-%20Stretch)
* [Linux 64 Buster](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/artifacts/master/download?job=Linux%2064%20-%20Buster)
* [Linux 64 Bullseye](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/artifacts/master/download?job=Linux%2064%20-%20Bullseye)

## How do I install it?

* Download and extract the correct files depending on your system
* Launch OpenRGB
* From the Settings -> Plugins menu, click the "Install plugin" button

### Windows 

Download and place [lhwm-wrapper.dll](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/raw/master/dependencies/lhwm-cpp-wrapper/x64/Release/lhwm-wrapper.dll) in the directory of `OpenRGB.exe`.

### Linux

You need `lm-sensors` installed.
