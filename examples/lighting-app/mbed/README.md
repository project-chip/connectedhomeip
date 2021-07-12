# CHIP ARM-MbedOS  Lighting Example Application


The ARM-MbedOS Lighting Example demonstrates how to remotely control  a white
dimmable light source. It uses various input primitives to test changing  the lighting and device
states and LED to show the state of lighting device. Depending on target device also linear  touchpad "slider" is available so light can also be dimmed via PWM output instead of simple ON/OFF toggling. 
 You can use this example as a reference for creating your own application.


The example is based on  [CHIP](https://github.com/project-chip/connectedhomeip) and ARM MbedOS, and supports remote access and control of  a lighting over  Wi-Fi network.

The example behaves as a CHIP accessory, that is a device that can be paired into an existing CHIP network and can be controlled by this network.

<hr>

-   [CHIP ARM-MbedOS Lighting Example Application](#main_title)
	-   [Overview](#overview)
    -   [Bluetooth LE advertising](#bluetooth-le-advertising)
    -   [Bluetooth LE rendezvous](#bluetooth-le-rendezvous)
    -   [WI-FI provisioning](#wifi_provisioning)

-   [Building and Flashing](#building_flashing)
    -   [Development environment setup](#environment_setup)
    -   [Building](#building)
    -   [Flashing](#flashing)
    -   [Testing the example](#testing_cli)
    -   [Testing using CHIPTool](#testing_chiptool)
    -   [Supported devices](#supported_devices)
        -   [Notes](#notes)
-   [Device UI](#device_ui)
 

<hr> 


<a name="overview"></a>
## Overview


The CHIP device that runs the lighting application is controlled by the CHIP
controller device over WIFI. By default, the CHIP device is disconnected
, and it should be paired with CHIP controller and get
configuration from it. Actions required before establishing full
communication are described below.

<a name="bluetooth-le-advertising"></a>
### Bluetooth LE advertising

To commission the device onto a CHIP network, the device must be discoverable
over Bluetooth LE. . The Bluetooth LE advertising starts
automatically after device boot-up.

<a name="bluetooth-le-rendezvous"></a>
### Bluetooth LE rendezvous

In CHIP, the commissioning procedure (called rendezvous) is done over Bluetooth
LE between a CHIP device and the CHIP controller, where the controller has the
commissioner role.

To start the rendezvous, the controller must get the commissioning information
from the CHIP device. The data payload is encoded within a QR code, printed to
the UART console.

<a name="wifi_provisioning"></a>
#### WiFi provisioning

Last part of the rendezvous procedure, the provisioning operation involves
sending the  network credentials from the CHIP controller to the CHIP
device. As a result, device is able to join the  network and communicate
with other  devices in the network.



<a name="building_flashing"></a>
# Building and Flashing

<a name="environment_setup"></a>
## Development environment setup

Before building the example, check out the CHIP repository and sync submodules
using the following command:

        $ git submodule update --init

Building the example application requires the use of **ARM Mbed-OS** sources 
and **arm-none-gnu-eabi** toolchain. The OpenOCD package is used for 
flashing purpose.
<br>
Some additional packages may be needed, depending on selected build target 
and its requirements.

> **The VSCode devcontainer has these components pre-installed, so using 
the VSCode devcontainer is recommended way to interact with ARM Mbed-OS 
port of the CHIP Project.**    
>    
> **Please read this [README.md](../../..//docs/VSCODE_DEVELOPMENT.md) for 
more information about using VSCode in container.**
To initialize development environment, download all registered sub-modules 
and activate development environment:
```
$ source ./scripts/bootstrap.sh    
$ source ./scripts/activate.sh
```
If packages are already installed then simply just activate development 
environment:
```
$ source ./scripts/activate.sh
```
<a name="building"></a>
## Building

The Lock-app application can be built in the same way as any other CHIP 
example ported to mbed-os platform.
<br>
* **by using generic vscode task**:  
```
Terminal => Run Task... => Run Mbed application => build => (example name) => (board name) => (build profile)`    
```  
<br>

* **by calling explicitly building script:**
```
${HOME}/scripts/examples/mbed_example.sh -c build -a <example name> -b <board name> -p <build profile>
```
Both approaches are limited only to supported evaluation boards which are 
listed in [Supported devices](#supported_devices) paragraph. 
<br>When using building script, it is possible expand its list of acceptable 
targets which may be useful for rapid testing a new mbed-targets.
<br><br>For more detailed description about porting mbed targets into the 
CHIP project, please read **[LINK HERE]**

<a name="flashing"></a>
## Flashing

The Lock-app application can be flashed in the same way as any other CHIP 
example ported to mbed-os platform.
<br>
* **by using generic vscode task**: 
```
Terminal => Run Task... -> Run Mbed application => flash => (example name) => (board name) => (build profile)
```
<br>

* **by calling explicitly building script:**
```
${HOME}/scripts/examples/mbed_example.sh -c flash -a <example name> -b <board name> -p <build profile>
```

<a name="testing_cli"></a>
## Testing the example

Check the [**LINK HERE**] to learn how to use command-line interface 
of the application.

<a name="testing_chiptool"></a>
## Testing using CHIPTool

Read the
[**LINK HERE**] to see how to use 
[CHIPTool](../../../src/android/CHIPTool/README.md) for
Android smartphones to commission and control the application 
within a Wi-Fi network.

<a name="supported_devices"></a>
## Supported devices

The example supports building and running on the following mbed-enabled devices:

| Manufacturer | Hardware platform | Build target | Platform image | Status |
| ------------ | ----------------- | ------------ | -------------- | :----: |
| [Cypress<br> Semiconductor](https://www.cypress.com/) | [CY8CPROTO-062-4343W](https://os.mbed.com/platforms/CY8CPROTO-062-4343W/) | `CY8CPROTO_062_4343W` | <details><summary>CY8CPROTO-062-4343W</summary><img src="https://os.mbed.com/media/cache/platforms/p6_wifi-bt_proto.png.250x250_q85.jpg" alt="CY8CPROTO-062-4343W"/></details> | :heavy_check_mark: |
| [STMicroelectronics](https://www.st.com) | [DISCO-L475VG-IOT01A](https://os.mbed.com/platforms/ST-Discovery-L475E-IOT01A/) | `DISCO_L475VG_IOT01A` | <details><summary>DISCO-L475VG-IOT01A</summary><img src="https://os.mbed.com/media/cache/platforms/DISCO_L475VG_IOT01A.jpg.250x250_q85.jpg"/></details> | :x: |

There is a plan to expend above list with new platforms within mbed-os ecosystem.

<a name="notes"></a>
#### Notes

* More details and guidelines about porting new hardware into the CHIP project withing mbed-os platform can be found in **[LINK HERE]**
* Some useful information about HW platform specific settings can be found in `lock-app/mbed/mbed_app.json`.    
Information about this file syntax and its meaning in project building can be found here: [Mbed-Os configuration system](https://os.mbed.com/docs/mbed-os/v6.12/program-setup/advanced-configuration.html))
ProTip! Use n and p to navigate between commits in a pull request.

 <hr>
<a name="device_ui"></a>
# Device UI

This section lists the User Interface elements that you can use to control and
monitor the state of the device. These correspond to PCB components on the
platform image.

**LED 1** shows the overall state of the device and its connectivity. The
following states are possible:

-   _Short Flash On (50 ms on/950 ms off)_ &mdash; The device is in the
    unprovisioned (unpaired) state and is waiting for a commissioning
    application to connect.

-   _Rapid Even Flashing (100 ms on/100 ms off)_ &mdash; The device is in the
    unprovisioned state and a commissioning application is connected through
    Bluetooth LE.

-   _Short Flash Off (950ms on/50ms off)_ &mdash; The device is fully
    provisioned, but does not yet have full Thread network or service
    connectivity.

-   _Solid On_ &mdash; The device is fully provisioned and has full Thread
    network and service connectivity.

**LED 2** simulates the light bulb and shows the state of the lighting. The
following states are possible:

-   _Solid On_ &mdash; The light bulb is on.

-   _Off_ &mdash; The light bulb is off.

**Button 1** can be used for the following purposes:

-   _Pressed for 6 s_ &mdash; Initiates the factory reset of the device.
    Releasing the button within the 6-second window cancels the factory reset
    procedure. **LEDs 1-4** blink in unison when the factory reset procedure is
    initiated.

-   _Pressed for less than 3 s_ &mdash; Initiates the OTA software update
    process. This feature is not currently supported.

**Button 2** &mdash; Pressing the button once changes the lighting state to the
opposite one.


 
