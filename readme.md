# Hardware Sync

[![pipeline status](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/badges/master/pipeline.svg)](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/commits/master)

## What is this?

This is a plugin for [OpenRGB](https://gitlab.com/CalcProgrammer1/OpenRGB) that allows you sync your ARGB devices with hardware measures (CPU temperature, RAM usage, fans speed, etc...)

## Experimental (Master)

* [Windows 64](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/artifacts/master/download?job=Windows%2064)
* [Linux 64 Buster](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/artifacts/master/download?job=Linux%2064%20-%20Buster)
* [Linux 64 Bullseye](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/artifacts/master/download?job=Linux%2064%20-%20Bullseye)
* [Linux 64 Bookworm](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/artifacts/master/download?job=Linux%2064%20-%20Bookworm)

## Stable (0.8)

* [Linux 64 Buster](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/3418212011/artifacts/download)
* [Linux 64 Bullseye](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/3418212012/artifacts/download)
* [Windows 64](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/3418212013/artifacts/download)

## How do I install it?

* Download and extract the correct files depending on your system
* Launch OpenRGB
* From the Settings -> Plugins menu, click the "Install plugin" button

### Windows 

Download and place [lhwm-wrapper.dll](https://gitlab.com/OpenRGBDevelopers/lhwm-wrapper/-/jobs/artifacts/master/download?job=main_build) in the directory of `OpenRGB.exe`.

Notice: some of the hardware measures may require that you run OpenRGB as admin.

### Linux

Dependencies

- `lm-sensors`
- `libgtop2`
- `libglib2`

## How do I use it?

Here is a link to find the documentation you need.

[HardwareSync Help](https://gitlab.com/OpenRGBDevelopers/OpenRGB-Wiki/-/blob/stable/Plugins/HardwareSync/HardwareSync.md)
