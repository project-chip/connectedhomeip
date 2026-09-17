# Display Subsystem Architecture & Design

This directory implements the on-device display subsystem for `all-devices-app`
on ESP32 targets.

---

## 1. Architectural Boundary

The application interacts with the display subsystem exclusively through a
single minimal C-linkage header: [`DeviceDisplay.h`](DeviceDisplay.h).

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
│  - Vendored TFT Driver        │ │  - LVGL 9 Graphics   │
│  - ScreenFramework (C++)      │ │  - CoreS3 BSP        │
│  - Mechanical Buttons (A/B/C) │ │  - Capacitive Touch  │
└───────────────────────────────┘ └──────────────────────┘
```

`main.cpp` and Matter cluster server code have no dependencies on graphics
libraries, widget hierarchies, or hardware drivers. Exactly one renderer
implementation is compiled per build target.

---

## 2. Directory & Source Layout

```
display/
├── DeviceDisplay.h                 # Public display contract (InitDeviceDisplay, ShowRestartingMessage)
├── README.md                       # This design document
│
├── tft/                            # Button-driven renderer for original ESP32 targets
│   ├── DeviceDisplay.cpp           # TFT initialization and display task loop
│   ├── Button.h / Button.cpp       # Mechanical button driver (BtnA, BtnB, BtnC polling)
│   └── screens/                    # ScreenFramework subclasses (fixed tab views)
│       ├── DeviceInfoScreen.h/.cpp
│       ├── DeviceSelectionScreen.h/.cpp
│       └── QRCodeScreen.h/.cpp
│
└── lvgl/                           # Touch-driven renderer for ESP32-S3 targets (M5Stack CoreS3)
    ├── DeviceDisplay.cpp           # BSP bring-up, dark theme, auto-sleep & wake-on-touch
    ├── NavigationStack.h/.cpp      # Hierarchical push/pop navigation with clickable breadcrumbs
    └── screens/                    # Stateless screen render callbacks
        ├── HomeScreen.h/.cpp             # Root menu (Devices, Select Device, System)
        ├── SystemMenuScreen.h/.cpp       # Submenu (QR Code, Status, Operations)
        ├── CommissioningCodesScreen.h/.cpp # Matter onboarding QR code and manual setup code
        ├── DeviceInfoScreen.h/.cpp       # Diagnostics (fabrics, IP, memory, active device)
        ├── DeviceSelectionScreen.h/.cpp  # Device type switching with confirmation dialog
        ├── DeviceEndpointsScreen.h/.cpp  # Summary of bridged endpoints or active endpoint
        └── DeviceOperationsScreen.h/.cpp # Reboot and factory reset triggers with confirmation dialogs
```

---

## 3. Renderers & Build Configuration

Renderer selection is governed by board-specific Kconfig options in
`main/CMakeLists.txt`:

| Renderer | Supported Boards               | Framework & Dependencies                                           |
| :------- | :----------------------------- | :----------------------------------------------------------------- |
| `tft/`   | M5Stack Basic/Gray, WROVER-KIT | `screen-framework` + vendored TFT SPI driver (ESP32 only)          |
| `lvgl/`  | M5Stack CoreS3                 | LVGL 9 + `espressif/m5stack_core_s3` BSP via IDF Component Manager |

> **Note**: The legacy vendored TFT library relies on ESP32-specific register
> definitions and cannot compile on ESP32-S3 or other Xtensa/RISC-V
> architectures. Any new chip architecture must use `lvgl/` or provide its own
> renderer directory.

In `main/CMakeLists.txt`, `PRIV_INCLUDE_DIRS_LIST` and `SRCS` conditionally add
the appropriate subdirectory:

```cmake
if(CONFIG_DEVICE_TYPE_M5STACK_CORES3)
    list(APPEND PRIV_INCLUDE_DIRS_LIST
        "${CMAKE_CURRENT_LIST_DIR}/display/lvgl"
        "${CMAKE_CURRENT_LIST_DIR}/display/lvgl/screens"
    )
    # lvgl sources added to SRCS...
endif()
```

---

## 4. LVGL Renderer Lifecycle & Control Flow

The `lvgl/` renderer manages the 320x240 ILI9342C panel and FT6336U capacitive
touch screen through the following lifecycle:

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
    │                            │  no touch input)          │
    │                            ▼                           │
    │                    ┌───────────────┐                   │
    │                    │ Display Sleep │                   │
    │                    │ - Backlight 0 │                   │
    │                    │ - Wake Overlay│                   │
    │                    └───────┬───────┘                   │
    └────────────────────────────┘───────────────────────────┘
```

