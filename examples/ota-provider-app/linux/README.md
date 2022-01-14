# ota-provider-app

This is a reference application that implements an example of an OTA Provider
Cluster Server.

## Building

Suggest doing the following:
`scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/debug chip_config_network_layer_ble=false`

## Usage

`./ota-provider-app [-f/--filepath \<file\>]`

If `--filepath` is supplied, `ota-provider-app` will automatically serve that
file to the OTA Requestor (SoftwareVersion will be Requester version + 1).

If no `--filepath` is supplied, `ota-provider-app` will respond to `QueryImage`
with `NotAvailable` status.

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
