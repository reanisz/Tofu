#!/bin/bash

cd `dirname $0`

mkdir -p lsquic-linux
cd lsquic-linux
#cmake -DBORINGSSL_DIR=../boringssl-linux ../../libs/lsquic
cmake -DBORINGSSL_DIR=$(pwd)/../boringssl-linux -DBORINGSSL_INCLUDE=../../libs/boringssl/include ../../libs/lsquic
make

