mkdir lsquic-win > NUL 2>&1
if ERRORLEVEL 1 cmd /c exit 0

cd lsquic-win
cmake -DBORINGSSL_DIR=%~dp0/boringssl-win -DBORINGSSL_INCLUDE=../../libs/boringssl/include -DZLIB_INCLUDE_DIR=%~dp0/libs/zlib-include/ ..\..\libs\lsquic
