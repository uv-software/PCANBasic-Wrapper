@echo off

pushd
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat" x86
popd

pushd
call msbuild.exe .\pcb_testing.vcxproj /t:Clean;Build /p:"Configuration=Debug";"Platform=Win32"
if errorlevel 1 goto end

call .\Debug\pcb_testing.exe --gtest_filter=SmokeTest.*

:end
popd
pause
