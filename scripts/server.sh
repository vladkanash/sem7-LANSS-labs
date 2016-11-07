#!/usr/bin/env bash

if [ $# -eq 0 ]
  then
    echo "Usage:"
    echo -e "\t server.sh <port_number>"
    exit 1
fi

#if cannot find build directory, run build process
if [ ! -d "../build/" ]; then
  ./build.sh
fi

../build/bin/server $1
