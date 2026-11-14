#!/bin/bash

cd "$(dirname "$0")"

./bin/mem_sensor &
./bin/cpu_sensor &
./bin/proc_sensor &
