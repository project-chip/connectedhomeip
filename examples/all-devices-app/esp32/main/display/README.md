# Display

The application talks to the panel through one header, [`DeviceDisplay.h`](DeviceDisplay.h).
`main.cpp` includes nothing else and has no knowledge of the rendering library
in use.

## Renderers

Exactly one renderer is compiled, selected by the board configuration in
`main/CMakeLists.txt`:

| Directory | Used by                       | Built on                                              |
| --------- | ----------------------------- | ----------------------------------------------------- |
| `tft/`    | M5Stack (ESP32), WROVER-KIT   | `examples/common/screen-framework` + the vendored TFT library |
| `lvgl/`   | M5Stack CoreS3 (ESP32-S3)     | LVGL 9, via the `espressif/m5stack_core_s3` board support package |

Each renderer directory provides a `DeviceDisplay.cpp` implementing the
interface, and keeps its screen classes in a `screens/` subdirectory. Screens
are renderer-specific objects: they derive from whatever base class the
rendering library expects.

Note that `SRC_DIRS` and include directories in ESP-IDF are not recursive, so a
new subdirectory must be listed explicitly in `main/CMakeLists.txt`.

## Adding a renderer

1. Create `display/<name>/` with a `DeviceDisplay.cpp` defining every function
   declared in `DeviceDisplay.h`.
2. Add a Kconfig symbol selecting it, and extend the `CONFIG_HAVE_DISPLAY`
   branches of `main/CMakeLists.txt` to compile that directory instead.

The vendored TFT library is ESP32-only, so a board on any other chip needs its
own renderer rather than a port of `tft/`.
