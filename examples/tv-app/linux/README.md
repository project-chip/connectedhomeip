# Matter TV Example

An example showing the use of CHIP on the Linux. The document will describe how
to build and run Matter TV Example on Raspberry Pi. This doc is tested on
**Ubuntu for Raspberry Pi Server 20.04 LTS (aarch64)** and **Ubuntu for
Raspberry Pi Desktop 20.10 (aarch64)**

<hr>

-   [Matter TV Example](#matter-tv-example)
    -   [Building](#building)
    -   [Exercising Commissioning](#exercising-commissioning)
    -   [App Platform commands](#app-platform-commands)
    -   [Casting](#casting)
    -   [Running the Complete Example on Raspberry Pi 4](#running-the-complete-example-on-raspberry-pi-4)

<hr>

## Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application:

          $ cd ~/connectedhomeip/examples/tv-app/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/tv-app/linux
          $ rm -rf out/

## Exercising Commissioning

-   Regular Commissioning

Start the tv-app. Set ports to not conflict with other Matter apps you might run
on the same machine (chip-tool, tv-casting-app, etc)

    $ ./out/debug/chip-tv-app --secured-device-port 5640 --secured-commissioner-port 5552

Using the tv-app shell, invoke the controller commands:

Print out all controller commands (discovery, commissioning, etc.)

    $ controller help

Try the "commission-onnetwork <pincode> <disc> <IP> <port>" command

    $ controller commission-onnetwork 34567890 2976 192.168.65.3 5540 -or-
    $ controller commission-onnetwork 34567890 2976 fe80::50:ff:fe00:1 5540

When the tv-app receives a UDC request from a commissionable node (like a
tv-casting-app), it will print out

```
CHIP:SVR: UserDirectedCommissioningServer::OnMessageReceived
CHIP:SVR: UDC instance=F879911BF17129AA
CHIP:SVR: OnCommissionableNodeFound instance: name=F879911BF17129AA old_state=1 new_state=3
CHIP:CTL: ------PROMPT USER: Test TV casting app is requesting permission to cast to this TV, approve? [0x0000_FFF1,0x0000_8001,F879911BF17129AA,01005DD9BB0990AF18F19C35C30C670532BC]
CHIP:CTL: ------Via Shell Enter: controller ux ok|cancel
```

Begin commissioning it by running

    $ controller ux ok

-   User Directed Commissioning (UDC)

Print out the cached list of UDC sessions

    $ udc-print

Commission an entry from this UDC session cache. This will allow you to skip
entering discriminator, IP and port because these will be taken from the UDC
session cache:

    $ udc-commission <pincode> <udc-entry>
    $ udc-commission 34567890 0

## App Platform commands

As an app platform, Content Apps can be launched and assigned to endpoints
following (see Video Player Architecture in the Device Library spec).

There is a dummy app platform included in the linux tv-app which includes a
small number of hardcoded apps. See `examples/tv-app/tv-common/src/AppTv.h` and
`examples/tv-app/tv-common/src/AppTv.cpp` for this dummy implementation. These
apps have hardcoded values for many operations - on a real device, these apps
would usually be developed by streaming video content providers and the native
platform may or may not provide Matter interfaces to these apps. In some cases,
the video player platform will bridge its existing internal interfaces to
Matter, allowing apps to continue to not be Matter-aware, while other platforms
may provide Matter interfaces to Content Apps so that they can directly respond
to each Matter cluster.

On Linux, there are shell commands to start and stop the dummy apps (by vendor
id):

    $ app add 1 (vendor id 1)
    $ app add 2 (vendor id 2)
    $ app add 9050 (vendor id 9050)
    $ app remove 1

As an app platform, local apps can be used to facilitate commissioning using
their AccountLogin clusters. The dummy apps have hardcoded setup codes - on a
real device, these apps would communicate with a cloud service to obtain the
setup code given a rotating id from a tv-casting-app (eg. a phone app). You can
change the setup code for a given Content App using "setpin <vid> <pincode>":

    $ setpin 9050 20202021
    $ setpin 9050 34567890

When a UDC message comes from a vendor id that maps to a ContentApp in the
ContentAppPlatform, the AccountLogin cluster of the ContentApp endpoint will be
given the chance to obtain a setup code. You can trigger this process using "app
commission <udc-entry>". <udc-entry> will usually be 0 (when there is only one
entry in the UDC cache):

    $ app commission 0

-   App Launching from chip-tool

You can use chip-tool to launch apps by invoking the Application Launcher
cluster on endpoint 1 using chiptool:

    $ ./out/debug/chip-tool applicationlauncher launch-app Data CatalogVendorId ApplicationId node-id endpoint-id
    $ ./out/debug/chip-tool applicationlauncher launch-app foo1 1 App2 1234 1

-   Target Navigation from chip-tool

You can use chip-tool to navigate among targets on endpoint 1 (main video
player) and on Content App endpoints:

Read targets for a given endpoint:

    $ ./out/debug/chip-tool targetnavigator read attr-name node-id endpoint-id
    $ ./out/debug/chip-tool targetnavigator read target-navigator-list 1234 1 (video player endpoint 1)
    $ ./out/debug/chip-tool targetnavigator read target-navigator-list 1234 6 (content app endpoint 6 - requires app to be launched)

Navigate to a new target:

    $ ./out/debug/chip-tool targetnavigator navigate-target Target Data node-id endpoint-id
    $ ./out/debug/chip-tool targetnavigator navigate-target 2 foo1 1234 6 (target id 2 on endpoint 6)

## Casting

The tv-casting-app can be used to discover casting video players, selecting one,
sending a UDC message, get commissioned, and then send commands to the video
player and/or a Content App on it.

-   Start the Apps

Start the tv-app:

    $ ./out/debug/chip-tv-app --secured-device-port 5640 --secured-commissioner-port 5552

Start the tv-casting-app:

    $ ./out/debug/chip-tv-casting-app

TV casting app should discover video players on the network. Into the shell,
enter "1" to select the first one in the list:

    $ 1

TV casting app will send a UDC command to the selected video player. The tv-app
should print out receipt of the UDC message.

-   Commission the Casting App

If the VID for the tv-casting-app matches the vid for a Content App, then you
can initiate commissioning using the Account Login cluster of the Content App:

    $ app commission <udc cache index>
    $ app commission 0

If the VID does not match a ContentApp or the Account Login cluster of the
ContentApp endpoint does not provide the correct setup code, initiate
commissioning by entering the setup code into the shell:

    $ udc-commission <pincode> <udc-entry>
    $ udc-commission 34567890 0

-   Send commands from the Casting App

TODO

## Running the Complete Example on Raspberry Pi 4

-   Prerequisites

    1. A Raspberry Pi 4 board
    2. A USB Bluetooth Dongle, Ubuntu desktop will send Bluetooth advertisement,
       which will block Matter from connecting via BLE. On Ubuntu server, you
       need to install `pi-bluetooth` via APT.
    3. Ubuntu 20.04 or newer image for ARM64 platform.

-   Building

    Follow [Building](#building) section of this document.

-   Running

    -   [Optional] Plug USB Bluetooth dongle

        -   Plug USB Bluetooth dongle and find its bluetooth device number. The
            number after `hci` is the bluetooth device number, `1` in this
            example.

                  $ hciconfig
                  hci1:	Type: Primary  Bus: USB
                      BD Address: 00:1A:7D:AA:BB:CC  ACL MTU: 310:10  SCO MTU: 64:8
                      UP RUNNING PSCAN ISCAN
                      RX bytes:20942 acl:1023 sco:0 events:1140 errors:0
                      TX bytes:16559 acl:1011 sco:0 commands:121 errors:0

                  hci0:	Type: Primary  Bus: UART
                      BD Address: B8:27:EB:AA:BB:CC  ACL MTU: 1021:8  SCO MTU: 64:1
                      UP RUNNING PSCAN ISCAN
                      RX bytes:8609495 acl:14 sco:0 events:217484 errors:0
                      TX bytes:92185 acl:20 sco:0 commands:5259 errors:0

        -   Run TV Example App

                  $ cd ~/connectedhomeip/examples/tv-app/linux
                  $ sudo out/debug/chip-tv-app --ble-device [bluetooth device number]
                  # In this example, the device we want to use is hci1
                  $ sudo out/debug/chip-tv-app --ble-device 1

        -   Test the device using ChipDeviceController on your laptop /
            workstation etc.
