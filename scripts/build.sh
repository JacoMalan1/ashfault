#!/usr/bin/env sh

if [ -d build ]; then
  rm -rf build
fi

EXTRA_OPTS=''
if [ "$1" == '-r' ]; then
  EXTRA_OPTS='-DCMAKE_BUILD_TYPE=Release'
fi

mkdir build
cd build
cmake .. -DCMAKE_C_COMPILER=$(which clang) -DCMAKE_CXX_COMPILER=$(which clang++) ${EXTRA_OPTS}
make
