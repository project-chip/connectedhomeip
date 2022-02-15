# ota-requestor-app (Linux)

This is a reference application that is both a server for the OTA Requestor
Cluster, as well as a client of the OTA Provider Cluster. It can initiate a
software update with a given OTA Provider node, and download a file.

## Building

Suggest doing the following:

```
scripts/examples/gn_build_example.sh examples/ota-requestor-app/linux out/debug chip_config_network_layer_ble=false
```

## Usage

In addition to the general options available to all Linux applications, the
following command line options are available for the OTA Requestor application.

| Directory                               | Description                                                                                                                                                                       |
| --------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| -n/--providerNodeId <node ID>           | Node ID of the OTA Provider to connect to (hex format) <br> This assumes that you've already commissioned the OTA Provider node with chip-tool                                    |
| -f/--providerFabricIndex <fabric index> | Fabric index of the OTA Provider to connect to. If none is specified, default value is 1. <br> This assumes that you've already commissioned the OTA Provider node with chip-tool |
| -q/--delayQuery <Time in seconds>       | From boot up, the amount of time to wait before triggering the QueryImage command. If none or zero is supplied, QueryImage will not be triggered automatically.                   |
| -c/--requestorCanConsent                | If supplied, the RequestorCanConsent field of the QueryImage command is set to true. Otherwise, the value is determined by the driver.                                            |

## Software Image Header

All Matter software images must contain a header as defined in section 11.21.1
of the specification. The
[ota_image_tool](https://github.com/project-chip/connectedhomeip/blob/master/src/app/ota_image_tool.py)
is available for generating the required header on a software image. Any
software images that the OTA Requestor application receives must contain the
required header. If the header is missing, the software download will not
succeed.

## Example

One way to use this reference application to connect to a device running OTA
Provider server and download a software image is to issue an AnnounceOTAProvider
command. This will trigger a QueryImage command and start the OTA process. To
test using this method, the following steps should be followed:

### In terminal 1:

**Build the OTA Provider application**

Follow instructions
[here](https://github.com/project-chip/connectedhomeip/tree/master/examples/ota-provider-app/linux#building)

**Run the OTA Provider application**

```
./chip-ota-provider-app --discriminator ${REQUESTOR_LONG_DISCRIMINATOR} --secured-device-port ${REQUESTOR_UDP_PORT} --KVS ${KVS_STORE_LOCATION} --filepath ${SW_IMAGE_FILE}
```

-   `{PROVIDER_LONG_DISCRIMINATOR}` is the long discriminator specified for the
    OTA Provider application for commissioning discovery. If none is supplied,
    the default is 3840. This must be different from the value used by the OTA
    Requestor application.
-   `{PROVIDER_UDP_PORT}` is the UDP port that the OTA Provider application
    listens on for secure connections. If none is supplied, the default is 5540.
    This must be different from the value used by the OTA Requestor application.
-   `{KVS_STORE_LOCATION}` is a location where the KVS items will be stored. If
    none is supplied, the default is /tmp/chip_kvs. This must be different from
    the value used by the OTA Requestor application.
-   `${SW_IMAGE_FILE}` is the file representing a software image to be served.
    This file must include a header as defined in the specification.

### In terminal 2:

**Build the OTA Requestor application**

Follow instructions
[here](https://github.com/project-chip/connectedhomeip/tree/master/examples/ota-requestor-app/linux#building)

**Run the OTA Requestor application:**

```
./chip-ota-requestor-app --discriminator ${REQUESTOR_LONG_DISCRIMINATOR} --secured-device-port ${REQUESTOR_UDP_PORT} --KVS ${KVS_STORE_LOCATION}
```

-   `{REQUESTOR_LONG_DISCRIMINATOR}` is the long discriminator specified for the
    OTA Requestor application for commissioning discovery. If none is supplied,
    the default is 3840. This must be different from the value used by the OTA
    Provider application.
-   `{REQUESTOR_UDP_PORT}` is the UDP port that the OTA Requestor application
    listens on for secure connections. If none is supplied, the default is 5540.
    This must be different from the value used by the OTA Provider application.
-   `{KVS_STORE_LOCATION}` is a location where the KVS items will be stored. If
    none is supplied, the default is /tmp/chip_kvs. This must be different from
    the value used by the OTA Provider application.

### In terminal 3:

**Commission the OTA Provider application**

```
./chip-tool pairing onnetwork-long ${PROVIDER_NODE_ID} 20202021 ${PROVIDER_LONG_DISCRIMINATOR}
```

-   `${PROVIDER_NODE_ID}` is the node id to assign to the OTA Provider
    application running in terminal 1
-   `${PROVIDER_LONG_DISCRIMINATOR}` is the long discriminator of the OTA
    Provider application specified in terminal 1 above

**Commission the OTA Requestor application**

```
./chip-tool pairing onnetwork-long ${REQUESTOR_NODE_ID} 20202021 ${REQUESTOR_LONG_DISCRIMINATOR}
```

-   `${REQUESTOR_NODE_ID}` is the node id to assign to the OTA Requestor
    application running in terminal 2
-   `${REQUESTOR_LONG_DISCRIMINATOR}` is the long discriminator of the OTA
    Requestor application specified in terminal 2 above

**Issue the AnnounceOTAProvider command**

```
./chip-tool otasoftwareupdaterequestor announce-ota-provider ${PROVIDER_NODE_ID} 0 0 0 ${REQUESTOR_NODE_ID} 0
```

-   `${PROVIDER_NODE_ID}` is the OTA Provider application node ID assigned
    during the pairing step above
-   `${REQUESTOR_NODE_ID}` is the OTA Requestor application node ID assigned
    during the pairing step above

The OTA Requestor application will process this command and send a QueryImage
command to the OTA Provider

## Limitations

-   Stores the downloaded file in the directory this reference app is launched
    from
