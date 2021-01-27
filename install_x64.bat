@echo off
set PWD="%~dp0"
pushd
cd /D %PWD%
copy /Y .\Libraries\CANAPI\x64\Release_dll\u3canpcb.dll C:\Windows\System32
copy /Y .\Libraries\UVPCAN\x64\Release_dll\uvpcan.dll C:\Windows\System32
popd
dir C:\Windows\System32\u*can*.dll
pause
