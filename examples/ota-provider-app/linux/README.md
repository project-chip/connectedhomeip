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

If `--otaImageList` is supplied, `ota-provider-app` will parse the CSV file
and extract all required data. Then the most recent software version will be
selected and sent to the OTA Requestor.

The `ota-provider-app`'s Basic CSV parser expects a precise format -
Device Software Version Model Schema (lines beginning with # are ignored).

vendorId,productId,softwareVersion,softwareVersionString,CDVersionNumber,
softwareVersionValid,minApplicableSoftwareVersion,maxApplicableSoftwareVersion,otaURL

Here's an example of the otaImageList file contents:

#vendorId,productId,softwareVersion,softwareVersionString,CDVersionNumber,softwareVersionValid,minApplicableSoftwareVersion,maxApplicableSoftwareVersion,otaURL
1,1,10,Version_1,18,true,0,100,/tmp/ota.txt
1,1,20,Version_2,14,true,0,100,/tmp/ota.txt
1,1,30,Version_3,49,true,0,100,/tmp/ota.txt
1,1,40,Version_4,10,true,0,100,/tmp/ota.txt
1,1,50,Version_5,23,true,0,100,/tmp/ota.txt


TODO: Replace with a JSON file for simpler parsing. The idea is that the contents of the
response from the DCL server will be populated into this file appropriately.

If neither `--filepath` nor `--otaImageList` are supplied, `ota-provider-app` will respond to `QueryImage` with `NotAvailable` status.

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
