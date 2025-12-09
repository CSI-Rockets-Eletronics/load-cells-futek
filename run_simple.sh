#!/bin/bash

set -eo pipefail

# kill all child processes on exit
trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

# code to unbind USBs (TG)
./usb_unbind.sh

# start main programs - they will print to terminal
echo "Starting LoadCell1 (serial: 1076702)..."
./main 1076702 772 2>&1 | grep --line-buffered "^rec:" | while read line; do
    echo "LoadCell1: $line"
done &

Uncomment to enable LoadCell2:
echo "Starting LoadCell2 (serial: 652964)..."
./main 652964 3616 2>&1 | grep --line-buffered "^rec:" | while read line; do
    echo "LoadCell2: $line"
done &

echo "Load cells running. Press Ctrl+C to stop."

# wait for any of the background processes to finish
wait -n
