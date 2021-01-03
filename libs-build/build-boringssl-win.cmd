mkdir boringssl-win > NUL 2>&1
if ERRORLEVEL 1 cmd /c exit 0

cd boringssl-win
cmake ..\..\libs\boringssl
