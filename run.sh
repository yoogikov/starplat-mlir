#!/bin/bash

printf "Running Tests\n\n"

./build/bin/app "$1" 
./build/bin/app "$1" > ./build/runtest/out.ll 
 
printf '\n\n'
echo "Testing complete"
