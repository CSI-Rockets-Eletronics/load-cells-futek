#!/bin/bash

files=(/sys/bus/usb/drivers/ftdi_sio/*)
for file in ${files[@]}; do
    regex='[0-9]+-[0-9]+\.+[0-9]+\.+[0-9]+:+[0-9]+\.+[0-9]'
    if [[ $file =~ $regex ]]
    then
    echo -n ${file##*/} | sudo tee /sys/bus/usb/drivers/ftdi_sio/unbind
    echo "Hi"
    fi
done
