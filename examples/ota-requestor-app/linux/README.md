# ota-requestor-app (Linux)

This is a reference application that is both a server for the OTA Requestor
Cluster, as well as a client of the OTA Provider Cluster. It should initiate a
Software Update with a given OTA Provider node, and download a file.

## Usage

In order to use this reference application to connect to a device running OTA
Provider server and download a software image, these commands should be called
in the following order:

In terminal 1:

```
./chip-ota-provider-app -f ${SW_IMAGE_FILE}
```

-   `${SW_IMAGE_FILE}` is the file representing a software image to be served.

In terminal 2:

```
./chip-tool pairing onnetwork ${NODE_ID_TO_ASSIGN_PROVIDER} 20202021
```

-   `${NODE_ID_TO_ASSIGN_PROVIDER}` is the node id to assign to the
    ota-provider-app running in terminal 1.

In terminal 3:

```
./chip-ota-requestor-app -d ${REQUESTOR_LONG_DISCRIMINATOR} -u ${REQUESTOR_UDP_PORT} -i ${PROVIDER_IP_ADDRESS} -n ${PROVIDER_NODE_ID} -q ${DELAY_QUERY_SECONDS}
```

-   `{REQUESTOR_LONG_DISCRIMINATOR}` is the long discriminator specified for the
    ota-requestor-app for commissioning discovery
-   `{REQUESTOR_UDP_PORT}` is the UDP port that the ota-requestor-app listens on
    for secure connections
-   `${PROVIDER_IP_ADDRESS}` is the IP address of the ota-provider-app that has
    been resolved manually
-   `${PROVIDER_NODE_ID}` is the node ID of the ota-provider-app assigned above
-   `${DELAY_QUERY_SECONDS}` is the amount of time in seconds to wait before
    initiating secure session establishment and query for software image

In terminal 2:

```
./chip-tool pairing onnetwork-long ${NODE_ID_TO_ASSIGN_REQUESTOR}  20202021 ${REQUESTOR_LONG_DISCRIMINATOR}
```

-   `${NODE_ID_TO_ASSIGN_REQUESTOR}` is the node id to assign to the
    ota-requestor-app running in terminal 3
-   `${REQUESTOR_LONG_DISCRIMINATOR}` is the long discriminator of the
    ota-requestor-app specified in terminal 3 above

## Current Features / Limitations

### Features

-   Code for running a full BDX download exists in BDX
-   Sends QueryImage command
-   Downloads a file over BDX served by an OTA Provider server
-   Supports various command line configurations

### Limitations

-   Needs chip-tool to commission the OTA Provider device first because the Node
    ID and IP Address of the OTA Provider must be supplied to this reference
    application
-   Does not verify QueryImageResponse message contents
-   Stores the downloaded file in the directory this reference app is launched
    from
-   Does not support AnnounceOTAProvider command or OTA Requestor attributes
