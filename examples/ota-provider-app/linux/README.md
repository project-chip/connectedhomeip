# ota-provider-app

This is a reference application that implements an example of an OTA Provider
Cluster Server.

## Usage

`./ota-provider-app [--filepath \<filepath\>]`

If `-filepath` is provided, `ota-provider-app` will automatically serve that
file to the Requestor (SoftwareVersion will be Requester version + 1).

If no `-filepath` is provided, `ota-provider-app` will respond to `QueryImage`
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
-   no configuration for `Busy`/`DelayedActionTime`
-   no configuration for `AwaitNextAction`
-   only one transfer at a time (does not check incoming `UpdateTokens`)
