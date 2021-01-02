#!/bin/sh

cd `dirname $0`

mkdir -p build
cd build

cmake ../ -DCMAKE_CXX_COMPILER=/usr/bin/g++-10

make
