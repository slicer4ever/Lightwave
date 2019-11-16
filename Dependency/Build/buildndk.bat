@echo off
setlocal EnableDelayedExpansion
REM This bat file builds each module, by calling the Build.ps1 file in each underlying directory, these Build.ps1's are designed for building a shared library, to build an application see the samples Build.ps1 files.
set Clean=
set Target=
set DoNext=
set Performance=
set Module=0
set Mode=Release
set ogg=0
set vorbis=0
set freetype=0
set png=0
set zlib=0

for %%x in (%*) do (
if "%%x"=="Debug" (set Mode=Debug)
if "%%x"=="Clean" (set Clean=Clean)
if "%%x"=="Performance" (set Performance=Performance)
if "%%x"=="All" (
set ogg=1 
set vorbis=1 
set freetype=1 
set png=1 
set zlib=1
)
if "%%x"=="libogg" (set ogg=1)
if "%%x"=="libvorbis" (set vorbis=1)
if "%%x"=="libfreetype" (set freetype=1)
if "%%x"=="libpng" (set png=1)
if "%%x"=="libz" (set zlib=1 )
if !DoNext! equ 1 (
	set DoNext=0
	set Target=PlatformLevel' '%%x
	)
if "%%x"=="PlatformLevel" (
	set DoNext=1
	)
)
if %ogg% equ 1 (
cd libogg/NDK/
powershell "./Build.ps1 'ogg' '%Mode%' '%Performance%' '%Clean%' '%Target%'"
cd ../../
)

if %vorbis% equ 1 (
cd libvorbis/NDK/
powershell "./Build.ps1 'vorbis' '%Mode%' '%Performance%' '%Clean%' '%Target%'"
cd ../../
)
if %png% equ 1 (
cd libpng/NDK/
powershell "./Build.ps1 'png' '%Mode%' '%Performance%' '%Clean%' '%Target%'"
cd ../../
)
if %freetype% equ 1 (
cd libfreetype/NDK/
powershell "./Build.ps1 'freetype' '%Mode%' '%Performance%' '%Clean%' '%Target%'"
cd ../../
)
if %zlib% equ 1 (
cd libz/NDK/
powershell "./Build.ps1 'z' '%Mode%' '%Performance%' '%Clean%' '%Target%'"
cd ../../
)
if %ogg% equ 0 (
	if %vorbis% equ 0 (
		if %png% equ 0 (
			if %freetype% equ 0 (
				if %zlib% equ 0 (
					goto :error
				)
			)
		)
	)
)
goto :finished
:error
echo "Incorrect argument list, arguments: <Module(libogg, libvorbis, libpng, libfreetype, libz, All(builds all modules))> <Mode(Optional Debug mode, otherwise Release assumed)> <Performance(for optimized debug builds)> <Clean> <PlatformLevel=x>"
:finished