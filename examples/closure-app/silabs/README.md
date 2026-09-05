# Matter SiWx917 Closure Example

An example showing the use of CHIP on the Silicon Labs SiWx917

<hr>

-   [Matter SiWx917 Closure Example](#matter-siwx917-closure-example)
    -   [Introduction](#introduction)
    -   [Extending Base App Implementation](#extending-base-app-implementation)
        -   [CustomerAppManager](#customerappmanager)
        -   [How to Override APIs](#how-to-override-apis)
        -   [DataModelCallbacks and CustomerAppTask](#datamodelcallbacks-and-customerapptask)
        -   [Sample Implementation](#sample-implementation)
        -   [Override API Reference](#override-api-reference)
    -   [Building](#building)
    -   [Flashing the Application](#flashing-the-application)
    -   [Running the Complete Example](#running-the-complete-example)
    -   [Group Communication (Multicast)](#group-communication-multicast)
    -   [Building options](#building-options)
        -   [Disabling logging](#disabling-logging)
        -   [Debug build / release build](#debug-build--release-build)
        -   [Disabling QR CODE](#disabling-qr-code)
        -   [KVS maximum entry count](#kvs-maximum-entry-count)

<hr>

> **NOTE:** Silicon Laboratories now maintains a public matter GitHub repo with
> frequent releases thoroughly tested and validated. Developers looking to
> develop matter products with silabs hardware are encouraged to use our latest
> release with added tools and documentation.
> [Silabs matter_sdk Github](https://github.com/SiliconLabsSoftware/matter_sdk/tags)

## Introduction

The SiWx917 Closure example provides a baseline demonstration of a closure base
device type, built using Matter and the Silicon Labs gecko SDK. It can be
controlled by a Chip controller over Wifi network.

The SiWx917 device can be commissioned over Bluetooth Low Energy where the
device and the Chip controller will exchange security information with the
Rendez-vous procedure.

The closure example is intended to serve both as a means to explore the workings
of Matter as well as a template for creating real products based on the Silicon
Labs platform.

Unlike the lighting example (single CRTP chain on `AppTask`), this app keeps a
separate `ClosureManager` for closure domain logic and a second CRTP chain for
product customization. See
[Extending Base App Implementation](../../../docs/platforms/silabs/silabs_extending_base_app_implementation.md)
for `CustomerAppTask` basics and
[Extending Base App Implementation](#extending-base-app-implementation) below
for closure-specific `CustomerAppManager` customization.

## Extending Base App Implementation

See
[Extending Base App Implementation](../../../docs/platforms/silabs/silabs_extending_base_app_implementation.md)
for `CustomerAppTask` customization, CRTP `*Impl()` hooks, and data model
callback routing. Per-example AppTask API references:
[`include/AppTaskImpl.h`](include/AppTaskImpl.h),
[`src/AppTask.cpp`](src/AppTask.cpp).

| Concern                                      | Base             | CRTP hook layer                                      | Customer leaf                                                         |
| -------------------------------------------- | ---------------- | ---------------------------------------------------- | --------------------------------------------------------------------- |
| Lifecycle / UI / buttons / DM callbacks      | `AppTask`        | [`AppTaskImpl`](include/AppTaskImpl.h)               | [`CustomerAppTask`](../../platform/silabs/customer/CustomerAppTask.h) |
| Closure domain logic (motion, latch, panels) | `ClosureManager` | [`ClosureManagerImpl`](include/ClosureManagerImpl.h) | [`CustomerAppManager`](include/customer/CustomerAppManager.h)         |

### CustomerAppManager

Closure domain logic lives on `ClosureManager`. The customer leaf is already in
this app under
[`include/customer/CustomerAppManager.h`](include/customer/CustomerAppManager.h)
and
[`src/customer/CustomerAppManager.cpp`](src/customer/CustomerAppManager.cpp).
Override `*Impl()` hooks there; do not edit `ClosureManager.cpp` for
app-specific behavior. See
[`ClosureManagerImpl.h`](include/ClosureManagerImpl.h) for the full hook list.

### How to Override APIs

Both leaves use the Curiously Recurring Template Pattern (CRTP). You override
only the `*Impl()` methods you need; each base declares one `*Impl()` per
overridable API. Steps:

1. Find the method to override in the base API (see
   [Override API reference](#override-api-reference) below).
2. Declare the same method signature in `CustomerAppTask.h` or
   `CustomerAppManager.h` under `private:`. Match the base `*Impl()` signature
   exactly — note that `*Impl()` overrides are **non-static instance methods**
   even when the public dispatcher (e.g. `ButtonEventHandler`) is `static`.
3. Implement the method in the corresponding `.cpp`.
4. Build. The CRTP layer automatically routes each call to your `*Impl()` if
   present, otherwise to the Silicon Labs default.

Some `ClosureManager` APIs (`Init`, cluster `On*Command` handlers) are virtual
so shared `closure-common/` code can reach your leaf. Override the matching
`*Impl()` on `CustomerAppManager` — do not override the virtual methods
directly.

### DataModelCallbacks and CustomerAppManager

In addition to the general data model callback routing described in
[Extending Base App Implementation](../../../docs/platforms/silabs/silabs_extending_base_app_implementation.md#datamodelcallbacks-and-customerapptask),
this app forwards closure-specific cluster attribute-changed callbacks
(`MatterClosureControlClusterServerAttributeChangedCallback`,
`MatterClosureDimensionClusterServerAttributeChangedCallback`) from
`AppTask.cpp` to `DMClosureControlClusterAttributeChangedCallback` /
`DMClosureDimensionClusterAttributeChangedCallback`, customized via the matching
`*Impl()` hooks on `CustomerAppTask`.

Forwarding into `AppTask` still goes through CRTP as in
[How to Override APIs](#how-to-override-apis).

-   **Methods that already exist in the ClosureManager** — Customize them by
    overriding the matching `*Impl()` method in `CustomerAppManager`. Do not
    edit `ClosureManager.cpp` for app-specific behavior.

-   **New custom data model methods** — Add them in `CustomerAppTask` or
    `CustomerAppManager` directly. Do not add new application logic in
    autogenerated sources; those edits will not survive regeneration or project
    upgrades.

### Sample Implementation

The following shows a minimal example that overrides `AppInitImpl()` and
`ButtonEventHandlerImpl()` on `CustomerAppTask`, and adds
`OnMoveToCommandImpl()` to the existing `CustomerAppManager` leaf. Declare the
manager hook under `private:` without replacing `GetInstance` / `sInstance`.

**CustomerAppTask.h**

```cpp
#pragma once
#include "AppTaskImpl.h"

/**
 * Minimal AppTaskImpl-derived class. Override only the *Impl() methods you need;
 * add AppInitImpl(), GetAppTask(), and sAppTask as required by the CRTP base.
 */
class CustomerAppTask : public AppTaskImpl<CustomerAppTask>
{
public:
    static CustomerAppTask & GetAppTask() { return sAppTask; }

private:
    friend class AppTaskImpl<CustomerAppTask>;
    CHIP_ERROR AppInitImpl();
    void ButtonEventHandlerImpl(uint8_t button, uint8_t btnAction);
    static CustomerAppTask sAppTask;
};
```

**CustomerAppTask.cpp**

```cpp
#include "CustomerAppTask.h"
#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include <platform/CHIPDeviceLayer.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

using namespace ::chip::DeviceLayer::Silabs;

#define APP_FUNCTION_BUTTON 0
#define APP_CLOSURE_BUTTON  1

CustomerAppTask CustomerAppTask::sAppTask;

AppTask & AppTask::GetAppTask()
{
    return CustomerAppTask::GetAppTask();
}

CHIP_ERROR CustomerAppTask::AppInitImpl()
{
    SILABS_LOG("CustomerAppTask: custom implementation (AppInitImpl)");
    CHIP_ERROR err = this->AppTask::AppInit();
    if (err == CHIP_NO_ERROR)
    {
        // Override the SDK default button handler registered in AppTask::AppInit().
        chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(CustomerAppTask::ButtonEventHandler);
    }
    return err;
}

void CustomerAppTask::ButtonEventHandlerImpl(uint8_t button, uint8_t btnAction)
{
    SILABS_LOG("CustomerAppTask: custom implementation (ButtonEventHandlerImpl)");
    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = btnAction;
    if (button == APP_CLOSURE_BUTTON && btnAction == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        button_event.Handler = &CustomerAppTask::ClosureButtonActionEventHandler;
        AppTask::GetAppTask().PostEvent(&button_event);
    }
    else if (button == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = BaseApplication::ButtonHandler;
        AppTask::GetAppTask().PostEvent(&button_event);
    }
}
```

**CustomerAppManager.h**

```cpp
chip::Protocols::InteractionModel::Status OnMoveToCommandImpl(
    const chip::Optional<chip::app::Clusters::ClosureControl::TargetPositionEnum> position,
    const chip::Optional<bool> latch,
    const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> speed);
```

**CustomerAppManager.cpp**

```cpp
chip::Protocols::InteractionModel::Status CustomerAppManager::OnMoveToCommandImpl(
    const chip::Optional<chip::app::Clusters::ClosureControl::TargetPositionEnum> position,
    const chip::Optional<bool> latch,
    const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> speed)
{
    SILABS_LOG("CustomerAppManager: custom implementation (OnMoveToCommandImpl)");
    // Call through for default motion behavior, or drive real hardware here.
    return ClosureManager::OnMoveToCommand(position, latch, speed);
}
```

### Override API Reference

This example has a ClosureManager CRTP chain in addition to the AppTask chain
documented in
[Extending Base App Implementation](../../../docs/platforms/silabs/silabs_extending_base_app_implementation.md#override-api-reference):

| File                                                           | Purpose                                                                                                                                                     |
| -------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [`include/ClosureManagerImpl.h`](include/ClosureManagerImpl.h) | Declarations of every overridable `*Impl()` method. Copy the signatures you need from here into `CustomerAppManager.h`.                                     |
| [`src/ClosureManager.cpp`](src/ClosureManager.cpp)             | Silicon Labs default implementation of ClosureManager. This is what runs for any `*Impl()` you do not override. Use as reference when customizing behavior. |

## Building

-   Download the
    [Simplicity Commander](https://www.silabs.com/mcu/programming-options)
    command line tool, and ensure that `commander` is your shell search path.
    (For Mac OS X, `commander` is located inside
    `Commander.app/Contents/MacOS/`.)

-   Download and install a suitable ARM gcc tool chain:
    [GNU Arm Embedded Toolchain 9-2019-q4-major](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

-   Install some additional tools (likely already present for CHIP developers):

    -   Linux: `sudo apt-get install git ninja-build`

    -   Mac OS X: `brew install ninja`

-   Supported hardware:

    -   > For the latest supported hardware please refer to the
        > [Hardware Requirements](https://docs.silabs.com/matter/latest/matter-prerequisites/hardware-requirements)
        > in the Silicon Labs Matter Documentation

    917SoC boards :

    -   BRD4338A

*   Build the example application:

            cd ~/connectedhomeip
            ./scripts/examples/gn_silabs_example.sh ./examples/closure-app/silabs/ ./out/closure-app BRD4338A

-   To delete generated executable, libraries and object files use:

            $ cd ~/connectedhomeip
            $ rm -rf ./out/

    OR use GN/Ninja directly

            $ cd ~/connectedhomeip/examples/closure-app/silabs
            $ git submodule update --init
            $ source third_party/connectedhomeip/scripts/activate.sh
            $ export SILABS_BOARD=BRD4338A

    To build the Closure example

            $ gn gen out/debug
            $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

            $ cd ~/connectedhomeip/examples/closure-app/silabs
            $ rm -rf out/

For more build options, help is provided when running the build script without
arguments

         ./scripts/examples/gn_silabs_example.sh

## Flashing the Application

-   SiWx917 SoC device support is available in the latest Simplicity Commander.
    The SiWx917 SOC board will support .rps as the only file to flash.

-   Or with the Ozone debugger, just load the .out file.

All SiWx917 boards require a connectivity firmware, see Silicon Labs
documentation for more info. Pre-built firmware binaries are available on the
[Matter Software Artifacts page](https://docs.silabs.com/matter/latest/matter-prerequisites/matter-artifacts#siwx917-firmware-for-siwn917-ncp-and-siwg917-soc).

## Running the Complete Example

-   To run a Matter over Wi-Fi application, you must first create a Matter
    network using the chip-tool, and then control the Matter device from the
    chip-tool.

**Creating the Matter Network**

     This procedure uses the chip-tool installed on the Matter Hub. The commissioning procedure does the following:
     - Chip-tool scans BLE and locates the Silicon Labs device that uses the specified discriminator.
     - Establishes operational certificates.
     - Sends the Wi-Fi SSID and Passkey.
     - The Silicon Labs device will join the Wi-Fi network and get an IP address. It then starts providing mDNS records on IPv4 and IPv6.
     - Future communications (tests) will then happen over Wi-Fi.

-   You can provision and control the Chip device using the python controller,
    Chip tool standalone, Android or iOS app

    [CHIPTool](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md)

    Here is an example with the chip-tool:

            $ chip-tool pairing ble-wifi 1 <SSID> <Password> 20202021 3840

*   User interface :

    **Push Button 0**

          -   _Press and Release_ : Start, or restart, BLE advertisement in fast mode. It will advertise in this mode
              for 30 seconds. The device will then switch to a slower interval advertisement.
              After 15 minutes, the advertisement stops.
              Additionally, it will cycle through the QR code, application status screen and device status screen, respectively.

          -   _Pressed and hold for 6 s_ : Initiates the factory reset of the device.
              Releasing the button within the 6-second window cancels the factory reset
              procedure. **LEDs** blink in unison when the factory reset procedure is
              initiated.

## Group Communication (Multicast)

With this Closure example you can also use group communication to send Closure
commands to multiples devices at once. Please refer to the
[chip-tool documentation](../../chip-tool/README.md) _Configuring the server
side for Group Commands_ and _Using the Client to Send Group (Multicast) Matter
Commands_

## Building options

All of Silabs's examples within the Matter repo have all the features enabled by
default, as to provide the best end user experience. However some of those
features can easily be toggled on or off. Here is a short list of options to be
passed to the build scripts.

### Disabling logging

`chip_progress_logging, chip_detail_logging, chip_automation_logging`

    $ ./scripts/examples/gn_silabs_example.sh ./examples/closure-app/silabs ./out/closure-app BRD4338A "chip_detail_logging=false chip_automation_logging=false chip_progress_logging=false"

### Debug build / release build

`is_debug`

    $ ./scripts/examples/gn_silabs_example.sh ./examples/closure-app/silabs ./out/closure-app BRD4338A "is_debug=false"

### Disabling QR CODE

show_qr_code

    $ ./scripts/examples/gn_silabs_example.sh ./examples/closure-app/silabs ./out/closure-app BRD4338A "show_qr_code=false"

### KVS maximum entry count

`kvs_max_entries`

    Set the maximum Kvs entries that can be stored in NVM (Default 255)
    Thresholds: 30 <= kvs_max_entries <= 511

    $ ./scripts/examples/gn_silabs_example.sh ./examples/closure-app/silabs ./out/closure-app BRD4338A kvs_max_entries=355
