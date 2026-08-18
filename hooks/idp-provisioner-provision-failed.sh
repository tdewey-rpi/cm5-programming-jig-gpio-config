#!/bin/sh
# rpi-idp-provisioner provision-failed hook for CM5 provisioning rig
#
# Bootstrap-phase failures are a mix. Most are the expected USB
# re-enumeration while the DUT reboots (duplicate bootstrap@, transient
# triage), which must not light red or every successful run would flash a
# failure on its way past. But some are permanent misconfiguration -- no
# signing key, no OS image selected -- which will never resolve on retry, and
# suppressing those left the head sitting on blue in-progress indefinitely
# with nothing to tell the operator anything was wrong.
#
# rpi-sb-provisioner (>= 2.3.2) distinguishes the two with
# PROVISION_FAILED_PERMANENT, so only the genuinely transient case is
# swallowed. Older provisioners do not set it; there the default keeps the
# previous behaviour exactly.

if [ "${PROVISION_FAILED_CONTEXT:-provisioning}" = "bootstrap" ] && \
   [ "${PROVISION_FAILED_PERMANENT:-0}" != "1" ]; then
    exit 0
fi

/usr/bin/provisioning_failed

exit 0
