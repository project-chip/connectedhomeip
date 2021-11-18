# CHIP Linux switch Example

An example showing the use of CHIP binding on Linux.

After binding, all the changes on the On/Off cluster will be reflected on the bound
device.

## Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application:

          $ cd ~/connectedhomeip/examples/switch-app/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/lighting-app/linux
          $ rm -rf out/


## Running the example

We will use the controller to bind the switch to another device with On/Off cluster support. The peer device can be [all-cluster-app](../../all-cluster-app) or [lighting-app](../../lighting-app).

* Pair the peer device with command:

```
chip-tool pairing ble-wifi 1234 $SSID $PSK 0 $PAIRING_CODE $DISCRIMINATOR
```

* Run the switch app:
```
./out/debug/chip-switch-app
```

* Pair the switch app:
```
chip-tool pairing ethernet 1235 20202021 3840 127.0.0.1 5540
```

* Setup the binding:
```
chip-tool binding bind 1234 0 1 6 1235 1
```

* Toggle the On/Off cluster on the switch:
```
chip-tool onoff toggle 1235 1
```

You'll find the On/Off cluster on the peer device also toggled.
