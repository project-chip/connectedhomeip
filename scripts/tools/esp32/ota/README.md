---
orphan: true
---

# ESP32 OTA image tool

## Overview

`esp32_multi_ota_tool.py` creates and inspects Matter OTA (`.ota`) images for
ESP32. It is a wrapper over the standard
[`ota_image_tool.py`](../../../../src/app/ota_image_tool.py) and supports the
same `create` options, adding a `create-multi` subcommand that packs several
component binaries into a single **multi-image OTA bundle** for use with the
ESP32 multi-image OTA feature (`CONFIG_ENABLE_MULTI_IMAGE_OTA`).

A multi-image bundle is the standard Matter OTA header wrapping a payload of:

```
MultiImageHeader | SubImageHeader[0..N-1] | binary[0] | ... | binary[N-1]
```

where `MultiImageHeader` is an 8-byte preamble (magic `"MIOT"` + image count)
and each 48-byte `SubImageHeader` records a binary's image ID, expected version,
offset, length, and SHA-256 digest.

## Subcommands

| Subcommand      | Purpose                                                              |
| --------------- | -------------------------------------------------------------------- |
| `create`        | Create a single-image OTA file (same as `ota_image_tool.py`).        |
| `create-multi`  | Create a multi-image OTA bundle from a CSV manifest.                 |
| `show`          | Print the OTA header; multi-image bundles are decoded automatically. |
| `extract`       | Write a copy of an OTA file with the Matter OTA header removed.      |
| `change_header` | Rewrite header fields (vendor/product ID, version, ...) in place.    |

## Creating a multi-image bundle

`create-multi` takes the same header options as `create` (vendor ID, product ID,
version, digest algorithm) plus a **CSV manifest** describing the component
binaries, and an output path:

```
./scripts/tools/esp32/ota/esp32_multi_ota_tool.py create-multi \
    -v 0xFFF1 -p 0x8000 -vn 2 -vs "2.0" -da sha256 \
    images.csv my-bundle.ota
```

The manifest has a header row with three columns — `id`, `version`, `path`:

```csv
id,version,path
0x00000100,2,build/coprocessor.bin
0x00000001,2,build/my-app.bin
```

-   `id` — the 4-byte image ID that selects the device-side `SubImageProcessor`.
    Values may be given in decimal or hex. `0x00000001` is the reserved
    application-firmware image ID and **must appear exactly once**.
-   `version` — the expected installed version of that binary.
-   `path` — path to the binary; relative paths are resolved against the
    manifest's directory.

The tool computes each binary's offset, length, and SHA-256, then writes the
bundle with the application image moved to the **last** position (co-processor
entries keep their manifest order, which is the delivery order). It enforces the
bundle rules at packaging time: non-zero and unique image IDs, non-empty
binaries, at most 255 images, and exactly one application image.

## Inspecting an image

```
./scripts/tools/esp32/ota/esp32_multi_ota_tool.py show my-bundle.ota
```

For a multi-image bundle this prints the Matter OTA header followed by the
`MultiImageHeader` and each `SubImageHeader` (image ID, version, offset, length,
SHA-256).

See [Multi-Image OTA](../../../../docs/platforms/esp32/multi_image_ota.md) for
how the bundle is consumed on the device.
