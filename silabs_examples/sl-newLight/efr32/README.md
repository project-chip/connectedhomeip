# Matter EFR32 Colored Lighting Application

A lighting app with color control and level control.

<hr>

-   [Matter EFR32 Colored Lighting Application](#matter-EFR32-colored-lighting-application)
    -   [Introduction](#introduction)
    -   [Running the application](#running-the-application)
        -   [Material](#material)
        -   [Compiling](#compiling)
            -   [Building and flashing the example](#building-and-flashing-the-example)
        -   [Monitoring the app](#monitoring-the-app)
        -   [Interacting with the app](#interacting-with-the-app)

<hr>

## Introduction

This is a lighting application implementing Level Control and Color Control. It
allows to control the color and the intensity of an RGB LED.

First time using a Matter and/or EFR32 application ? Check out [The Glorious
Guide for running the BASIC Lighting App][1] for detailed information about
Building and Flashing an EFR32 application.

## Running the application

In addition to the steps in [The Glorious Guide][1], here is some relevant
information to help you run Color-Light app.

### Material

To set up this application, you will need:

-   1 Thunderboard Sense 2 (BRD4166A).
-   1 OT-BR running on Raspberry Pi (including its own RCP).

_This application was tested to run on a ThunderBoard Sense 2. Since this
application uses RGB Leds, level control and color control are disabled on most
boards that are not BRD4166A._

### Compiling

Follow the steps below to compile successfully Color-Light app.

Note: You will need GSDK with version 4.1.1 or later to run this application.

#### Building and flashing the example

**Important:** If you have followed correctly the steps in [The Glorious
Guide][1], you should now have your own way of building and flashing a Matter
EFR32 app. However, for reference, I put here the instructions that I execute in
the terminal to build Color-Light app, whenever I start from a _new, freshly
opened_ terminal.

1. After opening a new terminal, execute the following commands to build and
   flash Color-Light app on MacBook:

    ```Bash
     # Note: Starting in the root directory (i.e: "connectedhomeip/").
     # One time instructions:
     export PATH="$PATH:/Applications/Commander.app/Contents/MacOS/"
     export EFR32_BOARD=BRD4166A
     source scripts/activate.sh
     cd silabs_examples/sl-newLight/efr32

     # Excecute the following instructions everytime you need to build.
     rm -rf out
     gn gen out/debug
     ninja -C out/debug

     # Execute this instruction to flash.
     python3 out/debug/chip-efr32-lighting-example.flash.py
    ```

### Monitoring the app

You can monitor the EFR32 running the Color-Light app by using JLinkRTTViewer.
For more information, refer to [The Glorious Guide][1].

### Interacting with the app

In order to send commands to the Color-Light app, you will need an OTBR
(OpenThread Border Router). Note that the following commands work only if you
are using "Silabs Matter Raspberry Pi Image" as an OTBR.

2. Once your OTBR is set up, run the following commands to establish
   communication with the device running Color-Light and to send commands to it:

    ```Bash
    # These commands are executed once, everytime you power on your Raspberry Pi.
    mattertool startThread
    mattertool bleThread

    # Commands to turn On/Off the light.
    mattertool on
    mattertool off

    # You need to know the Node ID, do it by executing the following command:
    mattertool -h
    # The node ID will now be displayed, take note of it.

    # Command to set the level of the light.
    mattertool levelcontrol move-to-level {desired_level} 0 1 1 {node_ID} 1

    # Command to set the color of the light.
    # First Set the Saturation.
    mattertool colorcontrol move-to-saturation {desired_saturation} 0 1 1 {node_ID} 1
    # Now Set the Hue.
    mattertool colorcontrol move-to-hue {desired_hue} 0 0 1 1 {node_ID} 1
    ```

If you don't know what values of Hue and Saturation you need, look-up "HSV to
RGB color conversion" online. Note that "Value" is **always** 100%.

For more information about levelcontrol and colorcontrol commands, please refer
to "appclusters.pdf" Section 1.6. and 3.2., respectively.

###### Last modified August 1st, 2022.

[1]: ../../../examples/lighting-app/efr32/README.md
