# Harekaze2 fork
Forked version of [Harekaze/pvr.epgstation](https://github.com/Harekaze/pvr.epgstation)

The motivation for this fork is to support EPGStation v2 and fix minor issues.

## Supported environment

### Backend
- EPGStation [v2.6.20](https://github.com/l3tnun/EPGStation/tree/v2.6.20)

### Frontend
- Kodi **18.8** Leia
  + for Android (armv7)
  + for Windows 10 (64 bit)

It might work on other platforms, but it hasn't been tested.

## Building from source

### Android ARM
*Android NDK is required.*

```sh
$ ndk-build APP_ABI=armeabi-v7a
$ ./jni/pack.sh APP_ABI=armeabi-v7a
$ ls pvr.chinachu.zip
```

### Windows

Requirements:
- Visual Studio 2019
- PowerShell v5

```powershell
> ./build.ps1
> ls ./pvr.epgstation.zip
```

## Installation

### Install manually

1. Build or download the appropriate version for your platform.
2. Launch Kodi.
3. Navigate to System -> Add-ons -> Install from zip file
4. Select the zip file which you get in first step.

## License

[GPLv3](LICENSE)
