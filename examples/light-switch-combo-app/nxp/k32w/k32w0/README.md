# CHIP K32W061 Light Switch Combo Example Application

The Project CHIP K32W061 Light Switch Combo example provides a baseline demonstration of a 
dual-endpoint application. Endpoint =1 is used for the Light-Switch device and Endpoint = 2 is 
for the Light Device (bulb) . The light bulb is simulated using one of the LEDs from the expansion
board. It uses buttons to test turn on/turn off of the local light bulb or the binded lights. 
You can use this example as a reference for creating your own application.

The example is based on
[Project CHIP](https://github.com/project-chip/connectedhomeip) and the NXP K32W
SDK, and supports remote access and control of a light bulb over a low-power,
802.15.4 Thread network.

The example behaves as a Project CHIP accessory, that is a device that can be
paired into an existing Project CHIP network and can be controlled by this
network.

<hr>

-   [CHIP K32W061 Lighting Example Application](#chip-k32w-lighting-example-application) -
-   [Introduction](#introduction)
    -   [Bluetooth LE Advertising](#bluetooth-le-advertising)
    -   [Bluetooth LE Rendezvous](#bluetooth-le-rendezvous)
-   [Device UI](#device-ui)
-   [Building](#building)
    -   [Known issues](#building-issues)
-   [Manufacturing data](#manufacturing)
-   [Flashing and debugging](#flashdebug)
-   [Pigweed Tokenizer](#tokenizer)
    -   [Detokenizer script](#detokenizer)
    -   [Notes](#detokenizer-notes)
    -   [Known issues](#detokenizer-known-issues)
-   [Tinycrypt ECC operations](#tinycrypt)
    -   [Building steps](#tinycrypt-building-steps)

    </hr>

<a name="intro"></a>

## Introduction

![K32W061 DK6](../../../../platform/nxp/k32w/k32w0/doc/images/k32w-dk6.jpg)

The K32W061 Light Switch Combo example provides a working demonstration of a
dual endpoint application built using the Project CHIP codebase and the NXP K32W061
SDK. The example supports remote access (e.g.: using CHIP Tool from a mobile
phone) and control of a light bulb over a low-power, 802.15.4 Thread network. It
is capable of being paired into an existing Project CHIP network along with
other Project CHIP-enabled devices.

The example targets the
[NXP K32W061 DK6](https://www.nxp.com/products/wireless/thread/k32w061-41-high-performance-secure-and-ultra-low-power-mcu-for-zigbeethread-and-bluetooth-le-5-0-with-built-in-nfc-option:K32W061_41)
development kit, but is readily adaptable to other K32W-based hardware.

The CHIP device that runs the light-switch combo application is controlled by the CHIP
controller device over the Thread protocol. By default, the CHIP device has
Thread disabled, and it should be paired over Bluetooth LE with the CHIP
controller and obtain configuration from it. The actions required before
establishing full communication are described below.

The example also comes with a test mode, which allows to start Thread with the
default settings by pressing a button. However, this mode does not guarantee
that the device will be able to communicate with the CHIP controller and other
devices.

### SE051H Secure Element

Deployment of this firmware configuration requires the K32W061 board setups
using the K32W061 module board, SE051 Expansion board and Generic Expansion
board as shown below:

![SE051H  + K32W061 DK6](../../../../platform/nxp/k32w/k32w0/doc/images/k32w-se.jpg)

The SE051H Secure Element extension may be used for best in class security and
offloading some of the Project CHIP cryptographic operations. Depending on your
hardware configuration, choose one of the options below (building with or
without Secure Element). NOTE: the SE051H is a derivative of the SE051 product
family (see http://www.nxp.com/SE051) including dedicated CHIP support in
addition to the SE051 feature set. See the material provided separately by NXP
for more details on SE051H.

### Bluetooth LE Advertising

In this example, to commission the device onto a Project CHIP network, it must
be discoverable over Bluetooth LE. For security reasons, you must start
Bluetooth LE advertising manually after powering up the device by pressing
Button USERINTERFACE.

### Bluetooth LE Rendezvous

In this example, the commissioning procedure (called rendezvous) is done over
Bluetooth LE between a CHIP device and the CHIP controller, where the controller
has the commissioner role.

To start the rendezvous, the controller must get the commissioning information
from the CHIP device. The data payload is encoded within a QR code, printed to
the UART console and shared using an NFC tag. For security reasons, you must
start NFC tag emulation manually after powering up the device by pressing
Button 4.

### Thread Provisioning

Last part of the rendezvous procedure, the provisioning operation involves
sending the Thread network credentials from the CHIP controller to the CHIP
device. As a result, device is able to join the Thread network and communicate
with other Thread devices in the network.

## Device UI

The example application provides a simple UI that depicts the state of the
device and offers basic user control. This UI is implemented via the
general-purpose LEDs and buttons built in to the OM15082 Expansion board
attached to the DK6 board.

**LED D2** shows the overall state of the device and its connectivity. Four
states are depicted:

-   _Short Flash On (50ms on/950ms off)_ &mdash; The device is in an
    unprovisioned (unpaired) state and is waiting for a commissioning
    application to connect.

*   _Rapid Even Flashing (100ms on/100ms off)_ &mdash; The device is in an
    unprovisioned state and a commissioning application is connected via BLE.

-   _Short Flash Off (950ms on/50ms off)_ &mdash; The device is full
    provisioned, but does not yet have full network (Thread) or service
    connectivity.

*   _Solid On_ &mdash; The device is fully provisioned and has full network and
    service connectivity.

**LED D3** shows the state of the simulated light bulb. When the LED is lit the
light bulb is on; when not lit, the light bulb is off.

**Button SW2** can be used to toggle a binded light device. 
A short Press toggles the light corresponde to binding entry 1.

**Button SW3** can be used to toggle a binded light device or to change the state of 
the simulated light bulb. Short press toogles the light corresponded to bonding entry 2. 
Long press can be used to mimic a user manually operating a switch. The button behaves 
as a toggle, swapping the state every time it is pressed.

**Button SW4** can be used to toggle the peer lights. 
On this example it will send a unicast packet to toggle th light to binding entry 1 and 2.

The remaining two LEDs (D1/D4) and button (SW1) are unused.

Directly on the development board, **Button USERINTERFACE** can be used for
enabling Bluetooth LE advertising for a predefined period of time. Also, pushing
this button starts the NFC emulation by writing the onboarding information in
the NTAG.  **Button USERINTERFACE** long press (more than 3 seconds)  
initiates a factory reset. After an initial period of 3 seconds, LED2
D2 and D3 will flash in unison to signal the pending reset. After 6 seconds will
cause the device to reset its persistent configuration and initiate a reboot.
The reset action can be cancelled by press SW2 button at any point before the 6
second limit.


### Identify cluster LED state

The Identify cluster server supports two identification commands: **Identify**
and **TriggerEffect**. These commands allow a user to identify a particular
device. For these commands, the **LED D3** is used.

The **Identify command** will use the **LED D3** to flash with a period of 0.5
seconds.

The **TriggerEffect command** will use the **LED D3** with the following
effects:

-   _Blink_ &mdash; flash with a 1 second period for 2 seconds
-   _Breathe_ &mdash; flash with a 1 second period for 15 seconds
-   _Okay_ &mdash; flash with a 1 second period for 4 seconds
-   _Channel change_ &mdash; same as Blink
-   _Finish effect_ &mdash; complete current effect sequence and terminate
-   _Stop effect_ &mdash; terminate as soon as possible

<a name="building"></a>

## Building

In order to build the Project CHIP example, we recommend using a Linux
distribution (the demo-application was compiled on Ubuntu 20.04).

-   Download
    [K32W0 SDK 2.6.8](https://cache.nxp.com/lgfiles/bsps/SDK_2_6_8_K32W061DK6.zip).

-   Start building the application either with Secure Element or without
    -   without Secure Element

```
user@ubuntu:~/Desktop/git/connectedhomeip$ export NXP_K32W0_SDK_ROOT=/home/user/Desktop/SDK_2_6_8_K32W061DK6/
user@ubuntu:~/Desktop/git/connectedhomeip$ source ./scripts/activate.sh
user@ubuntu:~/Desktop/git/connectedhomeip$ cd examples/light-switch-combo-app/nxp/k32w/k32w0
user@ubuntu:~/Desktop/git/connectedhomeip/examples/light-switch-combo-app/nxp/k32w/k32w0$ gn gen out/debug --args="k32w0_sdk_root=\"${NXP_K32W0_SDK_ROOT}\" chip_with_OM15082=1 chip_with_ot_cli=0 is_debug=false chip_crypto=\"tinycrypt\" chip_with_se05x=0 chip_pw_tokenizer_logging=false mbedtls_repo=\"//third_party/connectedhomeip/third_party/nxp/libs/mbedtls\""
user@ubuntu:~/Desktop/git/connectedhomeip/examples/light-switch-combo-app/nxp/k32w/k32w0$ ninja -C out/debug
user@ubuntu:~/Desktop/git/connectedhomeip/examples/light-switch-combo-app/nxp/k32w/k32w0$ $NXP_K32W0_SDK_ROOT/tools/imagetool/sign_images.sh out/debug/
```

    -   with Secure element
        Exactly the same steps as above but set chip_with_se05x=1 in the gn command

    -   for K32W041AM flavor:
        Exactly the same steps as above but set build_for_k32w041am=1 in the gn command.
        Also, select the K32W041AM SDK from the SDK Builder.

In case that Openthread CLI is needed, chip_with_ot_cli build argument must be
set to 1.

In case the board doesn't have 32KHz crystal fitted, one can use the 32KHz free
running oscilator as a clock source. In this case one must set the use_fro_32k
argument to 1.

In case signing errors are encountered when running the "sign_images.sh" script
install the recommanded packages (python version > 3, pip3, pycrypto,
pycryptodome):

```
user@ubuntu:~$ python3 --version
Python 3.8.2
user@ubuntu:~$ pip3 --version
pip 20.0.2 from /usr/lib/python3/dist-packages/pip (python 3.8)
user@ubuntu:~$ pip3 list | grep -i pycrypto
pycrypto               2.6.1
pycryptodome           3.9.8
```

The resulting output file can be found in out/debug/chip-k32w0x-light-switch-combo-example.

<a name="building-issues"></a>

## Known issues

-   When using Secure element and cross-compiling on Linux, log messages from
    the Plug&Trust middleware stack may not echo to the console.

<a name="manufacturing"></a>

## Manufacturing data

See
[Guide for writing manufacturing data on NXP devices](../../../../platform/nxp/doc/manufacturing_flow.md).

There are factory data generated binaries available in
examples/platform/nxp/k32w/k32w0/scripts/demo_generated_factory_data folder.
These are based on the DAC, PAI and PAA certificates found in
scripts/tools/nxp/demo_generated_certs folder. The demo_factory_data_dut1.bin
uses the DAC certificate and private key found in
examples/platform/nxp/k32w/k32w0/scripts/demo_generated_factory_data/dac/dut1
folder. The demo_factory_data_dut2.bin uses the DAC certificate and private key
found in
examples/platform/nxp/k32w/k32w0/scripts/demo_generated_factory_data/dac/dut2
folder. These two factory data binaries can be used for testing topologies with
2 DUTS. They contain the corresponding DACs/PAIs generated using
generate_nxp_chip_factory_bin.py script. The discriminator is 14014 and the
passcode is 1000. These demo certificates are working with the CDs installed in
CHIPProjectConfig.h.

<a name="flashdebug"></a>

## Flashing and debugging

Program the firmware using the official
[OpenThread Flash Instructions](https://github.com/openthread/ot-nxp/tree/main/src/k32w0/k32w061#flash-binaries).

All you have to do is to replace the Openthread binaries from the above
documentation with _out/debug/chip-k32w0x-light-switch-combo-example.bin_ if DK6Programmer is
used or with _out/debug/chip-k32w0x-light-switch-combo-example_ if MCUXpresso is used.

<a name="tokenizer"></a>

## Pigweed tokenizer

The tokenizer is a pigweed module that allows hashing the strings. This greatly
reduces the flash needed for logs. The module can be enabled by building with
the gn argument _chip_pw_tokenizer_logging=true_. The detokenizer script is
needed for parsing the hashed scripts.

<a name="detokenizer"></a>

### Detokenizer script

The python3 script detokenizer.py is a script that decodes the tokenized logs
either from a file or from a serial port. It is located in the following path
`examples/platform/nxp/k32w/k32w0/scripts/detokenizer.py`.

The script can be used in the following ways:

```
usage: detokenizer.py serial [-h] -i INPUT -d DATABASE [-o OUTPUT]
usage: detokenizer.py file [-h] -i INPUT -d DATABASE -o OUTPUT
```

The first parameter is either _serial_ or _file_ and it selects between decoding
from a file or from a serial port.

The second parameter is _-i INPUT_ and it must se set to the path of the file or
the serial to decode from.

The third parameter is _-d DATABASE_ and represents the path to the token
database to be used for decoding. The default path is
_out/debug/chip-k32w0x-light-switch-combo-example-database.bin_ after a successful build.

The forth parameter is _-o OUTPUT_ and it represents the path to the output file
where the decoded logs will be stored. This parameter is required for file usage
and optional for serial usage. If not provided when used with serial port, it
will show the decoded log only at the stdout and not save it to file.

<a name="detokenizer-notes"></a>

### Notes

The token database is created automatically after building the binary if the
argument _chip_pw_tokenizer_logging=true_ was used.

The detokenizer script must be run inside the example's folder after a
successful run of the _scripts/activate.sh_ script. The pw_tokenizer module used
by the script is loaded by the environment. An example of running the
detokenizer script to see logs of a lighting app:

```
python3 ../../../../../examples/platform/nxp/k32w/k32w0/scripts/detokenizer.py serial -i /dev/ttyACM0 -d out/debug/chip-k32w0x-light-switch-combo-example-database.bin -o device.txt
```

<a name="detokenizer-known-issues"></a>

### Known issues

The building process will not update the token database if it already exists. In
case that new strings are added and the database already exists in the output
folder, it must be deleted so that it will be recreated at the next build.

Not all tokens will be decoded. This is due to a gcc/pw_tokenizer issue. The
pw_tokenizer creates special elf sections using attributes where the tokens and
strings will be stored. This sections will be used by the database creation
script. For template C++ functions, gcc ignores these attributes and places all
the strings by default in the .rodata section. As a result the database creation
script won't find them in the special-created sections.

If run, closed and rerun with the serial option on the same serial port, the
detokenization script will get stuck and not show any logs. The solution is to
unplug and plug the board and then rerun the script.

<a name="tinycrypt"></a>

## Tinycrypt ECC operations

<a name="tinycrypt-building-steps"></a>

### Building steps

Note: This solution is temporary.

In order to use the tinycrypt ecc operations, use the following build arguments:

-   Build without Secure element (_chip_with_se05x=0_), with tinycrypt enabled
    (_chip_crypto=\"tinycrypt\"_) and with the `NXPmicro/mbedtls` library
    (_mbedtls_repo=`\"//third_party/connectedhomeip/third_party/nxp/libs/mbedtls\"`_).

To disable tinycrypt ecc operations, simply build with _chip_crypto=\"mbedtls\"_
and with or without _mbedtls_repo_. If used with _mbedtls_repo_ the mbedtls
implementation from `NXPmicro/mbedtls` library will be used.

## Running demo:

### 1 x light-switch-combo, 2 x lighting-app

Pairing the devices:
 ```
$./chip-tool pairing ble-thread <NodeId> hex:<OT operational dataset> 20202021 3840
```
Write an access control table entry for the switch on both lights:
 ```
$./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null },{"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [<Switch NodeId>], "targets": null }]' <Light NodeId> 0
 ```
Bind the first light:
 ```
$./chip-tool chip-tool binding write binding '[{"fabricIndex": 1, "node": <First Light NodeId>, "endpoint": <EndpointLight>, "cluster": 6}]' <Switch NodeID> 1
 ```
Press any of the SW2, SW3 or SW4 from the OM15082 expansion board, this way you will register the light device and a log confirming that will be printed

Bind the second  light:
 ```
$./chip-tool chip-tool binding write binding '[{"fabricIndex": 1, "node": <Second Light NodeId>, "endpoint": <EndpointLight>, "cluster": 6}]' <Switch NodeID> 1
 ```
Press any of the SW2, SW3 or SW4 from the OM15082 expansion board, this way you will register the light device and a log confirming that will be printed
After this step, you should be able to control the nodes from the light switch. 