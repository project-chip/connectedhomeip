# Display Subsystem

This directory contains the display implementations for `all-devices-app` on
ESP32 targets.

---

## Integration Boundary

`main.cpp` interacts with the display subsystem through three functions declared
in `DeviceDisplay.h`:

```cpp
void InitDeviceDisplay();            // from app_main, before the CHIP event loop starts
void ShowRestartingMessage();
void InitDisplayDataModelListener(); // from the CHIP thread, after Server::Init
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
│  - InitDisplayDataModelListener()                      │
└──────────────┬───────────────────────────┬─────────────┘
               │ (ESP32)                   │ (ESP32-S3, ESP32-S31)
               ▼                           ▼
┌───────────────────────────────┐ ┌──────────────────────────┐
│        tft/ Renderer          │ │      lvgl/ Renderer      │
│  - Vendored SPI TFT driver    │ │  - LVGL 9                │
│  - ScreenFramework (C++)      │ │  - Capacitive touch      │
│  - Mechanical buttons (A/B/C) │ │  - board/DisplayBoard.h  │
└───────────────────────────────┘ └────────────┬─────────────┘
                                               │ one backend per board
                                   ┌───────────┴───────────┐
                                   ▼                       ▼
                        ┌────────────────────┐ ┌────────────────────┐
                        │ CoreS3DisplayBoard │ │ MosaicoDisplayBoard│
                        │ m5stack_core_s3 BSP│ │ esp-mosaico-bsp    │
                        │ esp_lvgl_port      │ │ esp_lvgl_adapter   │
                        └────────────────────┘ └────────────────────┘
```

---

## Directory Layout

```
display/
├── DeviceDisplay.h                 # Public interface (InitDeviceDisplay, ShowRestartingMessage, InitDisplayDataModelListener)
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
└── lvgl/                           # Touch-driven renderer (M5Stack CoreS3, ESP-Mosaico)
    ├── DeviceDisplay.cpp           # Display bring-up, lifecycle, and sleep
    ├── Ui.h/.cpp                   # Palette, fonts, size metrics and shared builders (tiles, badges, pills, animations)
    ├── Theme.h/.cpp                # LVGL theme built on the default one: cards, buttons, sliders, dialogs, tables
    ├── DisplayWake.h               # WakeDisplay(): undo an inactivity blank (display lock held)
    ├── CommissioningMonitor.h/.cpp # Platform events -> stage text on the QR screen, Home on success
    ├── board/                      # Board support package bindings, one backend compiled per board
    │   ├── DisplayBoard.h              # Start, Lock, Unlock, SetBrightness, BoardName, EdgeInset
    │   ├── CoreS3DisplayBoard.cpp      # M5Stack CoreS3: espressif/m5stack_core_s3 + esp_lvgl_port
    │   └── MosaicoDisplayBoard.cpp     # ESP-Mosaico: esp-mosaico-bsp + esp_lvgl_adapter
    ├── DisplayNotificationHub.h/.cpp # Bridges Matter DataModel changes to LVGL widgets
    ├── NavigationStack.h/.cpp      # Push/pop screen navigation and breadcrumbs
    ├── DeviceScreenRegistry.h/.cpp # Registry of device screens, keyed by endpoint
    ├── DeviceScreenHook.h          # DeviceFactory post-registration hook
    ├── DeviceScreenRegistration.h/.cpp # Per-device-type registration overloads
    ├── clusters/                   # Reusable cluster widgets, one card each
    │   ├── OnOffClusterWidget                 # State label and toggle button with caller-supplied labels
    │   ├── LevelControlClusterWidget          # Level slider with percentage and caller-supplied level name
    │   ├── ColorControlClusterWidget          # Mode selector plus hue/sat, xy and temperature groups
    │   ├── FanControlClusterWidget            # Speed slider and mode buttons
    │   ├── TemperatureMeasurementClusterWidget # Measured value and setter
    │   ├── OccupancySensingClusterWidget      # Occupancy state and toggle
    │   ├── BooleanStateClusterWidget          # Boolean state with caller-supplied labels
    │   ├── IdentifyClusterWidget              # Read-only identify countdown
    │   ├── ChimeClusterWidget                 # Sound selection, enable and play
    │   ├── ValveConfigurationAndControlClusterWidget # Valve state, countdown, level and open/close
    │   └── BridgedDeviceBasicInformationClusterWidget # Node label and reachability
    ├── devices/                    # Matter device type screens (composed from cluster widgets)
    │   ├── DeviceHeader.h/.cpp           # Standard device metadata header card
    │   ├── OnOffLoadScreen              # Any OnOffLoad device (light, plug-in unit, mounted control)
    │   ├── DimmableLoadScreen           # Any DimmableLoad device
    │   ├── ColorLightScreen             # Color temperature and extended color lights
    │   ├── FanLoadScreen                # Fan, air purifier, extractor hood
    │   ├── BooleanStateSensorScreen     # Contact, water leak, freeze and rain sensors
    │   ├── OccupancySensorScreen        # Occupancy sensor
    │   ├── TemperatureSensorScreen      # Temperature sensor
    │   ├── ChimeScreen                  # Chime
    │   ├── SpeakerScreen                # Speaker (mute and volume)
    │   ├── WaterValveScreen             # Water valve
    │   ├── BridgedNodeScreen            # Bridged node
    │   └── AggregatorScreen             # Aggregator
    └── screens/                    # Application screens, reached by navigation
        ├── HomeScreen.h/.cpp             # Root menu (Devices, Update Device Type, System)
        ├── SystemMenuScreen.h/.cpp       # Submenu (QR Code, Status, Operations)
        ├── CommissioningCodesScreen.h/.cpp # Matter onboarding QR code and manual setup code
        ├── DeviceInfoScreen.h/.cpp       # Diagnostics (fabrics, IP, memory, active device)
        ├── DeviceSelectionScreen.h/.cpp  # Device type switching with confirmation modal
        ├── DeviceEndpointsScreen.h/.cpp  # Endpoint tree of registered device screens
        └── DeviceOperationsScreen.h/.cpp # Reboot and factory reset triggers with confirmation modals
```

