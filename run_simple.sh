#!/bin/bash

# Start LoadCell1 in background
echo "Calibrating LoadCell1..."
{ echo "calibrate"; } | ./main 1076702 772 2>&1 | grep --line-buffered "^rec:" | while read line; do
    echo "LoadCell1: $line"
done &

# Start LoadCell2 in background
echo "Calibrating LoadCell2..."
{ echo "calibrate"; } | ./main 652964 3616 2>&1 | grep --line-buffered "^rec:" | while read line; do
    echo "LoadCell2: $line"
done &

echo "Load cells running. Press Ctrl+C to stop."

# Wait for processes
wait
