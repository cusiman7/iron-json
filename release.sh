#!/bin/bash

set -e

rm -rf debug
rm -rf release

mkdir -p debug 
pushd debug 
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j
./test
popd

mkdir -p release 
pushd release
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake --build . -j
./test
./bench
