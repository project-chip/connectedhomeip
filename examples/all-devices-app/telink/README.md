# All Devices App for Telink

This example demonstrates a dynamic Matter device application for the Telink
platform.

The device type is selected at runtime using shell commands and stored in
persistent storage. On the next boot, the stored device type is restored
automatically.

For the general application overview, supported devices, and common usage, see:
[All Devices App](../README.md)

## Build

**Initialize the environment:**

```
source ./scripts/activate.sh -p all,telink
```

**Build:**

```
./scripts/build/build_examples.py --target telink-tlsr9518adk80d-all-devices build
```

**Build output:**

```
out/telink-tlsr9518adk80d-all-devices/
```

**Application binary:**

```
out/telink-tlsr9518adk80d-all-devices/zephyr/merged.bin
```

**Flash:**

```
west flash --erase
```

## UART:

```
screen /dev/ttyUSB0 115200
```

## First Boot

**Set the device type from the device shell:**

```
devtype set <device-type>
```

**Example:**

```
devtype set contact-sensor
```

The Matter server starts after the device type is selected.

## Next boots

The selected device type is restored automatically from persistent storage.

## Change device type

**Factory reset the device:**

```
matter device factoryreset
```

**Reboot the device:**

```
telink reboot
```

**Then select a new device type:**

```
devtype set <device-type>
```

## Commissioning examples

**Wi-Fi:**

```
./chip-tool pairing ble-wifi 1234 <ssid> <password> 20202021 3840
```

**Thread:**

```
./chip-tool pairing ble-thread 1234 hex:${DATASET} 20202021 3840
```

More examples: [All Devices App](../README.md)
