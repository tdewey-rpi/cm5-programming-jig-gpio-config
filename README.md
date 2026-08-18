# cm5-programming-jig-gpio-config

Configuration files for the Raspberry Pi CM5 Programming Jig.

## GPIO Pinout

Assumes you connect the following test points to the host CM5:

| GPIO Number | Purpose |
|------------|---------|
| 16 | 5V Relay Control |
| 14 | Client Device UART Receive | 
| 15 | Client Device UART Transmit |
| 17 | Client Device nRPIBOOT (Pull low to activate) |
| 9 | Provisioning Status LED - BLUE  |
| 10 | Provisioning Complete LED - GREEN |
| 11 | Provisioning Status LED - RED |
| 24 | Pressure Switch Detect (Pull-up) |
| 26 | Buzzer Control |

## Status LED behaviour

| State | LED | When |
|-------|-----|------|
| Power on | Red solid | Hardware pull-up on GPIO 11 (no software required) |
| Ready | Green solid | `rpi-provisioner-ui` active (`cm5-provisioner-led-ready.service`) |
| In progress | Blue blink (2 Hz) | Bootstrap hook (USB device detected) through flash complete |
| Success | Green solid | `post-flash` hook; returns to green ready on lever release |
| Failure | Red blink (2 Hz) | `provision-failed` hook, on any provisioning-phase failure and on permanent misconfiguration during bootstrap; returns to green ready on lever release |

Bootstrap-phase failures are filtered, because most of them are the expected USB re-enumeration as the device reboots and lighting red for those would flash a failure on every successful run. Permanent misconfiguration is not filtered: no signing key, or no OS image selected, will never resolve on retry, so it lights red rather than leaving the head on blue in-progress with nothing to tell the operator. This relies on `PROVISION_FAILED_PERMANENT`, which `rpi-sb-provisioner` sets from 2.3.2; against older versions the hooks behave exactly as before.

Hooks are installed for `sb-`, `fde-`, `naked-`, and `idp-provisioner`.

## What This Package Installs

- **Effector scripts** (`/usr/bin/`) -- low-level GPIO control for power, RPIBOOT, and status LEDs.
- **LED daemon** (`cm5-provisioner-led.service`) -- C++20/libgpiod service owning status LED GPIOs 9/10/11; hooks call the `cm5_led_blink` client.
- **Trigger configuration** (`/etc/gpio/`) -- binds GPIO 24 (pressure switch) to the power-on/power-off effectors via `rpi-systemd-gpio`.
- **systemd units** (`cm5-provisioner-led.service`, `cm5-provisioner-led-ready.service`) -- LED daemon and solid green ready indicator once the provisioner stack is up.
- **rpi-sb-provisioner hooks** (`/etc/rpi-sb-provisioner/scripts/`) -- integrates with `rpi-sb-provisioner` so that status LEDs and RPIBOOT are driven automatically during provisioning.
- **USB port filter** (`/usr/share/rpi-sb-provisioner/usb-ports.d/50-cm5-jig.conf`) -- restricts provisioning to the jig head's USB port (`4-1`). See [USB port filter](#usb-port-filter) below.

## USB port filter

By default `rpi-sb-provisioner` programs any Raspberry Pi that appears on any
USB port of the host. On a single-headed jig that is wrong: only the port wired
to the head should be picked up, and a board attached anywhere else -- a bench
cable, something left connected for debugging -- must be left alone.

This package ships a drop-in rule file to
`/usr/share/rpi-sb-provisioner/usb-ports.d/50-cm5-jig.conf` restricting
provisioning to `4-1` -- bus 4, port 1 -- which is the jig head.

On the CM5 Lite host that is a root port of the second xHCI controller
(`.../1f00300000.usb/xhci-hcd.1/usb4`), wired directly to the head with no
intervening hub, so the path is fixed by the carrier layout rather than by
enumeration order. It belongs to the physical port rather than the board, so it
is stable as boards are swapped through the head, and across the USB
re-enumeration a device performs between the bootstrap and fastboot phases.

### Checking it against your jig

The value was measured on a PVT jig. To confirm it on yours, seat a board in
the head, press the lever to power it up, and list the boards the provisioner
would act on:

```
for d in /sys/bus/usb/devices/[0-9]*-[0-9]*; do
    [ -f "$d/idVendor" ] || continue
    case "$(cat "$d/idVendor"):$(cat "$d/idProduct")" in
      0a5c:2764|0a5c:2711|0a5c:2712|18d1:4e40) echo "$(basename "$d")" ;;
    esac
done
```

Ports the provisioner has already used are also on record:

```
sudo sqlite3 /srv/rpi-sb-provisioner/state.db 'SELECT DISTINCT endpoint FROM devices;'
```

### Correcting or disabling it

If the port differs on your hardware, put the right value in a same-named file
under `/etc`, which takes precedence over the copy this package ships:

```
sudo cp /usr/share/rpi-sb-provisioner/usb-ports.d/50-cm5-jig.conf \
        /etc/rpi-sb-provisioner/usb-ports.d/50-cm5-jig.conf
sudo editor /etc/rpi-sb-provisioner/usb-ports.d/50-cm5-jig.conf
```

Edit the `/etc` copy, never the one under `/usr/share`: files in `/usr/share`
are replaced wholesale on every package upgrade, so an edit made there is
silently lost. An *empty* `/etc` file of that name disables this package's rule
entirely, restoring the unrestricted behaviour. No service restart is needed --
the rule files are read afresh each time a device appears.

A board on a port that is not permitted is skipped rather than failed: the
reason is logged, a `PORT-EXCLUDED` state is recorded so the skip is visible in
the web UI, and no provisioning is attempted. The jig's status LEDs are driven
by the provisioning hooks, which do not run for a skipped device, so the head
stays on its ready indication.

Requires `rpi-sb-provisioner` 2.3.2 or later.

## Building

Build dependencies: `debhelper (>= 13)`, `pandoc`, `cmake`, `pkg-config`, `libgpiod-dev`, `g++`.

Install them if needed:

```
sudo apt install debhelper pandoc cmake pkg-config libgpiod-dev g++
```

Then build the `.deb` from the repository root:

```
dpkg-buildpackage -us -uc
```

The resulting package will be placed in the parent directory (e.g. `../cm5-programming-jig-gpio-config_<version>_arm64.deb`).

## Installing

Install the built package with:

```
sudo dpkg -i ../cm5-programming-jig-gpio-config_*.deb
sudo apt-get install -f
```

The `apt-get install -f` step resolves any missing dependencies (`rpi-systemd-gpio`, `rpi-sb-provisioner`).

Alternatively, if the package is available from a configured APT repository:

```
sudo apt install cm5-programming-jig-gpio-config
```
