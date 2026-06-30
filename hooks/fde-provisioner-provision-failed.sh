#!/bin/sh
# rpi-fde-provisioner provision-failed hook for CM5 provisioning rig
#
# Bootstrap-phase failures include expected USB re-enumeration while the DUT
# reboots (duplicate bootstrap@, transient triage). Keep blue in-progress;
# red is only for provisioning-phase failure.

if [ "${PROVISION_FAILED_CONTEXT:-provisioning}" = "bootstrap" ]; then
    exit 0
fi

/usr/bin/provisioning_failed

exit 0
