#!/bin/sh

cd `dirname $0`

mkdir -p build
cd build

# cmake -DBORINGSSL_DIR=$(pwd)/libs-build/boringssl-linux -DBORINGSSL_INCLUDE=$(pwd)/libs/boringssl/include -DCMAKE_CXX_COMPILER=/usr/bin/g++-10 ../ 
cmake -DCMAKE_CXX_COMPILER=/usr/bin/g++-10 ../ 

make
