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

| Directory                                   | Description                                                                                                                                 |
| ------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------- |
| -p/--periodicQueryTimeout <Time in seconds> | Periodic timeout for querying providers in the default OTA provider list. If none or zero is supplied the timeout is set to every 24 hours. |
| -c/--requestorCanConsent                    | If supplied, the RequestorCanConsent field of the QueryImage command is set to true. Otherwise, the value is determined by the driver.      |
| -f/--otaDownloadPath <file path>            | If supplied, the OTA image is downloaded to the given fully-qualified file-path. Otherwise, the value defaults to /tmp/test.bin.            |

## Software Image Header

All Matter software images must contain a header as defined in section 11.21.1
of the specification. The
[ota_image_tool](https://github.com/project-chip/connectedhomeip/blob/master/src/app/ota_image_tool.py)
is available for generating the required header on a software image. Any
software images that the OTA Requestor application receives must contain the
required header. If the header is missing, the software download will not
succeed.

## Common Instructions

The Linux
[ota-provider-app](https://github.com/project-chip/connectedhomeip/tree/master/examples/ota-provider-app/linux)
can be used together with this reference application to test OTA features. Here
are some common instructions for building and commissioning the applications.

### OTA Provider application

#### Build the OTA Provider application

Follow instructions
[here](https://github.com/project-chip/connectedhomeip/tree/master/examples/ota-provider-app/linux#building)

#### Run the OTA Provider application

```
out/chip-ota-provider-app --discriminator ${PROVIDER_LONG_DISCRIMINATOR} --secured-device-port ${PROVIDER_UDP_PORT} --KVS ${KVS_STORE_LOCATION} --filepath ${SW_IMAGE_FILE}
```

-   `${PROVIDER_LONG_DISCRIMINATOR}` is the long discriminator specified for the
    OTA Provider application for commissioning discovery. If none is supplied,
    the default is 3840. This must be different from the value used by the OTA
    Requestor application.
-   `${PROVIDER_UDP_PORT}` is the UDP port that the OTA Provider application
    listens on for secure connections. If none is supplied, the default is 5540.
    This must be different from the value used by the OTA Requestor application.
-   `${KVS_STORE_LOCATION}` is a location where the KVS items will be stored. If
    none is supplied, the default is /tmp/chip_kvs. This must be different from
    the value used by the OTA Requestor application.
-   `${SW_IMAGE_FILE}` is the file representing a software image to be served.
    This file must include a header as defined in the specification.

#### Commission the OTA Provider application

```
out/chip-tool pairing onnetwork-long ${PROVIDER_NODE_ID} 20202021 ${PROVIDER_LONG_DISCRIMINATOR}
```

-   `${PROVIDER_NODE_ID}` is the node id to assign to the OTA Provider
    application running
-   `${PROVIDER_LONG_DISCRIMINATOR}` is the long discriminator of the OTA
    Provider application running

### OTA Requestor application

#### Build the OTA Provider application

Follow instructions
[here](https://github.com/project-chip/connectedhomeip/tree/master/examples/ota-requestor-app/linux#building)

#### Run the OTA Requestor application:

```
out/chip-ota-requestor-app --discriminator ${REQUESTOR_LONG_DISCRIMINATOR} --secured-device-port ${REQUESTOR_UDP_PORT} --KVS ${KVS_STORE_LOCATION} --periodicQueryTimeout ${TIME_IN_SECONDS}

```

-   `${REQUESTOR_LONG_DISCRIMINATOR}` is the long discriminator specified for
    the OTA Requestor application for commissioning discovery. If none is
    supplied, the default is 3840. This must be different from the value used by
    the OTA Provider application.
-   `${REQUESTOR_UDP_PORT}` is the UDP port that the OTA Requestor application
    listens on for secure connections. If none is supplied, the default is 5540.
    This must be different from the value used by the OTA Provider application.
-   `${KVS_STORE_LOCATION}` is a location where the KVS items will be stored. If
    none is supplied, the default is /tmp/chip_kvs. This must be different from
    the value used by the OTA Provider application.
-   `${TIME_IN_SECONDS}` is the periodic timeout for querying providers in the
    default OTA provider list. If none or zero is supplied the timeout is set to
    every 24 hours.

#### Commission the OTA Requestor application

```
out/chip-tool pairing onnetwork-long ${REQUESTOR_NODE_ID} 20202021 ${REQUESTOR_LONG_DISCRIMINATOR}
```

-   `${REQUESTOR_NODE_ID}` is the node id to assign to the OTA Requestor
    application running
-   `${REQUESTOR_LONG_DISCRIMINATOR}` is the long discriminator of the OTA
    Requestor application running

## Examples

There are two methods for this reference application to connect to a device
running OTA Provider server and download a software image.

If the ACL entry on the provider has not been properly installed, the QueryImage
command will be denied. Follow instructions
[here](https://github.com/project-chip/connectedhomeip/tree/master/examples/ota-provider-app/linux#access-control-requirements)
to install. Note that this only needs to be done once. There is no need to write
this ACL entry again unless the KVS store has been removed.

### Trigger using AnnounceOTAProvider Command

When the `AnnounceOTAProvider` command is received, it will trigger a QueryImage
command to the provider specified in the command and start the OTA process.

#### In terminal 1:

**Build the OTA Provider application**

```
scripts/examples/gn_build_example.sh examples/ota-provider-app/linux/ out chip_config_network_layer_ble=false
```

**Run the OTA Provider application**

```
out/chip-ota-provider-app --discriminator 22 --KVS /tmp/chip_kvs_provider --filepath /tmp/test.bin
```

#### In terminal 2:

**Build the OTA Requestor application**

```
scripts/examples/gn_build_example.sh examples/ota-requestor-app/linux/ out chip_config_network_layer_ble=false
```

**Run the OTA Requestor application**

```
out/chip-ota-requestor-app --discriminator 18 --secured-device-port 5560 --KVS /tmp/chip_kvs_requestor
```

#### In terminal 3:

**Commission the OTA Provider application**

```
out/chip-tool pairing onnetwork-long 0xDEADBEEF 20202021 22
```

**Commission the OTA Requestor application**

```
out/chip-tool pairing onnetwork-long 0x1234567890 20202021 18
```

**Issue the AnnounceOTAProvider command**

```
out/chip-tool otasoftwareupdaterequestor announce-ota-provider 0xDEADBEEF 0 0 0 0x1234567890 0
```

The OTA Requestor application with node ID 0x1234567890 will process this
command and send a QueryImage command to the OTA Provider with node ID
0xDEADBEEF, as specified in the `AnnounceOTAProvider` command.

### Trigger using DefaultOTAProviders attribute

If one or more provider locations have been written to the `DefaultOTAProviders`
attribute, this can be used to trigger a QueryImage command to a provider in the
attribute and start the OTA process.

#### In terminal 1:

**Build the OTA Provider application**

```
scripts/examples/gn_build_example.sh examples/ota-provider-app/linux/ out chip_config_network_layer_ble=false
```

**Run the OTA Provider application**

```
out/chip-ota-provider-app --discriminator 22 --KVS /tmp/chip_kvs_provider --filepath /tmp/test.bin
```

#### In terminal 2:

**Build the OTA Requestor application**

```
scripts/examples/gn_build_example.sh examples/ota-requestor-app/linux/ out chip_config_network_layer_ble=false
```

**Run the OTA Requestor application**

```
out/chip-ota-requestor-app --discriminator 18 --secured-device-port 5560 --KVS /tmp/chip_kvs_requestor --periodicQueryTimeout 60 --otaDownloadPath /tmp/test.bin
```

#### In terminal 3:

**Commission the OTA Provider application**

```
out/chip-tool pairing onnetwork-long 0xDEADBEEF 20202021 22
```

**Commission the OTA Requestor application**

```
out/chip-tool pairing onnetwork-long 0x1234567890 20202021 18
```

**Write to the DefaultOTAProviders attribute**

```
out/chip-tool otasoftwareupdaterequestor write default-ota-providers '[{"fabricIndex": 1, "providerNodeID": 3735928559, "endpoint": 0}]' 0x0000001234567890 0
```

Every 60 seconds from when the OTA Requestor application has launched, the OTA
Requestor application with node ID 0x1234567890 will send a QueryImage command
to the OTA Provider with node ID 0xDEADBEEF, as specified in the
`DefaultOTAProviders` attribute.

## DefaultOTAProviders attribute

The `DefaultOTAProviders` attribute represents a list of `ProviderLocation`
structs. Each entry in this list is a default OTA Provider per fabric. There can
not be more than one entry containing the same fabric.

To add more than one entry to the `DefaultOTAProviders` attribute, the OTA
Requestor app must be commissioned on multiple fabrics. This can be done as
specified below:

**Build and run the OTA Requestor application**

```
scripts/examples/gn_build_example.sh examples/ota-requestor-app/linux/ out chip_config_network_layer_ble=false
out/chip-ota-requestor-app --discriminator 18 --secured-device-port 5560 --KVS /tmp/chip_kvs_requestor --otaDownloadPath /tmp/test.bin
```

**Commission to the first fabric**

```
out/chip-tool pairing onnetwork-long 0x1234567890 20202021 18
```

**Open Basic Commissioning Window**

```
out/chip-tool administratorcommissioning open-basic-commissioning-window 600 0x1234567890 0 --timedInteractionTimeoutMs 600
```

**Commission to the second fabric**

```
out/chip-tool pairing onnetwork-long 0x858 20202021 18 --commissioner-name beta
```

For all operations, specify which fabric to use by passing in
`--commissioner-name`. The supported values are alpha, beta, and gamma. By
default, if none is supplied, alpha is used.

**Write/Read DefaultOTAProviders on first fabric**

```
out/chip-tool otasoftwareupdaterequestor write default-ota-providers '[{"fabricIndex": 1, "providerNodeID": 3735928559, "endpoint": 0}]' 0x0000001234567890 0
out/chip-tool otasoftwareupdaterequestor read default-ota-providers 0x1234567890 0
```

**Write/Read DefaultOTAProviders on second fabric**

```
out/chip-tool otasoftwareupdaterequestor write default-ota-providers '[{"fabricIndex": 2, "providerNodeID": 1, "endpoint": 0}]' 0x858 0 --commissioner-name beta
out/chip-tool otasoftwareupdaterequestor read default-ota-providers 0x858 0 --commissioner-name beta
```