`screens/` and `devices/` both hold screens; the split is by what they render.
`screens/` are fixed application views, declared in the global namespace as
`void Show<Name>(lv_obj_t * parent)` and pushed onto `NavigationStack` by name
from other screens. `devices/` are per-device-type views in `chip::app`, take
the device as an argument, and are reached only through `DeviceScreenRegistry`.

`DeviceEndpointsScreen` renders an `lv_table`: one row per registered device
screen, indented by its position in the endpoint tree, with the endpoint number
in a second column. Rows whose device type has no screen are drawn in red and
cannot be opened.

---

## Build Configuration

Exactly one renderer is compiled per target. `main/Kconfig.projbuild` derives
`CONFIG_HAVE_DISPLAY` (any board with a panel) and `CONFIG_DISPLAY_LVGL` (CoreS3
and Mosaico) from the selected board; `main/CMakeLists.txt` switches on those:

| Renderer | Supported Targets              | Framework & Dependencies                            |
| :------- | :----------------------------- | :-------------------------------------------------- |
| `tft/`   | M5Stack Basic/Gray, WROVER-KIT | `screen-framework` + SPI TFT driver (ESP32 only)    |
| `lvgl/`  | M5Stack CoreS3                 | LVGL 9 + `espressif/m5stack_core_s3` BSP            |
| `lvgl/`  | ESP-Mosaico (ESP32-S31)        | LVGL 9 + `esp-mosaico-bsp` (`ESP_MOSAICO_BSP_PATH`) |

The LVGL renderer never includes a BSP header directly. Everything it needs from
the board (start the panel and LVGL task, take and release the LVGL lock, blank
and restore the light output) goes through `display/lvgl/board/DisplayBoard.h`,
and `main/CMakeLists.txt` compiles exactly one backend from
`display/lvgl/board/` for the selected `CONFIG_DEVICE_TYPE_*`. Each backend
wraps the LVGL port of its BSP: `esp_lvgl_port` on the CoreS3,
`esp_lvgl_adapter` on the Mosaico.

The legacy `tft/` driver accesses ESP32 hardware registers directly and does not
compile for ESP32-S3. Targets using other chips must use `lvgl/` or add a
dedicated renderer.

Sources are picked up per directory, not per file:

