#!/bin/bash

cd `dirname $0`

mkdir -p boringssl-linux
cd boringssl-linux
cmake ../../libs/boringssl/
make
