#!/usr/bin/env bash

if [ $# -ne 2 ]
  then
    echo "Usage:"
    echo -e "\t server.sh <port_number>"
    exit 1
fi

#if cannot find build directory, run build process
if [ ! -d "../build/" ]; then
  ./build.sh
fi

../build/bin/client $1 $2