```cmake
if (CONFIG_DISPLAY_LVGL)
    list(APPEND SRC_DIRS_LIST
        "${CMAKE_CURRENT_LIST_DIR}/display/lvgl"
        "${CMAKE_CURRENT_LIST_DIR}/display/lvgl/screens"
        "${CMAKE_CURRENT_LIST_DIR}/display/lvgl/clusters"
        "${CMAKE_CURRENT_LIST_DIR}/display/lvgl/devices"
    )
endif()
# ...
idf_component_register(PRIV_INCLUDE_DIRS ${PRIV_INCLUDE_DIRS_LIST}
                       SRC_DIRS ${SRC_DIRS_LIST}
                       PRIV_REQUIRES ${BOARD_REQUIRES})
target_sources(${COMPONENT_LIB} PRIVATE ${BOARD_SRCS})
```

`display/lvgl/board/` and `devices/chime/` hold one file per board, so those are
added through `BOARD_SRCS` for the selected board instead of `SRC_DIRS`.

`SRC_DIRS` compiles every source file in each listed directory, so a new `.cpp`
in one of them builds without any CMake change. Adding a new _subdirectory_ does
require an edit, in both `SRC_DIRS_LIST` and `PRIV_INCLUDE_DIRS_LIST`. The
directory is scanned at configure time, so an existing build directory needs
`idf.py reconfigure` to notice a new file.

---

## LVGL Renderer Runtime Flow

```
                  ┌───────────────────────────────┐
                  │      InitDeviceDisplay()      │
                  └──────────────┬────────────────┘
                                 │
                                 ▼
                  ┌───────────────────────────────┐
                  │     DisplayBoard::Start()     │  (BSP: power rails, panel, touch
                  └──────────────┬────────────────┘   and the LVGL task)
                                 │
                                 ▼
                  ┌───────────────────────────────┐
                  │     DisplayBoard::Lock()      │
                  │  - InitTheme()                │
                  │  - NavigationStack::Init()    │
                  │  - Push Home                  │
                  │  - Register Inactivity Timer  │
                  │  - SetBrightness(on)          │  (Delayed until there is a frame)
                  │    DisplayBoard::Unlock()     │
                  └──────────────┬────────────────┘
                                 │
                                 ▼
                  ┌───────────────────────────────┐
                  │ InitDisplayDataModelListener()│  (CHIP thread, after Server::Init:
                  │  - Hub Init()                 │   the fabric table is only populated
                  │  - CommissioningMonitor::Init()│
                  │  - Push System > QR Code      │   by then. Pushed only when
                  │    when FabricCount() == 0    │   the device is not commissioned)
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
    │                    │ - Fade to 0   │                   │
    │                    │ - Wake Overlay│                   │
    │                    └───────┬───────┘                   │
    └────────────────────────────┘───────────────────────────┘
```

### Look and Feel

`Theme.cpp` extends the default LVGL dark theme with an apply callback, so every
widget is styled at creation: plain containers become cards, and buttons,
sliders, switches and message boxes get matching styles. Screens compose from
the builders in `Ui.h` and take sizes from `Ui::Px(large, small)`, which picks
the first value on 480-class panels and the second on 320x240, so the CoreS3
keeps its compact layout. Fonts follow the same rule.

### Thread Safety

Two threads touch this code: the LVGL port task, which owns every `lv_obj_t`,
and the CHIP event loop, which owns the data model. Each has a lock, and the
ordering between them is fixed:

> [!IMPORTANT]
>
> The CHIP thread may take the display lock; it does so on every attribute
> change. The LVGL task must therefore **never** take the CHIP stack lock — that
> is the opposite order and deadlocks.

LVGL operations must run under the LVGL port mutex:

```cpp
if (DisplayBoard::Lock())
{
    // LVGL operations...
    DisplayBoard::Unlock();
}
```

Callbacks dispatched from within the LVGL task (such as widget event handlers or
timer callbacks) already execute with this lock held.

The one-way rule dictates how each direction is written:

-   **Writes to the data model** (a touch that sends a command) are posted to
    the CHIP event loop with `DeviceLayer::SystemLayer().ScheduleLambda()`.
-   **Reads of cluster state** are done directly from the LVGL task through the
    cluster getters, without any lock. The values read are scalars, no CHIP API
    asserting the stack lock is involved, and a torn read only shows a stale
    value until the next notification repaints it.
-   **Reads that must be accurate** cannot use the shortcut above. They schedule
    the read onto the CHIP event loop, then take the display lock to apply the
    result, which means the widget renders a placeholder first.

### Data Model Notifications & Cross-Thread Synchronization

UI widgets display dynamic device state that can be updated either locally via
touch or remotely via Matter protocol commands (over Wi-Fi/Thread).

