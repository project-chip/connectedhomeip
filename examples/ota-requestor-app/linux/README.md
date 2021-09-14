# ota-requestor-app (Linux)

WARNING: this app currently does not build successfully. It is being submitted
as a starting point for further OTA Requestor development.

This is a reference application that is both a server for the OTA Requestor
Cluster, as well as a client of the OTA Provider Cluster. It should initiate a
Software Update with a given OTA Provider node, and download a file.

## Current Features / Limitations

### Features

-   Code for running a full BDX download exists in BDX
-   Sends QueryImage command

### Limitations

-   needs chip-tool to pair to the Provider device first, so it can steal the
    CASE session from persisted memory
-   uses Controller class to load the CASE session
-   Controller does not provide any way to access a new ExchangeContext for the
    BDX exchange
-   doesn't wait for QueryImageResponse to begin the BDX exchange
-   does not verify QueryImageResponse message contents
-   stores the downloaded file at a hardcoded filepath
-   doesn't close the BDX ExchangeContext when the exchange is over
-   only uses hardcoded node IDs
-   does not support AnnounceOTAProvider command or OTA Requestor attributes
