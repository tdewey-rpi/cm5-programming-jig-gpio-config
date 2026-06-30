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
| Failure | Red blink (2 Hz) | `provision-failed` hook (provisioning phase only); returns to green ready on lever release |

Hooks are installed for `sb-`, `fde-`, `naked-`, and `idp-provisioner`.

## What This Package Installs

- **Effector scripts** (`/usr/bin/`) -- low-level GPIO control for power, RPIBOOT, and status LEDs.
- **LED daemon** (`cm5-provisioner-led.service`) -- C++20/libgpiod service owning status LED GPIOs 9/10/11; hooks call the `cm5_led_blink` client.
- **Trigger configuration** (`/etc/gpio/`) -- binds GPIO 24 (pressure switch) to the power-on/power-off effectors via `rpi-systemd-gpio`.
- **systemd units** (`cm5-provisioner-led.service`, `cm5-provisioner-led-ready.service`) -- LED daemon and solid green ready indicator once the provisioner stack is up.
- **rpi-sb-provisioner hooks** (`/etc/rpi-sb-provisioner/scripts/`) -- integrates with `rpi-sb-provisioner` so that status LEDs and RPIBOOT are driven automatically during provisioning.

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
