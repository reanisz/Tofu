mkdir build-win > NUL 2>&1
if ERRORLEVEL 1 cmd /c exit 0

cd build-win
cmake ..

pause

