# ota-requestor-app (Linux)

This is a reference application that is both a server for the OTA Requestor
Cluster, as well as a client of the OTA Provider Cluster. It should initiate a
Software Update with a given OTA Provider node, and download a file.

## Usage

Due to #9518, this app must pretend to be `chip-tool` in order to establish a
connection to the OTA Provider. It does this by reading the CASE session and
other necessary credentials stored in persistent memory on startup.

Therefore, to use this app you should call these commands in the following
order:

In one terminal:

```
./chip-ota-provider-app [-f <filepath>]
```

In a second terminal:

```
./chip-tool pairing onnetwork-long node-id-to-assign 20202021 3840
./chip-ota-requestor-app [-p <provider node id>]
```

where `node-id-to-assign` is the node id to assign to the provider app, which
should match the `-p` argument passed to `chip-ota-requestor-app`.

## Current Features / Limitations

### Features

-   Code for running a full BDX download exists in BDX
-   Sends QueryImage command
-   Takes a peer Node ID as an argument
-   Takes an optional udpPort command argument which when present disables the
    self-commissioning and automatic image download logic and instead makes the
    Requestor initialize like a standard Matter application. The udpPort
    parameter specifies the UDP Port that the Requestor listens on for secure
    connections
-   Takes an optional discriminator command argument to specify the Setup
    Discriminator. When the parameter is omitted the value of 3840 is used

### Limitations

-   needs chip-tool to pair to the Provider device first, so it can steal the
    CASE session from persisted memory
-   uses Controller class to load the CASE session
-   does not verify QueryImageResponse message contents
-   stores the downloaded file at a hardcoded filepath
-   doesn't close the BDX ExchangeContext when the exchange is over
-   does not support AnnounceOTAProvider command or OTA Requestor attributes
