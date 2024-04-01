---
orphan: true
---

# Silabs OTA image tool

## Overview

This tool can generate an OTA image in the `|OTA standard header|TLV1|...|TLVn|`
format. The payload contains data in standard TLV format (not Matter TLV
format). During OTA transfer, these TLV can span across multiple BDX blocks,
thus the `OTAImageProcessorImpl` instance should take this into account.

## Supported platforms

-   EFR32 -

## Usage

This is a wrapper over standard `ota_image_tool.py`, so the options for `create`
are also available here:

```
python3 ./scripts/tools/silabs/ota/ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn 50000 -vs "1.0" -da sha256
```

followed by \*_custom options_- and a positional argument (should be last) that
specifies the output file. Please see the `create_ota_images.sh` for some
reference commands.

The list of **custom options**:

```
# Application options
--app-input-file   --> Path to the application binary.
--app-version      --> Application version. It's part of the descriptor and
                       can be different than the OTA image header version: -vn.
--app-version-str  --> Application version string. Same as above.
--app-build-date   --> Application build date. Same as above.

# SSBL options
--bl-input-file    --> Path to the SSBL binary.
--bl-version       --> SSBL version.
--bl-version-str   --> SSBL version string.
--bl-build-date    --> SSBL build date.

# Factory data options
--factory-data     --> If set, enables the generation of factory data.
--cert_declaration --> Certification Declaration.
--dac_cert         --> DAC certificate.
--dac_key          --> DAC private key.
--pai_cert         --> PAI certificate.

# Custom TLV options
--json             --> Path to a JSON file following ota_payload.schema
```

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
