@echo off

set OUTPUT_DIR=deploy
set EXE_PATH=release\rufus.exe
set LIB_DIR=%QTDIR%\bin

rem Create target directory
if not exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%

rem Deploy DLLs
%LIB_DIR%\windeployqt.exe --force --release --dir %OUTPUT_DIR% --no-quick-import --no-translations --no-svg %EXE_PATH%

rem Removing unused DLLs
if exist %OUTPUT_DIR%\opengl32sw.dll del /q %OUTPUT_DIR%\opengl32sw.dll

rem Deploy Exe
xcopy /y %EXE_PATH% %OUTPUT_DIR%
