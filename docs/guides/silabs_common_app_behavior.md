# Standard Application Behavior Guide

## Introduction

This section discusses the application behavior that is common to all Silabs
sample apps. The source files that implement the common functionality can be
found in the [example platform](../../examples/platform/silabs/) directory.

-   [Introduction](#introduction)
-   [LCD Screens](#lcd-screens)
    -   [QR Code](#qr-code)
    -   [Application UI](#application-ui)
    -   [Status Screen](#status-screen)
-   [Buttons](#buttons)
    -   [Operation Button](#operation-button)
    -   [Application Button](#application-button)
-   [LEDs](#leds)
    -   [Status LED](#status-led)
    -   [Application LED](#application-led)

## LCD Screens

When using a development kit that supports the LCD, the application has three
distinct windows. You can cycle between the three windows by pressing `BTN0`.
When the application UI is updated while on another window, the LCD will
automatically switch to it.

### QR Code

[QR Code](https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT%3A6FCJ142C00KA0648G00)
is the default QR code that can be used to commission the device over BLE and
when using the Basic Commissioning Mode. See the Matter specification to
understand what is encoded in the QR code.

> **Note**: The Basic Commissioning Mode is not recommended since it is less
> secure than the Enhanced Commissioning Mode. See the Matter specification for
> more details.

### Application UI

Each sample has an application UI that helps visualize the sample's app state.
See the sample app documentation for more information.

### Status Screen

The status screen is used to visualize the state of the device.

> **Note:** The support of the status screen for ICDs is yet to be done.

The following list describes the information that is common for OpenThread and
Wi-Fi devices.

|   LCD UI    | Description                                                                    |
| :---------: | :----------------------------------------------------------------------------- |
|  # fabrics  | Indicates the number of commissioned fabrics on the device                     |
|  Connected  | Indicates if the device is connected to the OpenThread or Wi-Fi network        |
| Advertising | Indicates if the devices is currently advertising an open commissioning window |
|   Is ICD    | Indicates if the device is an Intermittently Connected device                  |

The following list describes the information that is unique to OpenThread
devices.

| LCD UI  | Description                                              |
| :-----: | :------------------------------------------------------- |
|  PANID  | Indicates the PANID of the configured openthread network |
| OT Type | Indicates the openthread device type (FTD / MTD)         |

> **Note:** The PANID information is not yet printed on the LCD.

The following list describes the information that is unique to Wi-Fi devices. |
LCD UI | Description | | :----: | :---------- | | SSID | SSID of the connected
Wi-Fi network |

> **Note:** The SSID information is not yet printed on the LCD.

## Buttons

All sample applications are designed to work with two buttons: the application
button and the operation button. Button 0, **BTN0**, is the operation button and
Button 1, **BTN1**, is the application button.

> **Note:** Sparkfun dev kit (BRD2704A) does not have any buttons.

### Operation Button

The following list describes all the actions that can be executed with the
operation button.

<table>
    <tr>
        <th>Execution</th>
        <th>Description</th>
    </tr>
    <tr>
        <td>Press and Release</td>
        <td>
            <ul>
                <li>If the device is not already commissioned, it will start advertising in fast mode for 30 seconds. <br/>After 30 seconds, the device will then switch to a slower interval advertisement After 15 minutes, the advertisement stops.</li>
                <li>Prints initial and BCM commissioning QR code in the Logs</li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>Press and hold for 6 seconds</td>
        <td>
            <ul>
                <li>Factory Reset device</li>
            </ul>
        </td>
    </tr>
</table>

### Application Button

See the sample app documentation for more information on the application button.

## LEDs

All sample applications are designed to work with two LEDs: the application LED
and the status LED. **LED0** is the status LED and the **LED1** is the
application LED.

> **Note:** Some dev kits can only support the buttons or the LEDs. The button
> support is the default configuration. <br/> For dev kits with only LED, the
> application LED is the default configuration.

### Status LED

The following list describes all the states of the status LED.

|                   State                    | Description                                                                                                    |
| :----------------------------------------: | :------------------------------------------------------------------------------------------------------------- |
|    Short Flash On (50ms on / 950ms off)    | The device is in the unprovisioned (unpaired) state and is waiting for a commissioning application to connect. |
| Rapid Even Flashing (100ms on / 100ms off) | The device is in the unprovisioned state and a commissioning application is connected through Bluetooth LE.    |
|   Short Flash Off (950ms on / 50ms off)    | The device is fully provisioned, but does not yet have full Thread network or service connectivity.            |
|                  Solid On                  | The device is fully provisioned and has full Thread network and service connectivity.                          |
| Long Even Flashing (500ms on / 500ms off)  | Factory Reset procedure has been started.                                                                      |

### Application LED

See the sample app documentation for more information on the application LED.
