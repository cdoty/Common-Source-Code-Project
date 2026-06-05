echo off

if exist "%ProgramFiles(x86)%" goto is_x64
set path="%ProgramFiles%\Microsoft Visual Studio\2017\WDExpress\MSBuild\15.0\Bin";%PATH%
goto start

:is_x64
set path="%ProgramFiles(x86)%\Microsoft Visual Studio\2017\WDExpress\MSBuild\15.0\Bin";%PATH%

:start
rmdir /s /q build_vc15
mkdir build_vc15

msbuild.exe bmjr.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="x64"
mkdir build_vc15\bmjr
copy bin\x64\Release\bmjr.exe build_vc15\bmjr\.

msbuild.exe tvboy.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="x64"
mkdir build_vc15\tvboy
copy bin\x64\Release\tvboy.exe build_vc15\tvboy\.

pause
echo on
exit /b
