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
    ├── DeviceDisplay.cpp           # BSP bring-up, theme, auto-sleep, and wake-on-touch
    ├── NavigationStack.h/.cpp      # Push/pop screen navigation and breadcrumbs
    └── screens/                    # Stateless screen render callbacks
        ├── HomeScreen.h/.cpp             # Root menu (Devices, Select Device, System)
        ├── SystemMenuScreen.h/.cpp       # Submenu (QR Code, Status, Operations)
        ├── CommissioningCodesScreen.h/.cpp # Matter onboarding QR code and manual setup code
        ├── DeviceInfoScreen.h/.cpp       # Diagnostics (fabrics, IP, memory, active device)
        ├── DeviceSelectionScreen.h/.cpp  # Device type switching with confirmation modal
        ├── DeviceEndpointsScreen.h/.cpp  # Bridged endpoints overview
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
                  │  - Bind LVGL dark theme       │
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
-   **Top Bar (38px)**:
    -   Ancestor levels: `lv_button` pills with `lv_obj_set_ext_click_area(6)`.
        Tapping an ancestor pops directly to that level via
        `NavigationStack::PopTo(level)`.
    -   Leaf level: Static text label showing the active view title.
    -   Container: `sCrumbContainer` fills the top bar with horizontal scrolling
        enabled (`LV_DIR_HOR`) without a scrollbar.
-   **Content Area**:
    -   Takes remaining vertical height (202px).
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

Device-specific interactive screens (e.g. On/Off toggles, level sliders) can be
linked from `DeviceEndpointsScreen` or `HomeScreen`:

```cpp
void ShowOnOffControlScreen(lv_obj_t * parent)
{
    lv_obj_t * toggleBtn = lv_button_create(parent);
    // Wire toggle button to Matter OnOff cluster commands...
}
```

### Adding a New Target Board

1. Create directory `display/<target>/`.
2. Implement `InitDeviceDisplay()` and `ShowRestartingMessage()` in
   `display/<target>/DeviceDisplay.cpp`.
3. Add a Kconfig option in `main/Kconfig.projbuild`.
4. Update `main/CMakeLists.txt` to conditionally compile `display/<target>/`
   when that Kconfig option is set.
