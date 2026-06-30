#!/bin/sh
# rpi-naked-provisioner bootstrap hook for CM5 provisioning rig
# Called when a device is detected, before provisioning begins
# Arguments: $1=serial $2=device_family $3=usb_path $4=device_path

# Target USB device is present; start the in-progress indicator
/usr/bin/provisioning_active

exit 0
