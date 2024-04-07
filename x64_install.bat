rem Requires administrator rights!
@echo off
set PWD="%~dp0"
pushd
cd /D %PWD%
copy /Y .\Binaries\x64\u3canpcb.dll C:\Windows\System32
copy /Y .\Binaries\x64\uvPeakCAN.dll C:\Windows\System32
popd
dir C:\Windows\System32\u*can*.dll
pause