### Thread Safety Contract

LVGL is not thread-safe. Any access to LVGL objects, timers, or displays must
occur while holding the LVGL port mutex:

```cpp
if (bsp_display_lock(0))
{
    // LVGL operations...
    bsp_display_unlock();
}
```

Callbacks dispatched by LVGL (such as widget event handlers or timer callbacks)
already execute within the LVGL task context with the lock held.

### Auto-Sleep and Wake-on-Touch

1. An LVGL timer checks `lv_display_get_inactive_time()` every second.
2. If inactive for **30 seconds**, `bsp_display_backlight_off()` sets the
   backlight brightness to 0%.
3. While asleep, a transparent, full-screen hit-catcher widget is mounted on
   `lv_layer_top()`.
4. When touched, the wake overlay captures `LV_EVENT_PRESSED`, calls
   `bsp_display_backlight_on()`, deletes itself, and resets the activity timer.
   This prevents the initial wake tap from unintentionally clicking buttons
   underneath.

---

## 5. Hierarchical Navigation (`NavigationStack`)

Rather than rigid horizontal tabs with cramped titles, the CoreS3 UI uses a
push/pop stack model with dynamic breadcrumb navigation.

### Architecture

-   **Stack Representation**: A vector of
    `{ std::string title, RenderScreenFn renderFn }`.
-   **Top Bar (38px height)**:
    -   Displays the hierarchical breadcrumb path (e.g.
        `Home > System > QR Code`).
    -   Ancestor levels are rendered as clickable button pills (`lv_button`)
        with 6px extended touch padding (`lv_obj_set_ext_click_area(6)`).
        Tapping an ancestor pops directly to that level via
        `NavigationStack::PopTo(level)`.
    -   The active leaf level is rendered as a clean, high-contrast label.
    -   Horizontal scrolling (`LV_DIR_HOR`) is enabled on the breadcrumb
        container to handle deep hierarchies without truncation.
-   **Content Area**:
    -   Dynamically cleans and renders the top screen callback:
        `sStack.back().renderFn(sContentContainer)`.
    -   Configured with column flex layout and vertical scrolling
        (`LV_OBJ_FLAG_SCROLLABLE`).

---

## 6. Developer Extension Guide

### Adding a New Screen

To add a new screen to the LVGL interface:

1. **Declare the screen renderer** in `display/lvgl/screens/MyFeatureScreen.h`:

    ```cpp
    #pragma once
    #include <lvgl.h>

    void ShowMyFeatureScreen(lv_obj_t * parent);
    ```

2. **Implement the UI** in `display/lvgl/screens/MyFeatureScreen.cpp`:

    ```cpp
    #include "MyFeatureScreen.h"

    void ShowMyFeatureScreen(lv_obj_t * parent)
    {
        lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_all(parent, 10, LV_PART_MAIN);

        lv_obj_t * label = lv_label_create(parent);
        lv_label_set_text(label, "My Feature Content");
    }
    ```

3. **Navigate to the screen** from any existing screen or button handler:

    ```cpp
    #include "MyFeatureScreen.h"
    #include "NavigationStack.h"

    static void OnButtonClicked(lv_event_t * event)
    {
        NavigationStack::Push("My Feature", ShowMyFeatureScreen);
    }
    ```

4. **Add the file to CMake**: List `display/lvgl/screens/MyFeatureScreen.cpp` in
   `examples/all-devices-app/esp32/main/CMakeLists.txt` under the
   `CONFIG_DEVICE_TYPE_M5STACK_CORES3` block.

---

### Adding Device-Specific Controls (Phase 4 Extension Pattern)

When single-device mode is active or when an endpoint is selected from
`DeviceEndpointsScreen`, dedicated interactive widgets (sliders, toggles, color
wheels) can be attached:

```cpp
void ShowOnOffPluginScreen(lv_obj_t * parent)
{
    lv_obj_t * toggleBtn = lv_button_create(parent);
    // Wire toggle button to Matter OnOff cluster server commands...
}
```

---

### Adding a New Target Board / Renderer

1. Create a new directory under `display/<new_target>/`.
2. Implement `void InitDeviceDisplay()` and `void ShowRestartingMessage()` in
   `display/<new_target>/DeviceDisplay.cpp`.
3. Add a Kconfig option in `main/Kconfig.projbuild` (e.g.
   `CONFIG_DEVICE_TYPE_MY_BOARD`).
4. Update `main/CMakeLists.txt` to conditionally compile `display/<new_target>/`
   when that option is enabled.
