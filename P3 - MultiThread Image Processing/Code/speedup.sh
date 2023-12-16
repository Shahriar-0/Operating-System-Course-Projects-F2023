#!/bin/env bash

if [ $# -eq 1 ] && [ "$1" = "clean" ]; then
    cd ./serial
    make clean
    cd ..
    cd ./parallel
    make clean
else
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    BOLD='\033[1m'
    BOLDBLUE='\033[1;34m'
    NC='\033[0m'

    input="../../../Assets/Pictures/input4.bmp"
    serialTxt="../Assets/Results/serial.txt"
    parallelTxt="../Assets/Results/parallel.txt"

    cd ./serial
    make > /dev/null
    make run ARGS="$input" > ../$serialTxt

    cd ..

    cd parallel
    make > /dev/null
    make run ARGS="$input" > ../$parallelTxt

    cd ..
    serial=$(tail -1 $serialTxt | grep -o '[0-9.]*')
    parallel=$(tail -1 $parallelTxt | grep -o '[0-9.]*')
    speedup=$(echo "scale=2; $serial / $parallel" | bc)
    printf "Serial: \t\t\t  Parallel:\n"
    paste -d '|' <(sed "s/^/$(printf "${RED}")/" $serialTxt) <(sed "s/^/$(printf "${GREEN}")/" $parallelTxt | sed "s/\x1b\[0m/\x1b\[0m${GREEN}/") | column -t -s '|'
    echo -e "${BOLDBLUE}Speedup: $speedup${NC}"   
fi

