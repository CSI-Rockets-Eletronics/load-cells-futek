#!/bin/bash

files=(/sys/bus/usb/drivers/ftdi_sio/*)
for file in ${files[@]}; do
    regex=':' # match stuff like 3-1.3:1.0
    if [[ $file =~ $regex ]]
    then
    echo -n ${file##*/} | sudo tee /sys/bus/usb/drivers/ftdi_sio/unbind
    fi
done
