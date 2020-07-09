Lightwave is a cross platform C++ library designed for building games on linux, mac, windows, android, and iOS.  windows phones is planned for a future update.

Lightwave is mostly complete for creating full 2D and 3D games, the framework is designed for barebones cross platform building, while the engine is designed to plug in components, such as UI, asset managment, and job based multi-threading systems.
The framework is designed in modules, LWCore is required by all modules, LWPlatform and LWNetwork only rely on LWCore.  LWAudio and LWVideo rely on LWCore and LWPlatform.  you can pick and choose which module fits your required needs.

All features of lightwave are still in development, but the majority of the framework and engine is usable as-is to design games, the framework is nearly complete, but the engine will likely see substancial expansions as time goes on.

Use doxygen to generate documentation over the framework, the engine still requires a fair bit of documentation to be at an adequate place.


# Required dependencys
##### if platform requires manual building of librarys, the Build directory of dependency was built for the following versions

FreeType-2.9.1 https://www.freetype.org/ (be sure to rename the folder to libfreetype)

GLEW-1.13.0 http://glew.sourceforge.net/ (be sure to rename the folder to libGLEW)

lpng1.6.34 http://www.libpng.org/pub/png/libpng.html (be sure to rename the folder to libpng amd grab the .tar version if building for android(.zip does not have neon-arm sources))

zlib-1.2.8 https://zlib.net/ (be sure to rename the folder to libz)

libogg-1.3.2 https://xiph.org/ogg/ (be sure to rename the folder to libogg)

libvorbis-1.3.5 https://xiph.org/vorbis/ (be sure to rename the folder to libvorbis)

Place all library folders into the dependency directory then naviagate to the build system for that platform.  the build systems are designed to build statically linked librarys.
The build systems were built with the above versions.  Using more upto date librarys may required modifications to the build processes, but ideally if the library hasn't had radical changes it should be interchangable.

### Required for LWPlatform
#### Required on linux
libx11-dev

libxrandr-dev

### Required for LWVideo 

#### Required on linux
mesa-common-dev

libfreetype6-dev

libglew-dev

libpng-dev


#### Required on MacOSX
Installation done with brew (https://brew.sh)

Freetype

libpng


### Required for LWAudio

#### Required on linux
libvorbis-dev

libogg-dev

libpulse-dev

#### Required on mac OS X
libvorbis

libogg

### Optional dependency
Botan-2.9.0 https://github.com/randombit/botan (rename to libbotan)

This library is only fully implemented for building on msvs, other platforms well be made available in time.
Building requires python to be installed.
Rename the downloaded package to libbotan and place inside dependency folder alongside other dependencys.

The build is implemented through optional build steps in the solution.  if you use shared librarys, or non standard msvc runtime librarys then they configure.py build may need to be adjusted.

When using libbotan for x86 or x64 the include headers are found under libbotan/x86/include or libbotan/x64/include.


libvpx https://github.com/webmproject/libvpx (rename to libvpx)
This library is used for LWEVideoPlayer, omitting LWEVideoPlayer will not require this library.
The build steps can be followed if on non-windows platforms.  if on windows10 the Build project requires Windows Subsystem for linux with any distro installed as well with make installed(apt-get update, apt-get install make).
It also requires the correct yasm(http://yasm.tortall.net/) (Not the VS2010 version) to be installed to the visual studio common7/Tools directoy.  (copy yasm-1.x.0-win32.exe into directory and rename to yasm.exe).  after this is complete you should be able to load the msvc17 solution and build for each platform.

Older windows systems will have to follow the build process found on the project page.

## Building the framework
### G++
```
cd Framework/Build/G++/<module> 
make <"debug=1" for debug librarys>
```
### XCode
```
Open Framework/Build/XCode/LightWave.Framework.xcodeproj in xcode
Build each individual module.
```
### MS Visual Studio 17
```
Open Framework/Build/MSVC17/Lightwave.sln
Build each individual module.
```

### NDK
```
(note: these instructions were created for windows powershell, disect build.ps1 to see what arguments to pass to ndk-build)
open command prompt
cd Framework/Build/NDK/
build.bat #(instructions on parameters well be outputted here)
```

## Building the engine
### G++
```
cd Engine/Build/G++/
make <"debug=1" for debug librarys>
```
### XCode
```
Open Framework/Build/XCode/LWEngine.xcodeproj
Build LWEngine
```
### MS Visual Studio 17
```
Open Framework/Build/MSVC17/LWEngine.sln
Build LWEngine
```
### NDK
```
(note: these instructions were created for windows powershell, disect build.ps1 to see what arguments to pass to ndk-build)
open command prompt
cd Engine/Build/NDK/
powershell
./Build1.ps1 LWEngine <Debug>
```
