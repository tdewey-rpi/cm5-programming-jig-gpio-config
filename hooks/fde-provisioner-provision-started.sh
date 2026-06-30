#!/bin/sh
# rpi-fde-provisioner provisioning-started hook for CM5 provisioning rig
# Arguments: $1=fastboot_device_specifier $2=serial $3=storage_type
#
# Re-assert blue blink at flash start (bootstrap also starts it; idempotent).

set -e

/usr/bin/provisioning_active

exit 0
