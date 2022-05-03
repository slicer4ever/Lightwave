$SourceVersion = "1.10.0";
$PlatformTarget = $args[0];
$Platform = $args[1];
$Configuration = $args[2];
$TargetPlatformVersion = $args[3];
$Toolset = $args[4];
$Bin = $args[5];
$Target = ""
$Lib = ""
if($Configuration -eq 'Debug') { $Lib = 'vpxmdd.lib' }
elseif ($Configuration -eq 'Release') { $Lib = 'vpxmd.lib' }
if($PlatformTarget -eq 'x86') { $Target = "$($Target)x86"; }
elseif ($PlatformTarget -eq 'x64') { $Target = "$($Target)x86_64"; }
if($Platform -eq 'Win32') { $Target="$($Target)-win32"; }
elseif ($Platform -eq 'x64') { $Target="$($Target)-win64" }

write-host "Hello: $($PlatformTarget) | $($Platform) | $($Configuration) | $($Bin) | $($Target)"
New-Item -ItemType Directory -Force -Path "$($Platform)/$($Configuration)/"
Set-Location "./$($Platform)/$($Configuration)/"
wsl ../../../../../libvpx/configure --target=${Target}-vs15 --as=yasm --disable-docs --disable-examples --disable-tools
wsl make dist
wsl sed -i "s/ <ProjectGuid/ <WindowsTargetPlatformVersion>$($TargetPlatformVersion)<\/WindowsTargetPlatformVersion><ProjectGuid/g" *.vcxproj
wsl sed -i "s/v141/$($Toolset)/g" *.vcxproj
Start-Process -NoNewWindow -Wait -FilePath "$($Bin)\msbuild.exe" -ArgumentList "vpx.vcxproj -m -t:Build -p:Configuration=$($Configuration) -p:Platform=$($Platform)"
wsl mkdir -p ../../../../../libvpx/$Configuration/$PlatformTarget/
wsl rm -r -f ../../../../../libvpx/$Configuration/$PlatformTarget/include
wsl mv vpx-vp8-vp9-nodocs-${Target}md-vs15-v${SourceVersion}/include/ ../../../../../libvpx/$Configuration/$PlatformTarget/
wsl mv vpx_config.h ../../../../../libvpx/$Configuration/$PlatformTarget/include/vpx_config.h
wsl mv $Platform/$Configuration/$Lib ../../../../../Binarys/$Configuration/$PlatformTarget/libvpx.lib