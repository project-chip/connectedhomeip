---
orphan: true
---

# NXP OTA image tool

## Overview

This tool can generate an OTA image in the `|OTA standard header|TLV1|...|TLVn|`
format. The payload contains data in standard TLV format (not Matter TLV
format). During OTA transfer, these TLV can span across multiple BDX blocks,
thus the `OTAImageProcessorImpl` instance should take this into account.

For details related to the OTA implementation, please see
[OTA README](../../../../src/platform/nxp/common/ota/README.md).

## Supported platforms

-   `k32w0`
-   `k32w1`
-   `mcxw71`

## Usage

This is a wrapper over standard `ota_image_tool.py`, so the options for `create`
are also available here:

```
python3 ./scripts/tools/nxp/ota/ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn 50000 -vs "1.0" -da sha256
```

followed by **custom options**- and a positional argument (should be last) that
specifies the output file. Please see the `create_ota_images.sh` for some
reference commands.

The list of **custom options**:

| option               | description                                        |
| -------------------- | -------------------------------------------------- |
| `--app-input-file`   | Path to the application binary                     |
| `--app-version`      | Application version. Can differ from `-vn`         |
| `--app-version-str`  | Application version string. Same as above          |
| `--app-build-date`   | Application build date. Same as above              |
| `--bl-input-file`    | Path to the SSBL binary                            |
| `--bl-version`       | SSBL version                                       |
| `--bl-version-str`   | SSBL version string                                |
| `--bl-build-date`    | SSBL build date                                    |
| `--factory-data`     | Enable the generation of factory data              |
| `--cert_declaration` | Matter Certification Declaration                   |
| `--dac_cert`         | Matter DAC certificate                             |
| `--dac_key`          | Matter DAC private key                             |
| `--pai_cert`         | Matter PAI certificate                             |
| `--json`             | Path to a JSON file following `ota_payload.schema` |

Please note that the options above are separated into four categories:
application, bootloader, factory data and custom TLV (`--json` option). If no
descriptor options are specified for app/SSBL, the script will use the default
values (`50000`, `"50000-default"`, `"2023-01-01"`). The descriptor feature is
optional, TLV processors having the option to register a callback for descriptor
processing.

## Custom payload

When defining a custom processor, a user is able to also specify the custom
format of the TLV by creating a JSON file based on the `ota_payload.schema`. The
tool offers support for describing multiple TLV in the same JSON file. Please
see the `examples/ota_max_entries_example.json` for a multi-app + SSBL example.
Option `--json` must be used to specify the path to the JSON file.

## Examples

A set of examples can be found in `./examples`. Please run `create_ota_image.sh`
to generate the examples:

-   Application image with default descriptor
-   Application image with specified descriptor
-   Factory data image
-   SSBL image
-   Application + SSBL + factory data image
-   Maximum number of entries image, using `ota_max_entries_example.json`. The
    examples uses 8 SSBL binaries because they have a small size and fit in
    external flash.

The binaries from `./examples/binaries` should only be used only as an example.
The user should provide their own binaries when generating the OTA image.
