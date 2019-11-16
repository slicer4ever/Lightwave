#This Build.ps1 is designed for Library builds.
$Mode="Release";
$Module=$args[0];
$Debug = $False;
$Performance = $False;
$Clean = $False;
$PlatformLevel = 0;
for($i=1;$i -lt $args.length; $i++){
   if($args[$i].CompareTo("Debug") -eq 0){
      $Debug=$True;
      $Mode="Debug";
   }Elseif($args[$i].CompareTo("Clean") -eq 0){
      $Clean=$True;
   }Elseif($args[$i].CompareTo("PlatformLevel") -eq 0){
		$PlatformLevel = $args[$i+1];
		$i++;
   }Elseif($args[$i].CompareTo("Performance") -eq 0){
	    $Performance = $True;
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
Write-Host "Building for: $($Module) in mode: $($Mode) Platform Level: $($PlatformLevel)"
if ($Debug -eq $True -and $Performance -eq $True){
	Write-Host "Building debug with optimizations enabled."
}
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
  $files = Get-ChildItem "obj/local/"

  for($i = 0; $i -lt $files.Count; $i++){
  	echo $files[$i].Name
  	$TargetFile = '../../../Binarys/'+$Mode+'/'+$files[$i].Name+'/lib'+$Module+'.a';
  	Remove-Item $TargetFile -Force
  }
}else{
	if($PlatformLevel -ne 0){
		Write-Host "Updating application.mk for new platform level."
		$ReplaceStr = "APP_PLATFORM := android-$($PlatformLevel)"
		$FileContents = (Get-Content ./jni/Application.mk)
		$FileContents = $FileContents -replace 'APP_PLATFORM := android-.+',$ReplaceStr
		Set-Content -Path ./jni/Application.mk -Value $FileContents
	}
	$Progs = $Progs+"$($Env:ANDROID_NDK_ROOT)\ndk-build.cmd";
	$Args = $Args+"";
	if($Debug -eq $true){
		if($Performance -eq $true){
			$Args[0] = "NDK_DEBUG=0";
		}else{
			$Args[0] = "NDK_DEBUG=1";
		}
	}
}

$DataHandler = {
	Write-Host $Event.SourceEventArgs.Data;
}

$pinfo = New-Object System.Diagnostics.ProcessStartInfo
$pinfo.RedirectStandardError = $true
$pinfo.RedirectStandardOutput = $true
$pinfo.UseShellExecute = $false
for($i = 0; $i -lt $Progs.length; $i++){
   $p = New-Object System.Diagnostics.Process
   $pinfo.FileName = $Progs[$i];
   $pinfo.Arguments = $Args[$i];
   $p.StartInfo = $pinfo;
   
   Register-ObjectEvent -inputObject $p -EventName "OutputDataReceived" -Action $DataHandler -SupportEvent;
   Register-ObjectEvent -inputObject $p -EventName "ErrorDataReceived" -Action $DataHandler -SupportEvent;

   $p.Start();
   $p.BeginOutputReadLine();
   $p.BeginErrorReadLine();
   while($p.HasExited -eq $false){}
   if($p.ExitCode -ne 0){
      break;
   }
}

if ($Clean -eq $False){
  $files = Get-ChildItem "obj/local/"
  for($i = 0; $i -lt $files.Count; $i++){
  	$TargetDir = '../../../Binarys/'+$Mode+'/'+$files[$i].Name+'/'
  	$SourceDir = 'obj/local/'+$files[$i].Name+'/'
  	RoboCopy $SourceDir $TargetDir "lib$($Module).a" /njh /njs /ndl /nc /ns
  }
}
