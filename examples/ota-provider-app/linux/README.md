# ota-provider-app

This is a reference application that implements an example of an OTA Provider
Cluster Server.

## Building

Suggest doing the following:
`scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/debug chip_config_network_layer_ble=false`

## Usage

`./ota-provider-app [-f/--filepath \<file\>] [-o/--otaImageList \<file\>]`

If `--filepath` is supplied, `ota-provider-app` will automatically serve that
file to the OTA Requestor (SoftwareVersion will be Requester version + 1).

If `--otaImageList` is supplied, `ota-provider-app` will parse the JSON file and
extract all required data. Then the most recent, valid software version will be
selected and the corresponding ota-file will be sent to the OTA Requestor.

Here's an example of the otaImageList file contents:

{ "foo": 1, // ignored by parser "deviceSoftwareVersionModel": [ { "vendorId":
1, "productId": 1, "softwareVersion": 10, "softwareVersionString": "1.0.0",
"cDVersionNumber": 18, "softwareVersionValid": true,
"minApplicableSoftwareVersion": 0, "maxApplicableSoftwareVersion": 100,
"otaURL": "/tmp/ota.txt" }, { "vendorId": 1, "productId": 1, "softwareVersion":
20, "softwareVersionString": "1.0.1", "cDVersionNumber": 18,
"softwareVersionValid": false, "minApplicableSoftwareVersion": 0,
"maxApplicableSoftwareVersion": 100, "otaURL": "/tmp/ota.txt" }, { "vendorId":
1, "productId": 1, "softwareVersion": 30, "softwareVersionString": "1.0.2",
"cDVersionNumber": 18, "softwareVersionValid": true,
"minApplicableSoftwareVersion": 0, "maxApplicableSoftwareVersion": 100,
"otaURL": "/tmp/ota.txt" }, { "vendorId": 1, "productId": 1, "softwareVersion":
40, "softwareVersionString": "1.1.0", "cDVersionNumber": 18,
"softwareVersionValid": true, "minApplicableSoftwareVersion": 0,
"maxApplicableSoftwareVersion": 100, "otaURL": "/tmp/ota.txt" }, { "vendorId":
1, "productId": 1, "softwareVersion": 50, "softwareVersionString": "1.1.1",
"cDVersionNumber": 18, "softwareVersionValid": false,
"minApplicableSoftwareVersion": 0, "maxApplicableSoftwareVersion": 100,
"otaURL": "/tmp/ota.txt" } ] }

If neither `--filepath` nor `--otaImageList` are supplied, `ota-provider-app`
will respond to `QueryImage` with `NotAvailable` status.

## Current Features/Limitations

### Features

-   can provide local filepath to serve as OTA image
-   can complete full BDX transfer
-   supports variable-length / startoffset for BDX transfer

### Limitations:

-   Synchronous BDX transfer only
-   using hardcoded test values for local and peer Node IDs
-   does not check VID/PID
-   no configuration for `AwaitNextAction`
-   only one transfer at a time (does not check incoming `UpdateTokens`)
