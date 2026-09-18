# Display Subsystem

This directory contains the display implementations for `all-devices-app` on
ESP32 targets.

---

## Integration Boundary

`main.cpp` interacts with the display subsystem through two functions declared
in [`DeviceDisplay.h`](DeviceDisplay.h):

```cpp
void InitDeviceDisplay();
void ShowRestartingMessage();
```

No graphics library or hardware driver headers are exposed outside the
`display/` directory.

```
┌────────────────────────────────────────────────────────┐
│             Application Logic (main.cpp)               │
└───────────────────────────┬────────────────────────────┘
                            │ Calls public interface only
                            ▼
┌────────────────────────────────────────────────────────┐
│                    DeviceDisplay.h                     │
│  - InitDeviceDisplay()                                 │
│  - ShowRestartingMessage()                             │
└──────────────┬───────────────────────────┬─────────────┘
               │ (ESP32)                   │ (ESP32-S3)
               ▼                           ▼
┌───────────────────────────────┐ ┌──────────────────────┐
│        tft/ Renderer          │ │    lvgl/ Renderer    │
│  - Vendored SPI TFT driver    │ │  - LVGL 9            │
│  - ScreenFramework (C++)      │ │  - CoreS3 BSP        │
│  - Mechanical buttons (A/B/C) │ │  - Capacitive touch  │
└───────────────────────────────┘ └──────────────────────┘
```

---

## Directory Layout

```
display/
├── DeviceDisplay.h                 # Public interface (InitDeviceDisplay, ShowRestartingMessage)
├── README.md                       # This document
│
├── tft/                            # Button-driven renderer for ESP32
│   ├── DeviceDisplay.cpp           # Display initialization and main task loop
│   ├── Button.h / Button.cpp       # Polled mechanical buttons (BtnA, BtnB, BtnC)
│   └── screens/                    # ScreenFramework subclasses (fixed tab views)
│       ├── DeviceInfoScreen.h/.cpp
│       ├── DeviceSelectionScreen.h/.cpp
│       └── QRCodeScreen.h/.cpp
│
└── lvgl/                           # Touch-driven renderer for ESP32-S3 (M5Stack CoreS3)
    ├── DeviceDisplay.cpp           # Display bring-up, lifecycle, and sleep
    ├── DisplayNotificationHub.h/.cpp # Bridges Matter DataModel changes to LVGL widgets
    ├── NavigationStack.h/.cpp      # Push/pop screen navigation and breadcrumbs
    ├── DeviceScreenRegistry.h/.cpp # Registry for dynamically hooked device screens
    ├── DeviceScreenHook.h          # DeviceFactory post-registration hook
    ├── DeviceScreenRegistration.h/.cpp # Device screen registration implementations
    ├── clusters/                   # Reusable cluster-specific widgets
    │   ├── OnOffClusterWidget.h/.cpp     # Power status and toggle control
    │   └── LevelControlClusterWidget.h/.cpp # Level slider and percentage control
    ├── devices/                    # Matter device type screens (composed from cluster widgets)
    │   ├── DeviceHeader.h/.cpp           # Standard device metadata header card
    │   ├── OnOffLightScreen.h/.cpp       # On/Off Light device screen
    │   └── DimmableLightScreen.h/.cpp    # Dimmable Light device screen
    └── screens/                    # Stateless screen render callbacks
        ├── HomeScreen.h/.cpp             # Root menu (Devices, Select Device, System)
        ├── SystemMenuScreen.h/.cpp       # Submenu (QR Code, Status, Operations)
        ├── CommissioningCodesScreen.h/.cpp # Matter onboarding QR code and manual setup code
        ├── DeviceInfoScreen.h/.cpp       # Diagnostics (fabrics, IP, memory, active device)
        ├── DeviceSelectionScreen.h/.cpp  # Device type switching with confirmation modal
        ├── DeviceEndpointsScreen.h/.cpp  # Dynamic list of registered device screens
        └── DeviceOperationsScreen.h/.cpp # Reboot and factory reset triggers with confirmation modals
```

---

## Build Configuration

Exactly one renderer is compiled per target, controlled by Kconfig options in
`main/CMakeLists.txt`:

| Renderer | Supported Targets              | Framework & Dependencies                         |
| :------- | :----------------------------- | :----------------------------------------------- |
| `tft/`   | M5Stack Basic/Gray, WROVER-KIT | `screen-framework` + SPI TFT driver (ESP32 only) |
| `lvgl/`  | M5Stack CoreS3                 | LVGL 9 + `espressif/m5stack_core_s3` BSP         |

