#!/bin/bash
# Usage: ./start_sensors.sh [wait_time] [silent]

cd "$(dirname "$0")"

./bin/mem_sensor "$@" &
./bin/cpu_sensor "$@" &
./bin/proc_sensor "$@" &
