#!/bin/sh
# rpi-fde-provisioner post-flash hook for CM5 provisioning rig
# Arguments: $1=fastboot_device_specifier $2=serial $3=storage_type

/usr/bin/disable_rpiboot
/usr/bin/provisioning_complete

exit 0
