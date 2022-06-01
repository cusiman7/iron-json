#!/bin/bash

rm -rf debug
rm -rf release

mkdir -p debug 
pushd debug 
cmake -DCMAKE_BUILD_TYPE=DEBUG ..
cmake --build .
cmake --build . -t test
./test
popd

mkdir -p release 
pushd release
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
cmake --build .
cmake --build . -t test
./test
popd
