CHIP BL602 Lighting Example

## Supported Devices

The CHIP demo application is intended to work on three categories of BL602 devices.
See [Demo board](https://dev.bouffalolab.com/product)

## Build

-   Setting up the environment

```
$ sudo apt-get update
$ sudo apt-get upgrade
$ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev ninja build python3-venv python3-dev python3-pip unzip libgirepository1.0-dev libcairo2-dev bluez avahi-daemon
$ sudo apt-get install pi-bluetooth (if not raspberry pi sudo apt-get install bluetooth)
$ reboot

```

- Install packets

```
$ cd {path-to-connectedhomeip}
connectedhomeip$ source ./scripts/bootstrap.sh
connectedhomeip$ source ./scripts/activate.sh

```

-   Build the example application:

    `connectedhomeip$ ./scripts/build/build_examples.py --target bl602-lighting build`

Generated files

         connectedhomeip/out/lighting_app_bl602/chip-bl602-lighting-example.bin

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/
          $ rm -rf out/

## Flash

-   Download [FLASH_TOOL](https://dev.bouffalolab.com/download/).

### Setting up chip-tool

See [the build guide](../../../docs/guides/BUILDING.md#prerequisites) for
general background on build prerequisites.

Building the example:

- Set up python controller.

      $ cd examples/chip-tool
      
      $ rm -rf out
      
      $ gn gen out/debug
      
      $ ninja -C out/debug

​       which puts the binary at `out/debug/chip-tool`

### Commission a device using chip-tool

To initiate a client commissioning request to a device, run the built executable
and choose the pairing mode.

#### Commissioning over BLE

Run the built executable and pass it the discriminator and pairing code of the
remote device, as well as the network credentials to use.

The command below uses the default values hard-coded into the debug versions of
the BL602 lighting-app to commission it onto a Wi-Fi network:

```
$ ./out/debug/chip-tool pairing ble-wifi 123456 ${SSID} ${PASSWORD} 20202021 3904

 Parameters:
 1. Discriminator: 3904
 2. Setup-pin-code: 20202021
 3. Node ID: 123456
 4. SSID : Wi-Fi SSID
 5. PASSWORD : Wi-Fi Password
```

### Cluster control

-   After successful commissioning, use the OnOff cluster commands to control
    the OnOff attribute. This allows you to toggle a parameter implemented by
    the device to be On or Off.

    `$ ./out/debug/chip-tool onoff on 123456 1`
    
-   Use ColorControl cluster command to control the color attributes:
    
    `$ ./out/debug/chip-tool colorcontrol move-to-hue-and-saturation 240 100 0 0 0 12345 1`

