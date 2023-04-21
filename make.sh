#!/usr/bin/env sh

set -e

if ! [ -f Makefile ]
then
    cd src
fi

make clean
make CC='cc -g -Wall'
