#!/bin/bash
git fetch
git pull

mkdir -p ../../bin/
gcc ../../benchmark.c -lm -m64 -O3 -o ../../bin/benchmark.elf

retVal=$?
if [ $retVal -ne 0 ]; then
    echo Compilation Failed
    exit 1
fi

../../bin/benchmark.elf

retVal=$?
if [ $retVal -ne 0 ]; then
    echo Tests failed
    exit 1
fi

echo Completed Successfully
exit 0
