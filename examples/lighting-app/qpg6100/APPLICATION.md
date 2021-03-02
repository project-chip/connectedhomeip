# CHIP QPG6100 Lighting Example Application

An example application showing the use
[CHIP](https://github.com/project-chip/connectedhomeip) on the Qorvo QPG6100.

More detailed information to be included in
[SDK Documentation](../../platform/qpg6100/README.md).

## Lighting-app

The QPG6100 Lighting-app shows an implementation of a Lighting device, based on
CHIP\
and the Qorvo QPG6100 SDK. Intention of this example is to demonstrate a CHIP\
device, with Thread connectivity, using BLE to perform CHIP provisioning.

## Lighting-app button control

This application uses following buttons of the DK board:

-   SW1: Used to toggle the light between two levels
-   SW2: Used to perform a HW reset for the full board
-   SW4: Used to toggle the off/on state of the simulated light
-   SW5: Used to perform, depending on the time the button is kept pressed,
    -   Software update - not yet supported (released before 3s)
    -   Trigger Thread joining (release after 3s)
    -   Factory reset (released after 6s)

Unused:

-   SW3 - slider switch

## LED output

The following LEDs are used during the application:

-   LD2 - GRN led:
    -   Short blink every 1s: BLE advertising
    -   Fast blinks: BLE connected and subscribed
    -   Short off: Thread connected
    -   On: Full service connectivity
-   LD3 - WHT led:
    -   Off: Light is off
    -   Dim: Light is set to low level
    -   Bright: Light is set to high level

## Logging Output

-   See
    [View Logging Output](../../platform/qpg6100/README.md#view-logging-output)
-   At startup you will see:

```
qvCHIP v0.0.0.0 (CL:155586) running
[P][-] Init CHIP Stack
[P][DL] BLEManagerImpl::Init() complete
[P][-] Starting Platform Manager Event Loop
[P][-] ============================
[P][-] Qorvo Lighting-app Launching
[P][-] ============================
[D][DL] CHIP task running
```

-   Note! Logging is currently encapsulated by the Qorvo logging module. Output\
    will have additional header and footer bytes. This will be updated later\
    into a raw stream for any serial terminal to parse.
