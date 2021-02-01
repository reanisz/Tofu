#!/bin/bash

SCRIPT_DIR=$(cd $(dirname $0); pwd)

cd $SCRIPT_DIR/box2d
git reset --hard HEAD
git apply ../box2d.patch

cd $SCRIPT_DIR/picotls
git reset --hard HEAD
git apply ../picotls.patch

cd $SCRIPT_DIR/picoquic
git reset --hard HEAD
git apply ../picoquic.patch