The legacy `tft/` driver accesses ESP32 hardware registers directly and does not
compile for ESP32-S3. Targets using other chips must use `lvgl/` or add a
dedicated renderer.

`main/CMakeLists.txt` selectively includes the active renderer:

```cmake
if(CONFIG_DEVICE_TYPE_M5STACK_CORES3)
    list(APPEND PRIV_INCLUDE_DIRS_LIST
        "${CMAKE_CURRENT_LIST_DIR}/display/lvgl"
        "${CMAKE_CURRENT_LIST_DIR}/display/lvgl/screens"
    )
    # Append lvgl sources to SRCS...
endif()
```

---

## LVGL Renderer Runtime Flow

```
                  ┌───────────────────────────────┐
                  │      InitDeviceDisplay()      │
                  └──────────────┬────────────────┘
                                 │
                                 ▼
                  ┌───────────────────────────────┐
                  │      bsp_display_start()      │  (Initializes PMIC rails, SPI panel,
                  └──────────────┬────────────────┘   touch I2C, and esp_lvgl_port task)
                                 │
                                 ▼
                  ┌───────────────────────────────┐
                  │    bsp_display_lock(0)        │
                  │  - NavigationStack::Init()    │
                  │  - Push Home (and QR if new)  │
                  │  - Register Inactivity Timer  │
                  │    bsp_display_unlock()       │
                  └──────────────┬────────────────┘
                                 │
                                 ▼
                  ┌───────────────────────────────┐
                  │ bsp_display_backlight_on()    │  (Delayed until first frame drawn)
                  └──────────────┬────────────────┘
                                 │
                                 ▼
    ┌────────────────────► Active State ◄────────────────────┐
    │                            │                           │
    │ (Touch anywhere on glass)  │ (30 seconds of            │
    │                            │  inactivity)              │
    │                            ▼                           │
    │                    ┌───────────────┐                   │
    │                    │ Display Sleep │                   │
    │                    │ - Backlight 0 │                   │
    │                    │ - Wake Overlay│                   │
    │                    └───────┬───────┘                   │
    └────────────────────────────┘───────────────────────────┘
```

### Thread Safety

LVGL operations must run under the LVGL port mutex:

```cpp
if (bsp_display_lock(0))
{
    // LVGL operations...
    bsp_display_unlock();
}
```

Callbacks dispatched from within the LVGL task (such as widget event handlers or
timer callbacks) already execute with this lock held.

### Data Model Notifications & Cross-Thread Synchronization

UI widgets display dynamic device state that can be updated either locally via
touch or remotely via Matter protocol commands (over Wi-Fi/Thread).

#### 1. UI to Matter (Touch Interactions)

Local touch handlers must **never** synchronously block on
`chip::DeviceLayer::StackLock` while holding `bsp_display_lock` to avoid ABBA
deadlocks with the CHIP thread. Instead, user touch callbacks asynchronously
post cluster mutations using `DeviceLayer::SystemLayer().ScheduleLambda()`:

```cpp
lv_obj_add_event_cb(toggleBtn, [](lv_event_t * event) {
    auto * cluster = static_cast<Clusters::OnOffCluster *>(lv_event_get_user_data(event));
    DeviceLayer::SystemLayer().ScheduleLambda([cluster]() {
        cluster->SetOnOff(!cluster->GetOnOff());
    });
}, LV_EVENT_CLICKED, &cluster);
```

#### 2. Matter to UI (`DisplayNotificationHub`)

Dynamic updates from Matter protocol commands flow into
`DisplayNotificationHub`, which implements
`chip::app::DataModel::AttributeChangeListener`:

```
Matter Protocol Command (CHIP Thread)
       │
       ▼
DataModel::AttributeChangeListener::OnAttributeChanged(path)
       │
       ▼
bsp_display_lock(kWaitForever)
       │
       ▼
DisplayNotificationHub dispatches to matching widget callbacks
       │
       ▼
Widget lambda updates LVGL labels/sliders directly
       │
       ▼
bsp_display_unlock()
```

#### 3. Automatic Lifecycle & RAII Unregistration

To prevent dangling pointers when screens are popped from `NavigationStack`,
widgets subscribe using their parent container (`card`):

