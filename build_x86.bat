@echo off

pushd
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat" x86
popd

pushd
call build_no.bat
rem type .\Sources\build_no.h
rem pause

call msbuild.exe .\Trial\pcb_test.vcxproj /t:Clean;Build /p:"Configuration=Debug";"Platform=Win32"
if errorlevel 1 goto end

call msbuild.exe .\Libraries\CANAPI\uvcanpcb.vcxproj /t:Clean;Build /p:"Configuration=Release_dll";"Platform=Win32"
if errorlevel 1 goto end

call msbuild.exe .\Libraries\CANAPI\uvcanpcb.vcxproj /t:Clean;Build /p:"Configuration=Debug_lib";"Platform=Win32"
if errorlevel 1 goto end

call msbuild.exe .\Libraries\UVPCAN\PeakCAN.vcxproj /t:Clean;Build /p:"Configuration=Release_dll";"Platform=Win32"
if errorlevel 1 goto end

call msbuild.exe .\Libraries\UVPCAN\PeakCAN.vcxproj /t:Clean;Build /p:"Configuration=Debug_lib";"Platform=Win32"
if errorlevel 1 goto end

echo Copying artifacts...
set BIN=".\Binaries"
if not exist %BIN% mkdir %BIN%
set BIN="%BIN%\x86"
if not exist %BIN% mkdir %BIN%
copy /Y .\Libraries\CANAPI\Release_dll\u3canpcb.dll %BIN%
copy /Y .\Libraries\CANAPI\Release_dll\u3canpcb.lib %BIN%
copy /Y .\Libraries\UVPCAN\Release_dll\uvPeakCAN.dll %BIN%
copy /Y .\Libraries\UVPCAN\Release_dll\uvPeakCAN.lib %BIN%
set BIN="%BIN%\lib"
if not exist %BIN% mkdir %BIN%
copy /Y .\Libraries\CANAPI\Debug_lib\u3canpcb.lib %BIN%
copy /Y .\Libraries\CANAPI\Debug_lib\uvcanpcb.pdb %BIN%\u3canpcb.pdb
copy /Y .\Libraries\UVPCAN\Debug_lib\uvPeakCAN.lib %BIN%
copy /Y .\Libraries\UVPCAN\Debug_lib\PeakCAN.pdb %BIN%\uvPeakCAN.pdb
copy /Y .\Sources\PCAN_Basic\x86\PCANBasic.lib %BIN%
echo Static libraries (x86) > %BIN%\readme.txt

echo Copying header files...
set INC=".\Includes"
if not exist %INC% mkdir %INC%
copy /Y .\Sources\PCAN*.h %INC%
copy /Y .\Sources\CANAPI\CANAPI.h %INC%
copy /Y .\Sources\CANAPI\CANAPI_Types.h %INC%
copy /Y .\Sources\CANAPI\CANAPI_Defines.h %INC%
copy /Y .\Sources\CANAPI\can_api.h %INC%

call msbuild.exe .\Utilities\can_moni\can_moni.vcxproj /t:Clean;Build /p:"Configuration=Release";"Platform=Win32"
if errorlevel 1 goto end

call msbuild.exe .\Utilities\can_test\can_test.vcxproj /t:Clean;Build /p:"Configuration=Release";"Platform=Win32"
if errorlevel 1 goto end

echo Copying utilities...
set BIN=".\Binaries"
if not exist %BIN% mkdir %BIN%
set BIN="%BIN%\x86"
if not exist %BIN% mkdir %BIN%
copy /Y .\Utilities\can_moni\Release\can_moni.exe %BIN%
copy /Y .\Utilities\can_test\Release\can_test.exe %BIN%

:end
popd
pause
