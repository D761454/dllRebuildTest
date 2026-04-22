rem turn off echoing output
@echo off
rem inherit all current variables from the master environment/session
setlocal

rem https://learn.openwaterfoundation.org/owf-learn-windows-shell/batch-file-basics/batch-file-basics/
rem get solution path for rebuild
set SOLUTION_PATH=%~dp0testDLL.vcxproj

rem specify platform as editor that calls this script will only be on windows so rebuilds in editor will only need to be x64
set "PLATFORM=x64"
set CONFIGURATION=Release

set "CONSUMER_OUT=%CD%\x64\%CONFIGURATION%"

rem use vswhere to find VS installation
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
	rem iterate using i, parsing file line by line
	rem https://ss64.com/nt/for_cmd.html
	rem https://devblogs.microsoft.com/setup/vswhere-available/
	rem tokens=* - ensures return entire line
	for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -requires Microsoft.Component.MSBuild -property installationPath`) do set "VSINSTALL=%%i"
) else (
	echo vswhere not found; will try msbuild on PATH
)

rem use installed MSBuild, if none, use path ver
if defined VSINSTALL (
	if exist "%VSINSTALL%\MSBuild\Current\Bin\MSBuild.exe" (
		"%VSINSTALL%\MSBuild\Current\Bin\MSBuild.exe" "%SOLUTION_PATH%" /t:Rebuild /p:Configuration=%CONFIGURATION% /p:Platform=%PLATFORM% /p:OutDir="%CONSUMER_OUT%\\" /m
	) else (
		msbuild "%SOLUTION_PATH%" /t:Rebuild /p:Configuration=%CONFIGURATION% /p:Platform=%PLATFORM% /p:OutDir="%CONSUMER_OUT%\\" /m
	)
) else (
	msbuild "%SOLUTION_PATH%" /t:Rebuild /p:Configuration=%CONFIGURATION% /p:Platform=%PLATFORM% /p:OutDir="%CONSUMER_OUT%\\" /m
)

endlocal

rem https://www.tutorialspoint.com/batch_script/batch_script_syntax.htm
