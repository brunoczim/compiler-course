#!/usr/bin/env sh

set -e

cd src

make watch WATCH=../make.sh
