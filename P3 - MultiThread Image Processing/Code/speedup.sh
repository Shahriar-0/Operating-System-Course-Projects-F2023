#!/bin/env bash

RED='\033[0;31m'
GREEN='\033[0;32m'
BOLD='\033[1m'
BOLDBLUE='\033[1;34m'
NC='\033[0m'

inputMain="../Assets/Pictures/input3.bmp"
input="../../$inputMain"
serialTxt="../Assets/Results/serial.txt"
serialBmp="../Assets/Pictures/output_serial.bmp"
threads=4
parallelTxt="../Assets/Results/parallel.txt"
parallelThreadTxt=$(echo "../Assets/Results/parallel_$threads.txt")
parallelTxtRegex="../Assets/Results/parallel_[0-9]*.txt"
parallelBmp="../Assets/Pictures/output_parallel.bmp"
parallelBmpRegex="../Assets/Pictures/output_parallel_[0-9]*.bmp"
parallelThreadBmp=$(echo "../Assets/Pictures/output_parallel_$threads.bmp")

function showResult {
    serial=$(tail -1 $serialTxt | grep -o '[0-9.]*')
    parallel=$(tail -1 $parallelThreadTxt | grep -o '[0-9.]*')
    speedup=$(echo "scale=2; $serial / $parallel" | bc)
    printf "Serial: \t\t\tParallel with $threads threads:\n"
    paste -d '|' <(sed "s/^/$(printf "${RED}")/" $serialTxt) <(sed "s/^/$(printf "${GREEN}")/" $parallelThreadTxt | sed "s/\x1b\[0m/\x1b\[0m${GREEN}/") | column -t -s '|'
    echo -e "${BOLDBLUE}Speedup: $speedup${NC}"
}

function showPictures {
    feh -x $parallelThreadBmp &
    feh -x $serialBmp &
    feh -x $inputMain &
}

function show {
    showResult
    showPictures
}

function runSerial {
    cd ./serial
    make > /dev/null
    make run ARGS="$input" > ../$serialTxt
    cd ..
}

function runParallel {
    cd ./parallel
    make > /dev/null
    make run ARGS="$input" > ../$parallelTxt
    cd ..
    mv $parallelBmp $parallelThreadBmp
    mv $parallelTxt $parallelThreadTxt
}

if [ $# -eq 1 ] && [ "$1" = "clean" ]; then
    cd ./serial
    make clean > /dev/null
    cd ..
    cd ./parallel
    make clean > /dev/null
    cd ..
    rm -f $serialBmp
    rm -f $parallelBmpRegex
    rm -f $serialTxt
    rm -f $parallelTxtRegex
    echo -e "${GREEN}Cleaned${NC}"
elif [ $# -eq 1 ] && [ "$1" = "showresult" ]; then
    showResult
elif [ $# -eq 1 ] && [ "$1" = "showpictures" ]; then
    showPictures
elif [ $# -eq 1 ] && [ "$1" = "show" ]; then
    show
elif [ $# -eq 1 ] && [ "$1" = "runserial" ]; then
    runSerial
elif [ $# -eq 1 ] && [ "$1" = "runparallel" ]; then
    runParallel
elif [ $# -eq 1 ] && [ "$1" = "run" ]; then
    runSerial
    runParallel
elif [ $# -eq 2 ] && [ "$1" = "threads" ]; then
    python threads.py $2
elif [ $# -eq 2 ] && [ "$1" = "graph" ]; then
    python graph.py $2
else
    echo "Usage: ./speedup.sh [clean|showresult|showpictures|show|run| threads <input picture>]"
fi