#### 1. UI to Matter (Touch Interactions)

Local touch handlers must **never** synchronously block on
`chip::DeviceLayer::StackLock` while holding `DisplayBoard::Lock()` to avoid
ABBA deadlocks with the CHIP thread. Instead, user touch callbacks
asynchronously post cluster mutations using
`DeviceLayer::SystemLayer().ScheduleLambda()`:

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
DisplayBoard::Lock()
       │
       ▼
DisplayNotificationHub dispatches to matching widget callbacks
       │
       ▼
Widget lambda updates LVGL labels/sliders directly
       │
       ▼
DisplayBoard::Unlock()
```

#### 3. Automatic Lifecycle & RAII Unregistration

To prevent dangling pointers when screens are popped from `NavigationStack`,
widgets subscribe using their parent container (`card`):

```cpp
DisplayNotificationHub::Instance().Subscribe(
    card, cluster.GetPaths()[0].mEndpointId, Clusters::OnOff::Id,
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
`DisplayBoard::Lock()`, they are strictly serialized and cannot race.

### Commissioning Progress

`CommissioningMonitor` registers a platform event handler from
`InitDisplayDataModelListener()` and turns the commissioning session into a
status line under the manual pairing code:

| Event                                          | Status line                                   |
| :--------------------------------------------- | :-------------------------------------------- |
| `kCHIPoBLEConnectionEstablished`               | Controller connected over BLE                 |
| `kSecureSessionEstablished` (PASE)             | Passcode verified, secure session open        |
| `kOperationalNetworkEnabled`                   | Credentials received, joining Wi-Fi           |
| `kWiFiConnectivityChange` (established)        | Joined Wi-Fi, waiting for the controller      |
| `kSecureSessionEstablished` (CASE)             | Operational session established               |
| `kCommissioningComplete`                       | Commissioning successful (green)              |
| `kFailSafeTimerExpired`                        | Commissioning failed: timed out (red)         |
| `kCHIPoBLEConnectionClosed` before credentials | Commissioning failed: controller disconnected |

Before any of these the line reads "Waiting for a controller", or "Commissioned"
when the node already has a fabric. A commissioned node shows a notice in place
of the codes: the root node has no Basic Commissioning Method, so a window
opened later carries its own passcode and the printed codes would not pair. The
fabric count behind this comes from a `FabricTable::Delegate`, so a fabric
removed by a controller brings the codes back. Every update wakes a blanked
panel. On success, and only while the QR screen is the one showing, an LVGL
one-shot timer pops the stack back to Home after the verdict has been on screen
for a moment, and the line returns to "Commissioned".

### Auto-Sleep and Wake-on-Touch

1. An LVGL timer checks `lv_display_get_inactive_time()` every second.
2. After 30 seconds of inactivity, an LVGL animation fades the brightness to 0
   through `DisplayBoard::SetBrightness()`.
3. A transparent full-screen overlay is added to `lv_layer_top()`.
4. The first touch on the sleeping display triggers `LV_EVENT_PRESSED` on the
   overlay, which calls `WakeDisplay()`: brightness back on, overlay deleted,
   resets the inactivity timer. The wake touch is absorbed so underlying widgets
   are not activated.

---

## Hierarchical Navigation (`NavigationStack`)

The LVGL UI uses a push/pop stack model with clickable breadcrumb navigation.

### Structure

-   **Stack**: `std::vector<StackEntry>` where `StackEntry` contains
    `{ std::string title, RenderScreenFn renderFn }`.
-   **Top Bar**: shows the ancestors of the current page, not the page itself —
    its title is already the content below. The trailing crumb is scrolled into
    view so the deepest ancestor stays visible.
    -   Ancestor levels: `lv_button` pills. Tapping an ancestor pops directly to
        that level via `NavigationStack::PopTo(level)`.
    -   Root: with a single entry on the stack there is no ancestor, so that
        entry is rendered as a static label instead.
    -   Container: `sCrumbContainer` fills the top bar with horizontal scrolling
        enabled (`LV_DIR_HOR`) without a scrollbar.
-   **Content Area**:
    -   Fills the remaining vertical height.
    -   Cleared on transition and repopulated by calling
        `renderFn(sContentContainer)`.
    -   Vertical scrolling enabled (`LV_OBJ_FLAG_SCROLLABLE`) by default; each
        `renderFn` configures its own flex layout on `parent`.

---

## Adding New Components

### Adding a Screen to `lvgl/`

File names carry the `Screen` suffix; function names do not, except in
`devices/`, where `Show<Name>Screen` keeps the per-device entry points apart
from the fixed views.

1. **Declare the entry point** in `display/lvgl/screens/<Name>Screen.h`:

    ```cpp
    #pragma once
    #include <lvgl.h>

    void Show<Name>(lv_obj_t * parent);
    ```

2. **Implement widgets** in `display/lvgl/screens/<Name>Screen.cpp`:

    ```cpp
    #include "<Name>Screen.h"

    void Show<Name>(lv_obj_t * parent)
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
        NavigationStack::Push("<Title>", Show<Name>);
    }
    ```

No build change is needed: `display/lvgl/screens` is already listed in
`SRC_DIRS`. Run `idf.py reconfigure` if an existing build directory does not
pick the new file up.

### Adding Device Controls

Device screens are not pushed by name. They are registered as the device is
created and rendered later from `DeviceEndpointsScreen`. `DeviceFactory` is
instantiated with `DeviceScreenHook` (see `main/AppDeviceFactory.h`), which
calls `RegisterDeviceScreen(device, DeviceScreenRegistry::Instance())` on the
CHIP thread once the device has been registered with the data model.

To give a device type a screen:

1. Add `display/lvgl/devices/<Name>Screen.h/.cpp` exposing
   `void Show<Name>Screen(lv_obj_t * parent, <DeviceType> & device)`, composed
   from the widgets in `clusters/`. Prefer a capability type (`OnOffLoad`,
   `DimmableLoad`, `FanLoad`) over a concrete device type so several device
   types can share one screen.
2. Declare `void RegisterDeviceScreen(<DeviceType> &, DeviceScreenRegistry &);`
   in `DeviceScreenRegistration.h`.
3. Define it in `DeviceScreenRegistration.cpp`, filling in a title, the
   endpoint, and a `renderFn` that calls the screen.
4. Include the device type header in `DeviceScreenRegistration.h` if it is not
   already there.

> [!WARNING]
>
> Step 2 is not optional. `DeviceScreenHook` detects support with
> `std::void_t<decltype(RegisterDeviceScreen(...))>`, so a definition that is
> not declared in the header — or an overload that is ambiguous — makes the
> trait false. The build succeeds and the device silently falls back to
> `RegisterMissingDeviceScreen`, appearing in the list as
> `No UI - device type 0x…`. If a new device type shows up that way, a missing
> declaration is the first thing to check.

Devices that own several endpoints register one entry per endpoint. Entries are
placed in the tree by their parent endpoint, which `DeviceScreenRegistry` reads
from the data model provider injected with `SetEndpointSource()`.

### Adding a New Target Board

If the board has a touch panel and an ESP-IDF BSP with an LVGL port, reuse the
`lvgl/` renderer:

1. Add `display/lvgl/board/<Board>DisplayBoard.cpp` implementing the functions
   declared in `DisplayBoard.h` on top of the BSP.
2. Add a `DEVICE_TYPE_<BOARD>` choice in `main/Kconfig.projbuild` and include it
   in the `HAVE_DISPLAY` and `DISPLAY_LVGL` defaults.
3. Select the backend in the `BOARD_SRCS` block of `main/CMakeLists.txt` and add
   the BSP to `main/idf_component.yml` under a `target ==` rule, or through
   `EXTRA_COMPONENT_DIRS` if it is not on the registry.
4. Add an `sdkconfig_<board>.defaults` with the LVGL requirements of the BSP
   (color depth, `LV_CONF_SKIP`, allocator) plus `LV_USE_QRCODE`,
   `LV_USE_TABVIEW` and `LV_USE_MSGBOX` for the screens here.

For anything else, add a renderer:

1. Create directory `display/<target>/`.
2. Implement all three entry points declared in `DeviceDisplay.h` in
   `display/<target>/DeviceDisplay.cpp`: `InitDeviceDisplay()`,
   `ShowRestartingMessage()` and `InitDisplayDataModelListener()`. `main.cpp`
   calls all three, so omitting one fails the link; a renderer with no data
   model listener can leave that one empty.
3. Add a Kconfig option in `main/Kconfig.projbuild`.
4. Update `main/CMakeLists.txt` to conditionally compile `display/<target>/`
   when that Kconfig option is set.
