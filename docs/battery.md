# Battery and fuel gauge

[简体中文](battery.zh_CN.md)

## Hardware reference

The [AI Passport product specifications](https://github.com/FoloToy/ai-passport/blob/1178e40713455754ec9a6a2dd8e5dde46f80e513/docs/hardware-design/specifications.md)
describe a built-in 500 mAh rechargeable lithium battery, USB Type-C 5 V input,
and a CellWise CW2017 fuel gauge. These specifications apply to the documented
board and cell; check the hardware revision before using them for another device.

The CW2017 uses 7-bit I2C address `0x63` and shares Wire with the ES8311 codec:
SDA GPIO10, SCL GPIO7, 100 kHz. The [main repository battery driver](https://github.com/FoloToy/ai-passport/blob/1178e40713455754ec9a6a2dd8e5dde46f80e513/components/bsp/src/bsp_battery.c)
is the reference for register interpretation and cell configuration.

## Readings

- `passport.battery.percent()` reads the integer SOC byte at `0x04`, returning
  0–100 percent. It returns `-1` on a failed read or an invalid SOC value.
- `passport.battery.millivolts()` reads the 14-bit voltage at `0x02–0x03`.
  Conversion: `mV = (raw & 0x3fff) * 3125 / 10000`, using integer arithmetic.
  It returns `-1` on a failed read.
- `passport.battery.version()` reads `0x00`. A successful response establishes
  communication, not that the cell profile is correct or SOC is ready.

SOC comes from the gauge's configured cell model. This library does not estimate
percentage from voltage. These APIs do not report charging status, charge current,
remaining runtime, or measured battery capacity.

## Configuration and power management

The referenced main firmware checks both the profile update flag and the complete
80-byte profile for its specified 500 mAh cell. The capacity correction retains
the existing profile bytes; it does not establish a new calibration. If necessary, it writes the profile
in sleep mode, verifies it, sets the update flag, restarts the gauge, and waits up
to five seconds for a valid SOC. It also provides a gauge sleep operation.

The current Arduino driver is read-only: `begin()` probes the version register;
it does not configure a profile, wake/restart the gauge, or manage gauge sleep.
`AIPassport::end()` does not put the gauge into sleep. A sleeping or unconfigured
gauge therefore requires configuration/wake handling outside the current API.
Do not reuse the main firmware's cell profile for a different battery without
validating the cell parameters.

## Example and verification

[BatteryMonitor](../examples/BatteryMonitor/BatteryMonitor.ino) enables
`PassportConfig::battery` and prints SOC and millivolts once per second.
Treat `-1` as unavailable; do not display it as zero percent.

Verify plausible readings with USB connected and disconnected, gradual changes
while charging/discharging, and I2C reads during audio capture/playback. One valid
reading does not establish SOC accuracy or usable capacity. Record charge/discharge
acceptance separately from compilation and basic communication checks.

Reference snapshot: `1178e40713455754ec9a6a2dd8e5dde46f80e513`. Follow the main repository for later hardware or
cell-profile changes; this document does not claim automatic synchronization.

The corrected reference is proposed in [AI Passport PR #101](https://github.com/FoloToy/ai-passport/pull/101), pending upstream merge.
