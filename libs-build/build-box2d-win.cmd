mkdir box2d-win > NUL 2>&1
if ERRORLEVEL 1 cmd /c exit 0

cd box2d-win
cmake ..\..\libs\box2d
