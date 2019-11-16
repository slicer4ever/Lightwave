$Mode="Release";
$Debug = $False;
$Clean = $False;
$Install = $False;
$Uninstall = $False;
$Execute = $False;
for($i=0;$i -lt $args.length; $i++){
   if($args[$i].CompareTo("Debug") -eq 0){
      $Debug=$True;
      $Mode="Debug";
   }Elseif($args[$i].CompareTo("Clean") -eq 0){
      $Clean=$True;
   }Elseif($args[$i].CompareTo("Install") -eq 0){
      $Install = $True;
   }Elseif($args[$i].CompareTo("Uninstall") -eq 0){
      $UnInstall = $True;
   }Elseif($args[$i].CompareTo("Execute") -eq 0){
      $Execute = $True;
   }
}
if((Test-Path env:\ANDROID_NDK_ROOT) -eq $FALSE){
  Write-Host "Please set the ANDROID_NDK_ROOT environment variable to the ndk directory."
  Exit
}
if((Test-Path env:\ANDROID_HOME) -eq $FALSE){
   Write-Host "Please set the ANDROID_HOME environment variable to the android sdk directory."
   Exit
}
if((Test-Path env:\ANT_HOME) -eq $FALSE){
  Write-Host "Please set the ANT_HOME environment variable to the ant directory."
  Exit
}
if((Test-Path env:\JAVA_HOME) -eq $FALSE){
   Write-Host "Please set the JAVA_HOME environment variable to the java development kit home directory."
   Exit
}
Write-Host "Building in mode: $($Mode)"
$Progs = @()
$Args = @()
if($Clean){
   $Progs = $Progs+"$($Env:ANDROID_NDK_ROOT)\ndk-build.cmd";
   $Progs = $Progs+"$($Env:ANT_HOME)\bin\ant.bat";
   $Args = $Args+"clean";
   $Args = $Args+"clean";
   if($Debug -eq $true){
      $Args[0] = "NDK_DEBUG=1 clean";
      $Args[1] = "debug clean";
   }
}elseif($Install){
   $Files = Get-ChildItem bin\*.apk
   $TargetFile = "";
   for($i = 0; $i -lt $Files.length; $i++){
      if($Debug -eq $True){
         if($Files[$i].name -like "*debug*" -And $Files[$i].name -NotLike "*unaligned*"){
            $TargetFile = $Files[$i].name;
            break;
         }
      }else{
         if($Files[$i].name -NotLike "*debug*" -And $Files[$i].name -NotLike "*unaligned*"){
            $TargetFile = $Files[$i].name;
            break;
         }
      }
   }
   Write-Host "Target: $($TargetFile)"
   if($TargetFile -like "*.apk"){
      Write-Host "Discovered and installing: $($TargetFile)"
      $Progs = $Progs+"$($Env:ANDROID_HOME)\platform-tools\adb.exe";
      $Args = $Args+"install -r bin\$($TargetFile)";
   }else{
      Write-Host "Error: please build for the mode you intend to install for."
      Exit
   }
}elseif($Execute -Or $Uninstall){
   $TargetFile = Get-ChildItem AndroidManifest.xml
   if($TargetFile.length -eq 0){
      Write-Host "No AndroidManifest.xml detected!"
      Exit
   }
   Write-Host "Parsing: $($TargetFile.name)"
   [xml]$TXML = Get-Content $TargetFile
   Write-Host "Discovered Package: $($TXML.manifest.package)";
   Write-Host "Discovered Activity: $($TXML.manifest.application.activity.name)";
   $Progs = $Progs+"$($Env:ANDROID_HOME)\platform-tools\adb.exe";
   if($Execute){
      $Args = $Args+"shell am start -n $($TXML.manifest.package)/$($TXML.manifest.application.activity.name)";
   }else{
      $Args = $Args+"uninstall $($TXML.manifest.package)"
   }
}else{
   $Progs = $Progs+"$($Env:ANDROID_NDK_ROOT)\ndk-build.cmd";
   $Progs = $Progs+"$($Env:ANT_HOME)\bin\ant.bat";
   $Args = $Args+"";
   $Args = $Args+"release";
   if($Debug -eq $true){
      $Args[0] = "NDK_DEBUG=1";
      $Args[1] = "debug";
   }
}

$pinfo = New-Object System.Diagnostics.ProcessStartInfo
$pinfo.RedirectStandardError = $true
$pinfo.RedirectStandardOutput = $true
$pinfo.UseShellExecute = $false
$p = New-Object System.Diagnostics.Process
for($i = 0; $i -lt $Progs.length; $i++){
   $pinfo.FileName = $Progs[$i];
   $pinfo.Arguments = $Args[$i];
   $p.StartInfo = $pinfo;
   $p.Start() | Out-Null
   while($p.HasExited -eq $false){
      Write-Host $p.StandardOutput.ReadLine();
   }
   Write-Host $p.StandardOutput.ReadToEnd()
   Write-Host $p.StandardError.ReadToEnd()
   if($p.ExitCode -ne 0){
      break;
   }
}
