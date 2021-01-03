mkdir libevent-win > NUL 2>&1
if ERRORLEVEL 1 cmd /c exit 0

cd libevent-win
cmake ..\..\libs\libevent
