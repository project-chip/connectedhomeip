# OpenWeave nRF52840 Lock Example Application

An example application showing the use OpenWeave on the Nordic nRF52840.

[OpenWeave](https://github.com/openweave/openweave-core/) is the open source release of Nest's Weave technology, an application layer framework that provides a secure, reliable communications backbone for Nest's products.

<hr>

* [Introduction](#intro)
* [Software Architecture](#software-architecture)
* [Example Application Design](#example-app-design)
* [Device UI](#device-ui)
* [Building](#building)
* [Initializing the nRF52840 DK](#initializing)
* [Flashing the Application](#flashing)
* [Viewing Logging Output](#view-logging)
* [Debugging with GDB](#debugging)

<hr>

<a name="intro"></a>

## Introduction

![nrf52840 DK](doc/images/nrf52840-dk.jpg)

The nRF52840 lock example application provides a working demonstration of a connected door lock device, built using OpenWeave, OpenThread and the Nordic
nRF5 SDK. The example supports remote access and control of a simulated door lock over a low-power, 802.15.4 Thread network.  It is capable of being paired into
an existing Weave network along with other Weave-enabled devices, and supports communication (via a Weave border router device) with the Nest service.    The
example targets the [Nordic nRF52840 DK](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK) development kit, but is readily 
adaptable to other nRF52840-based hardware. 

The lock example is intended to serve both as a means to explore the workings of OpenWeave and OpenThread, as well as a template for creating real
products based on the Nordic platform. 

The example application builds upon the [OpenWeave](https://github.com/openweave/openweave-core/) and [OpenThread](https://github.com/openthread/openthread)
projects, which are incorporated as submodules and built from source.  A top-level Makefile orchestrates the entire build process, including building OpenWeave,
OpenThread and select files from the nRF5 SDK.  The resultant image file  can be flashed directly onto the Nordic dev kit hardware.


<a name="software-architecture"></a>

## Software Architecture


![nrf52840 Lock Example Architecture](doc/images/nrf52840-lock-example-architecture.svg)

 The lock example is built on the Weave application layer framework provided by [openweave-core](https://github.com/openweave/openweave-core/) . 
 At the heart of this are the **Weave Core** components.  These components provide the essential functionality required to speak Weave.
 This includes code for encoding and decoding Weave messages, communicating Weave messages over various transports (TCP, UDP, BLE), tracking
 Weave conversations (exchanges) and negotiating secure communications.

The **Weave Profiles** sit atop the core components and provide support for specific types of Weave interactions.  Central among these is the Weave Data
Management profile (**WDM**), which provides a generalized protocol for communicating state, configuration settings, commands and events between Weave
nodes.  Other profiles support device provisioning (pairing), OTA software update, time synchronization, and device identification and control.

The **Weave Device Layer** serves to adapt the portable Weave Core and Profile components to run in the context of a particular device platform.  In the case
of the lock example, the device platform is the nR52840.  Additionally, the Device Layer also provides platform-neutral
services (APIs) to the application for performing certain fundamental operations that are common to all Weave devices.  These include managing a device’s
persistent configuration, managing its network connectivity, scheduling and orchestrating OTA software updates and others.

The lock example makes use of various components provided by the **Nordic nRF52 SDK**, including BLE support libraries, persistent storage management,
crypto services, logging and others.  Nordic’s adaptation of **FreeRTOS** is used to support multi-threading and task synchronization.   **SEGGER RTT** support
provides access to log output using the J-Jink debug probe built in to the nRF52840 Dev Kit.  

**OpenThread** provides the core Thread stack implementation and manages persistent storage of Thread network configuration.  The **LwIP** Lightweight IP
stack provides IP services on top of Thread, including TCP, UDP and ICMPv6.

The **Nordic S140 SoftDevice** provides a BLE compatible protocol stack and well as multiprotocol radio support for simultaneous use of BLE and Thread.


<a name="example-app-design"></a>

## Example Application Design

The lock example application uses the Weave Data Management protocol (WDM) to enable remote access to state and control of a simulated door lock.  The
application implements the standard Nest-defined schema for a consumer-grade bolt lock.  In particular, the application publishes the `security.BoltLockTrait`, which
it uses to expose the overall state of the lock, and its bolt, to outside consumers.  In turn, the application consumes the properties of the `security.BoltLockSettingsTrait`
as published by the Nest service.  These properties contain user-supplied settings that configure the desired behavior of the device.

As part of implementing the BoltLockTrait, the application also responds to `BoltLockChangeRequest` commands instructing it to change the state of the bolt.  This
provides the ability to remotely lock and unlock the door.  To better simulate real lock hardware, the application incorporates a short delay mimicking the actuation
time of the bolt.

Whenever the state of the bolt changes, the application emits a `BoltActuatorStateChangeEvent`.  This event describes the end result of the change (bolt extended or
retracted), when the change happened and the actor or action that initiated it.  This is conveyed to the Nest service in a reliable manner.

Together, the features of the lock example have been designed to illustrate the four core interaction patterns typical of devices that use Weave and the
Weave Data Management protocol; namely:

- Publishing local state
- Subscribing to remote settings
- Reacting to received commands
- Emitting events


<a name="device-ui"></a>

## Device UI

The example application provides a simple UI that depicts the state of the device and offers basic user control.  This UI is implemented via the general-purpose
LEDs and buttons built in to the nRF52840 DK dev board.

**LED #1** shows the overall state of the device and its connectivity.  Four states are depicted:

- *Short Flash On (50ms on/950ms off)* &mdash; The device is in an unprovisioned (unpaired) state and is waiting for a commissioning application to connect.


- *Rapid Even Flashing (100ms on/100ms off)* &mdash; The device is in an unprovisioned state and a commissioning application is connected via BLE.


- *Short Flash Off (950ms on/50ms off)* &mdash; The device is full provisioned, but does not yet have full network (Thread) or service connectivity.


- *Solid On* &mdash; The device is fully provisioned and has full network and service connectivity.


**Button #1** can be used to initiate a OTA software update as well as to reset the device to a default state. 

A brief press of Button #1 instructs the device to perform a software update query to the Nest service.  Should the service indicate a software update is  available, the device
will download the corresponding software image file.  This feature is only available once the device completed the pairing process. While software update is running, another brief press on Button #1 will abort it.

Pressing and holding Button #1 for 6 seconds initiates a factory reset.  After an initial period of 3 seconds, all four LED will flash in unison to signal the pending reset.  Holding the button past 6 seconds
will cause the device to reset its persistent configuration and initiate a reboot.  The reset action can be cancelled by releasing the button at any point before the 6 second limit.

**LED #2** shows the state of the simulated lock bolt.  When the LED is lit the bolt is extended (i.e. door locked); when not lit, the bolt is retracted (door unlocked).  The LED will flash whenever the simulated bolt is in motion from one position to another.

**Button #2** can be used to change the state of the simulated bolt.  This can be used to mimick a user manually operating the lock.  The button behaves as a toggle, swapping the state every time it is pressed.

The remaining two LEDs and buttons (#3 and #4) are unused.


<a name="building"></a>

## Building

* Download and install the [Nordic nRF5 SDK for Thread and Zigbee version 3.1.0](https://www.nordicsemi.com/Software-and-Tools/Software/nRF5-SDK-for-Thread-and-Zigbee)
([Direct download link](https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5-SDK-for-Thread/nRF5-SDK-for-Thread-and-Zigbee/nRF5SDKforThreadandZigbeev310c7c4730.zip)) 


* Download and install the [Nordic nRF5x Command Line Tools](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools/Download)
(Direct download link: 
[Linux](https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/nRFCommandLineTools1030Linuxamd64tar.gz) 
[Mac OS X](https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/nRF-Command-Line-Tools_10_3_0_OSX.tar))


* Download and install a suitable ARM gcc tool chain: [GNU Arm Embedded Toolchain 7-2018-q2-update](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
(Direct download link: 
[Linux](https://developer.arm.com/-/media/Files/downloads/gnu-rm/7-2018q2/gcc-arm-none-eabi-7-2018-q2-update-linux.tar.bz2) 
[Mac OS X](https://developer.arm.com/-/media/Files/downloads/gnu-rm/7-2018q2/gcc-arm-none-eabi-7-2018-q2-update-mac.tar.bz2)) 


* Install some additional tools:

         # Linux
         $ sudo apt-get install git make automake libtool ccache
         
         # Mac OS X
         $ brew install automake libtool ccache

* Set the following environment variables based on the locations/versions of the packages installed above:

        export NRF5_SDK_ROOT=${HOME}/tools/nRF5_SDK_for_Thread_and_Zigbee_v3.1.0
        export NRF5_TOOLS_ROOT=${HOME}/tools/nRF-Command-Line-Tools
        export GNU_INSTALL_ROOT=${HOME}/tools/gcc-arm-none-eabi-7-2018-q2-update/bin/
        export PATH=${PATH}:${NRF5_TOOLS_ROOT}/nrfjprog

<p style="margin-left: 40px">For convenience, place these settings in local script file (e.g. setup-env.sh) so that they can be loaded into the environment as needed (e.g. by running 'source ./setup-env.sh').</p> 

* Clone the example application into a local directory

        $ cd ~ 
        $ git clone https://github.com/openweave/openweave-nrf52840-lock-example.git


* Initialize the dependent submodules

        $ cd ~/openweave-nrf52840-lock-example
        $ git submodule update --init 


* Run make to build the application

        $ cd ~/openweave-nrf52840-lock-example
        $ make clean
        $ make


<a name="initializing"></a>

## Initializing the nRF52840 DK

The example application is designed to run on the [Nordic nRF52840 DK](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK) development kit.  Prior to installing
the application, the device's flash memory should be erased and the Nordic SoftDevice image installed.

* Connect the host machine to the J-Link Interface MCU USB connector on the nRF52840 DK.  The Interface MCU connector is the one on the *short* side of the board.


* Use the Makefile to erase the flash and program the Nordic SoftDevice image.

        $ cd ~/openweave-nrf52840-lock-example
        $ make erase
        $ make flash-softdevice

Once the above is complete, it shouldn't need be done again *unless* the SoftDevice image or the Nordic configuration storage (fds) area becomes corrupt.  To correct either of these problems
erase the device and reflash the SoftDevice and application again.


<a name="flashing"></a>

## Flashing the Application

To flash the example app, run the following commands:

        $ cd ~/openweave-nrf52840-lock-example
        $ make flash-app


<a name="view-logging"></a>

## Viewing Logging Output

The example application is built to use the SEGGER Real Time Transfer (RTT) facility for log output.  RTT is a feature built-in to the J-Link Interface MCU on the development kit board.
It allows bi-directional communication with an embedded application without the need for a dedicated UART.

Using the RTT facility requires downloading and installing the *SEGGER J-Link Software and Documentation Pack* ([web site](https://www.segger.com/downloads/jlink#J-LinkSoftwareAndDocumentationPack)).

* Download the J-Link installer by navigating to the appropriate URL and agreeing to the license agreement. 

<p style="margin-left: 40px">Linux: <a href="https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.deb">JLink_Linux_x86_64.deb</a></p>
<p style="margin-left: 40px">MacOS: <a href="https://www.segger.com/downloads/jlink/JLink\_MacOSX.pkg">JLink_MacOSX.pkg</a></p>

* Install the J-Link software

        $ cd ~/Downloads
        $ sudo dpkg -i JLink_Linux_V*_x86_64.deb


* In Linux, grant the logged in user the ability to talk to the development hardware via the linux tty device (/dev/ttyACMx) by adding them to the dialout group.

        $ sudo usermod -a -G dialout ${USER} 

Once the above is complete, log output can be viewed using the JLinkExe tool in combination with JLinkRTTClient as follows:

* Run the JLinkExe tool with arguments to autoconnect to the nRF82480 DK board:

        $ JLinkExe -device NRF52840_XXAA -if SWD -speed 4000 -autoconnect 1

* In a second terminal, run the JLinkRTTClient:

        $ JLinkRTTClient

Logging output will appear in the second terminal.

An alternate method for viewing log output is to use the J-Link GDB server described in the following section. 


<a name="debugging"></a>

## Debugging with GDB

SEGGER provides a GDB server tool which can be used to debug a running application on the nRF82480 DK board.
The GDB server also provides a local telnet port which can be used to communicate with the device over RTT.  This can be used
to view logging output produced during a debugging session.

Note that you do not need to actually run gdb to see the logging output from the GDB server.  This means that the GDB server can also be
used as a convenient way to monitor the application activity in between debugging sessions.

#### Starting the GDB server

The example application includes a shell script, `start-jlink-gdb-server.sh`, that makes it easy to start  the J-Link GDB server:

        $ cd ~/openweave-nrf52840-lock-example
        $ ./start-jlink-gdb-server.sh

The `start-jlink-gdb-server.sh` script starts the SEGGER GDB server and then initiates a telnet session to the server's RTT output port.  Log output from the application is
then intermingled with  output from the GDB server.

#### Stopping the GDB server

To shutdown the GDB server, type CTRL-C.

#### Building the Example Application to Support Debugging

Because of the way the Nordic SoftDevice works, actual debugging of a running nRF82480 application requires the use of the
[SEGGER J-Link monitor mode](https://www.segger.com/products/debug-probes/j-link/technology/monitor-mode-debugging/) debugging feature.  This feature
works in conjunction with the J-Link GDB server to allow timing-critical parts of the system to continue running while the target device is ‘stopped’ at a breakpoint. Use
of monitor mode debugging is essential to avoid the Nordic SoftDevice crashing with a fatal error whenever the target is resumed from a breakpoint.

J-Link monitor mode requires the use a [J-Link Plus](https://www.segger.com/products/debug-probes/j-link/models/j-link-plus/) (or better) debug probe.
__Unfortunately, the on-board J-Link debugger included with the nRF52840 DK board cannot be used for GDB debugging.__   Additionally, the J-Link monitor
mode feature requires the application to be built with special code supplied by SEGGER.  The licensing of this code requires that it be downloaded separately
from the [SEGGER site](https://www.segger.com/products/debug-probes/j-link/technology/monitor-mode-debugging/).

To build the example application for use with GDB, perform the following steps:

* Download and unpack the J-Link monitor mode sample project:

        $ cd ~/openweave-nrf52840-lock-example
        $ wget https://www.segger.com/fileadmin/images/products/J-Link/Technology/Monitor_Mode_Debugging/Generic_Cortex-M_MonitorModeSystickExample_SES.zip
        $ unzip Generic_Cortex-M_MonitorModeSystickExample_SES.zip

* Copy or link the debug mode implementation file into the root directory of the example application:

        $ cd ~/openweave-nrf52840-lock-example
        $ ln -s ./Generic_Cortex-M_MonitorModeSystickExample_SES/Src/JLINK_MONITOR_ISR_SES.s .

* Rebuild the example application with monitor mode debugging support (MMD)  enabled:

        $ cd ~/openweave-nrf52840-lock-example
        $ make clean
        $ make MMD=1


#### Initiating a debugging session

Once the application has been built with monitor mode debugging support (MMD)  enabled, a debugging session can be initiated using 
an appropriate J-Link debug probe:

* Connect the J-Link debug probe to the 10-pin "Debug In" connector located near the center of the nRF52840 DK board.


* Slide the "nRF Power Source" switch to the "USB" position.


* Power the device by connecting USB power to the board's "nRF USB" connector (the connector on the long edge of the board).


* Start the GDB server using the `start-jlink-gdb-server.sh` script:

        $ cd ~/openweave-nrf52840-lock-example
        $ ./start-jlink-gdb-server.sh


* In a separate windows, start GDB using the `start-gdb.sh` script:

        $ cd ~/openweave-nrf52840-lock-example
        $ ./start-gdb.sh


The `start-gdb.sh` script will automatically launch gdb and instruct it to connect to the GDB server.  It will also issue the necessary commands to enable
monitor mode debugging.

By default, gdb is started with the example executable located in the build directory (`./build/openweave-nrf52840-lock-example.out`).  Alternatively one can pass
the name of a different executable as an argument to the `start-gdb.sh` script.

<a name="setup-test-network"></a>
