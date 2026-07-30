@echo OFF

git fetch
git pull

set BINDIR=%cd%\..\..\bin\
set SRCDIR=%cd%\..\..\
set SOURCENAME="benchmark"

if not exist "%BINDIR%" ( 
    mkdir "%BINDIR%"
)

cd "%BINDIR%"

if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\" (
    CALL "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64
) else (
    CALL "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" -arch=x64
)

cl.exe "%SRCDIR%\%SOURCENAME%.c" /std:c11 /link /out:"%BINDIR%\%SOURCENAME%.exe"

"%BINDIR%\%SOURCENAME%.exe"

exit /B %errorlevel%
