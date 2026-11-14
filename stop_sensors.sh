#!/bin/bash
# Usage: ./stop_sensors.sh

for sensor in mem_sensor cpu_sensor proc_sensor; do
    if pkill -f "bin/$sensor"; then
        echo "$sensor stopped"
    else
        echo "$sensor was not running"
    fi
done
