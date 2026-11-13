# Matter STM32 LIT ICD Generic Switch Example over Thread

This example describes a Matter Long Idle Time Intermittently Connected Device
(LIT ICD) with a Generic Switch on the STM32WBA65I-DK1 board. The device joins a
Thread network as a Sleepy End Device and reports physical button actions using
the Matter Switch cluster.

This application does not include an OTA Requestor. MCUBoot, OEMiROT signing,
and Matter OTA image generation are not part of this build or provisioning flow.

## Build and Commissioning

Follow the
[STM32 getting started guide](../../../../docs/platforms/stm32/stm32_getting_started_guide.md)
to prepare the Matter build environment and the STM32WBA65I-DK1 board.

Build the LIT ICD application with:

```bash
./scripts/build/build_examples.py --target stm32-$stm32_BOARD-lit build
```

Set `stm32_BOARD` to `STM32WBA65I-DK1` when configuring the build. No Secure
Gateway veneer copy, OEMiROT signing, or OTA packaging is required for this
non-OTA application. Flash the resulting firmware using the board's standard
non-OEMiROT procedure.

After boot, the device advertises over BLE for commissioning when it has no
Matter fabric. Commission it to a Thread network with `chip-tool`:

```bash
./chip-tool pairing ble-thread <NODE_ID> hex:<OPERATIONAL_DATASET> 20202021 3840
```

Replace `<NODE_ID>` with the node ID to assign and `<OPERATIONAL_DATASET>` with
the Thread network's operational dataset in hexadecimal. The example setup PIN
and discriminator above must match the values configured in the firmware.

## Device Behavior

### LIT ICD

-   The Thread interface is configured as a Sleepy End Device.
-   ICD Management is enabled. The device supports the ICD client-management and
    stay-active interactions exposed by the Matter ICD Management cluster.
-   The application uses the configured ICD slow-poll interval of 300 seconds,
    fast-poll interval of 200 milliseconds, idle duration of 450 seconds, and
    active duration of 2 seconds.
-   On first boot, the device opens a BLE commissioning window. After
    commissioning, it reconnects to its saved Thread network and retains its
    fabric and network data in non-volatile storage.

The ICD Management cluster is separate from the Switch cluster: ICD behavior
controls how the device sleeps and becomes reachable, while Generic Switch
events report local button input to Matter clients.

### Generic Switch

The board joystick's left direction (B3 / JOY_LEFT) is the Generic Switch input.
It reports the following Matter Switch cluster events on endpoint 1:

| Physical action         | Matter Switch event |
| ----------------------- | ------------------- |
| Button down             | `InitialPress`      |
| Short press and release | `ShortRelease`      |
| Long press              | `LongPress`         |
| Long press and release  | `LongRelease`       |

On press, the current position is set to 1. On release, it returns to 0. The
press duration determines whether the release is reported as short or long.
These are Switch cluster events, not On/Off commands: a Matter client should
subscribe to the Switch events on endpoint 1 to observe button actions.

### Board Buttons and LCD

| Input           | Application action                                      |
| --------------- | ------------------------------------------------------- |
| B1 / JOY_DOWN   | Save application and fabric data to non-volatile memory |
| B2 / JOY_SELECT | Factory reset                                           |
| B3 / JOY_LEFT   | Generate Generic Switch press/release events            |

The LCD is disabled by default for this application. If a build enables LCD
support (`CFG_LCD_SUPPORTED == 1`), it shows BLE connection and Thread network
status: `BLE Connected` while a BLE connection is active, `Fabric Created` after
commissioning, and `Network Joined` after Thread connectivity has been
established. A failed commissioning attempt is shown as `Fabric Failed`.

## Matter Data Model

The LIT ICD data model is already provided by
[`generic-switch-app.zap`](../data_model/generic-switch-app.zap) and selected by
the `stm32-lit-icd-app` GN data-model target. The STM32WBA65I-DK1 application
target depends on this model, which includes ICD Management and the Generic
Switch device type with its Switch cluster.

ICD server support is enabled for this application with
`CHIP_CONFIG_ENABLE_ICD_SERVER = 1`; the device is also configured as a Sleepy
End Device. The ICD Management cluster and Generic Switch events are therefore
both part of the application, not pending data-model work.

## Quick Reference

| Item                       | Value                     |
| -------------------------- | ------------------------- |
| Board                      | STM32WBA65I-DK1           |
| Network                    | Thread                    |
| Thread role                | Sleepy End Device         |
| Application                | LIT ICD + Generic Switch  |
| OTA Requestor              | Disabled / not included   |
| Generic Switch input       | B3 / JOY_LEFT, endpoint 1 |
| STM32 LIT ICD build target | `stm32-$stm32_BOARD-lit`  |