```cpp
DisplayNotificationHub::Instance().Subscribe(
    card, cluster.GetEndpointId(), Clusters::OnOff::Id,
    [stateLabel, toggleBtn, btnLabel, &cluster](const ConcreteAttributePath & path) {
        if (path.mAttributeId == Clusters::OnOff::Attributes::OnOff::Id) {
            UpdateOnOffDisplay(stateLabel, toggleBtn, btnLabel, cluster.GetOnOff());
        }
    });
```

`DisplayNotificationHub::Subscribe` automatically hooks the `LV_EVENT_DELETE`
event on `card`. When the screen or card widget is destroyed, the hub
immediately removes the subscription, ensuring no callback ever executes on a
freed widget. Because both unregistration and event dispatch execute under
`bsp_display_lock`, they are strictly serialized and cannot race.

### Auto-Sleep and Wake-on-Touch

1. An LVGL timer checks `lv_display_get_inactive_time()` every second.
2. After 30 seconds of inactivity, `bsp_display_backlight_off()` sets backlight
   brightness to 0%.
3. A transparent full-screen overlay is added to `lv_layer_top()`.
4. The first touch on the sleeping display triggers `LV_EVENT_PRESSED` on the
   overlay, which calls `bsp_display_backlight_on()`, deletes the overlay, and
   resets the inactivity timer. The wake touch is absorbed so underlying widgets
   are not activated.

---

## Hierarchical Navigation (`NavigationStack`)

The CoreS3 UI uses a push/pop stack model with clickable breadcrumb navigation.

### Structure

-   **Stack**: `std::vector<StackEntry>` where `StackEntry` contains
    `{ std::string title, RenderScreenFn renderFn }`.
-   **Top Bar**:
    -   Ancestor levels: `lv_button` pills. Tapping an ancestor pops directly to
        that level via `NavigationStack::PopTo(level)`.
    -   Leaf level: Static text label showing the active view title.
    -   Container: `sCrumbContainer` fills the top bar with horizontal scrolling
        enabled (`LV_DIR_HOR`) without a scrollbar.
-   **Content Area**:
    -   Fills the remaining vertical height.
    -   Cleared on transition and repopulated by calling
        `renderFn(sContentContainer)`.
    -   Configured with column flex layout and vertical scrolling
        (`LV_OBJ_FLAG_SCROLLABLE`).

---

## Adding New Components

### Adding a Screen to `lvgl/`

1. **Declare the entry point** in `display/lvgl/screens/<Name>Screen.h`:

    ```cpp
    #pragma once
    #include <lvgl.h>

    void Show<Name>Screen(lv_obj_t * parent);
    ```

2. **Implement widgets** in `display/lvgl/screens/<Name>Screen.cpp`:

    ```cpp
    #include "<Name>Screen.h"

    void Show<Name>Screen(lv_obj_t * parent)
    {
        lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_all(parent, 10, LV_PART_MAIN);

        lv_obj_t * label = lv_label_create(parent);
        lv_label_set_text(label, "Content");
    }
    ```

3. **Navigate from existing views**:

    ```cpp
    #include "<Name>Screen.h"
    #include "NavigationStack.h"

    static void OnButtonClicked(lv_event_t * event)
    {
        NavigationStack::Push("<Title>", Show<Name>Screen);
    }
    ```

4. **Register with build**: Add `display/lvgl/screens/<Name>Screen.cpp` to
   `main/CMakeLists.txt` under `CONFIG_DEVICE_TYPE_M5STACK_CORES3`.

### Adding Device Controls

Device-specific interactive screens (e.g. On/Off toggles, level sliders) should
be registered dynamically during device construction via `DeviceFactory` hooks:

1. Define a UI registration hook or callback invoked from
   `DeviceFactory::Create()`.
2. As each device interface is registered to the data model, register its
   endpoint control views with `NavigationStack` or a device screen registry.
3. Composed devices (such as refrigerator or oven) register screens for each of
   their composed endpoints.

### Adding a New Target Board

1. Create directory `display/<target>/`.
2. Implement `InitDeviceDisplay()` and `ShowRestartingMessage()` in
   `display/<target>/DeviceDisplay.cpp`.
3. Add a Kconfig option in `main/Kconfig.projbuild`.
4. Update `main/CMakeLists.txt` to conditionally compile `display/<target>/`
   when that Kconfig option is set.
