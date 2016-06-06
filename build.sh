#!/bin/bash
if [ -z "$1" ]; then
    echo "usage: build.sh <generator>"
    return 0
else
    echo "Use generator $1"
    CMAKE_GENERATOR=$1
fi

cd `dirname "$0"`

git submodule update

mkdir cmocka/build
cd cmocka/build

echo ">>>Compile cmocka static lib"

cmake -D WITH_STATIC_LIB=ON -G "$CMAKE_GENERATOR" ../

make cmocka_static

cd -

echo ">>>Compile lazy RE"

mkdir build

cd build

cmake -G "$CMAKE_GENERATOR" ../

make

echo ">>>Run tests"

./re_test
