@echo off
setlocal EnableDelayedExpansion
REM This bat file builds each module, by calling the Build.ps1 file in each underlying directory, these Build.ps1's are designed for building a shared library, to build an application see the samples Build.ps1 files.
set Clean=
set Target=
set DoNext=
set Performance=
set Module=0
set Mode=Release
set LWCore=0
set LWVideo=0
set LWPlatform=0
set LWAudio=0
set LWNetwork=0

for %%x in (%*) do (
if "%%x"=="Debug" (set Mode=Debug)
if "%%x"=="Clean" (set Clean=Clean)
if "%%x"=="Performance" (set Performance=Performance)
if "%%x"=="All" (
set LWCore=1 
set LWVideo=1 
set LWPlatform=1 
set LWAudio=1 
set LWNetwork=1
)
if "%%x"=="LWCore" (set LWCore=1)
if "%%x"=="LWPlatform" (set LWPlatform=1)
if "%%x"=="LWVideo" (set LWVideo=1)
if "%%x"=="LWAudio" (set LWAudio=1)
if "%%x"=="LWNetwork" (set LWNetwork=1 )
if !DoNext! equ 1 (
	set DoNext=0
	set Target=PlatformLevel' '%%x
	)
if "%%x"=="PlatformLevel" (
	set DoNext=1
	)
)
if %LWCore% equ 1 (
cd LWCore
powershell "./Build.ps1 'LWCore' '%Mode%' '%Performance%' '%Clean%' '%Target%'"
cd ../
)

if %LWPlatform% equ 1 (
cd LWPlatform
powershell "./Build.ps1 'LWPlatform' '%Mode%' '%Performance%' '%Clean%' '%Target%'"
cd ../
)
if %LWVideo% equ 1 (
cd LWVideo
powershell "./Build.ps1 'LWVideo' '%Mode%' '%Performance%' '%Clean%' '%Target%'"
cd ../
)
if %LWAudio% equ 1 (
cd LWAudio
powershell "./Build.ps1 'LWAudio' '%Mode%' '%Performance%' '%Clean%' '%Target%'"
cd ../
)
if %LWNetwork% equ 1 (
cd LWNetwork
powershell "./Build.ps1 'LWNetwork' '%Mode%' '%Performance%' '%Clean%' '%Target%'"
cd ../
)
if %LWCore% equ 0 (
	if %LWPlatform% equ 0 (
		if %LWVideo% equ 0 (
			if %LWAudio% equ 0 (
				if %LWNetwork% equ 0 (
					goto :error
				)
			)
		)
	)
)
goto :finished
:error
echo "Incorrect argument list, arguments: <Module(LWCore, LWPlatform, LWVideo, LWAudio, LWNetwork, all(builds all modules))> <Mode(Optional Debug mode, otherwise Release assumed)> <Performance(for optimized debug builds)> <Clean> <PlatformLevel=x>"
:finished