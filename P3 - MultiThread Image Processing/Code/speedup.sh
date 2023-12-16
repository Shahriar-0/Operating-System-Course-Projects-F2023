#!/bin/env bash

RED='\033[0;31m'
GREEN='\033[0;32m'
BOLD='\033[1m'
BOLDBLUE='\033[1;34m'
NC='\033[0m'

inputMain="../Assets/Pictures/input1.bmp"
input="../../$inputMain"
serialTxt="../Assets/Results/serial.txt"
serialBmp="../Assets/Pictures/output_serial.bmp"
parallelTxt="../Assets/Results/parallel.txt"
parallelBmp="../Assets/Pictures/output_parallel.bmp"

function showResult {
    serial=$(tail -1 $serialTxt | grep -o '[0-9.]*')
    parallel=$(tail -1 $parallelTxt | grep -o '[0-9.]*')
    speedup=$(echo "scale=2; $serial / $parallel" | bc)
    printf "Serial: \t\t\t  Parallel:\n"
    paste -d '|' <(sed "s/^/$(printf "${RED}")/" $serialTxt) <(sed "s/^/$(printf "${GREEN}")/" $parallelTxt | sed "s/\x1b\[0m/\x1b\[0m${GREEN}/") | column -t -s '|'
    echo -e "${BOLDBLUE}Speedup: $speedup${NC}"
}

function showPictures {
    feh -x $inputMain &
    feh -x $serialBmp &
    feh -x $parallelBmp &
}

function show {
    showResult
    showPictures
}

if [ $# -eq 1 ] && [ "$1" = "clean" ]; then
    cd ./serial
    make clean > /dev/null
    cd ..
    cd ./parallel
    make clean > /dev/null
    cd ..
    rm -f $serialBmp
    rm -f $parallelBmp
    rm -f $serialTxt
    rm -f $parallelTxt
    echo -e "${GREEN}Cleaned${NC}"
elif [ $# -eq 1 ] && [ "$1" = "showresult" ]; then
    showResult
elif [ $# -eq 1 ] && [ "$1" = "showpictures" ]; then
    showPictures
elif [ $# -eq 1 ] && [ "$1" = "show" ]; then
    show
elif [ $# -eq 1 ] && [ "$1" = "run" ]; then
    cd ./serial
    make > /dev/null
    make run ARGS="$input" > ../$serialTxt
    cd ..

    cd parallel
    make > /dev/null
    make run ARGS="$input" > ../$parallelTxt
    cd ..  
else
    echo "Usage: ./speedup.sh [clean|showresult|showpictures|show|run]"
fi

