# ota-requestor-app (Linux)

This is a reference application that is both a server for the OTA Requestor
Cluster, as well as a client of the OTA Provider Cluster. It can initiate a
software update with a given OTA Provider node, and download a file.

## Build

Suggest doing the following:

```
scripts/examples/gn_build_example.sh examples/ota-requestor-app/linux out/debug chip_config_network_layer_ble=false
```

## Usage

In addition to the general options available to all Linux applications, the
following command line options are available for the OTA Requestor application.
Note that these options are for testing purposes.

| Command Line Option                                      | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| -------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| -a, --autoApplyImage                                     | If supplied, apply the image immediately after download. Otherwise, the OTA update is complete after image download.                                                                                                                                                                                                                                                                                                                                                                                              |
| -c, --requestorCanConsent \<true \| false\>              | Value for the RequestorCanConsent field in the QueryImage command. If not supplied, the value is determined by the driver.                                                                                                                                                                                                                                                                                                                                                                                        |
| -d, --disableNotifyUpdateApplied                         | If supplied, disable sending of the NotifyUpdateApplied command. Otherwise, after successfully loading into the updated image, send the NotifyUpdateApplied command.                                                                                                                                                                                                                                                                                                                                              |
| -f, --otaDownloadPath \<file path\>                      | If supplied, the OTA image is downloaded to the given fully-qualified file-path. Otherwise, the default location for the downloaded image is at /tmp/test.bin                                                                                                                                                                                                                                                                                                                                                     |
| -p, --periodicQueryTimeout \<time in seconds\>           | The periodic time interval to wait before attempting to query a provider from the default OTA provider list. If none or zero is supplied, the value is determined by the driver.                                                                                                                                                                                                                                                                                                                                  |
| -u, --userConsentState \<granted \| denied \| deferred\> | Represents the current user consent status when the OTA Requestor is acting as a user consent delegate. This value is only applicable if value of the UserConsentNeeded field in the QueryImageResponse is set to true. This value is used for the first attempt to download. For all subsequent queries, the value of granted will be used.<li> granted: Authorize OTA requestor to download an OTA image <li> denied: Forbid OTA requestor to download an OTA image <li> deferred: Defer obtaining user consent |
| -w, --watchdogTimeout \<time in seconds\>                | Maximum amount of time allowed for an OTA download before the process is cancelled and state reset to idle. If none or zero is supplied, the value is determined by the driver.                                                                                                                                                                                                                                                                                                                                   |

## Software Image Version

The current software version of the OTA Requestor application is defined by
`CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION` in
[CHIPProjectConfig.h](https://github.com/project-chip/connectedhomeip/blob/master/config/standalone/CHIPProjectConfig.h).
This value can be confirmed by reading the `SoftwareVersion` attribute of the
Basic Information cluster:

```
out/chip-tool basic read software-version 0x1234567890 0
```

### Validations

On receiving the QueryImageResponse from the OTA Provider application, the OTA
Requestor application will verify that the software version specified in the
`SoftwareVersion` field of the response contains a value newer than the current
running version. If the update supplied does not pass this version check, the
following log message should be expected, indicating the update will not
proceed:

```
[1648233572232] [48462:7613274] CHIP: [SWU] Available update version 1 is <= current version 1, update ignored
```

If the OTA update progresses to downloading, the process will abort if the
software image
[header](https://github.com/project-chip/connectedhomeip/tree/master/examples/ota-provider-app/linux#software-image-header)
is missing. The following log messages should be expected:

```
[1648246917398] [71786:7874994] CHIP: [SWU] Image does not contain a valid header
[1648246917399] [71786:7874994] CHIP: [BDX] TransferSession error
```

On booting into the new image, if the running version does not match the version
specified in the QueryImageResponse, the following log message should be
expected:

```
[1648244159295] [58606:7774255] CHIP: [SWU] Failed to confirm image: ../../examples/ota-requestor-app/linux/third_party/connectedhomeip/src/platform/Linux/OTAImageProcessorImpl.cpp:110: CHIP Error 0x00000003: Incorrect state
```

This message serves as a warning that the new image is not expected. However,
the OTA Requestor application will recover gracefully and return to a state
where another OTA update may be initiated.

### Generate Images

To validate booting into a newer OTA Requestor image on the Linux platform, the
following must be performed:

1. Modify `CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION` to a value greater than
   the current running version
2. [Build](https://github.com/project-chip/connectedhomeip/blob/master/examples/ota-requestor-app/linux/README.md#build)
   an OTA Requestor application with the new version
3. Using the executable created in step 2, generate an image with a software
   image
   [header](https://github.com/project-chip/connectedhomeip/tree/master/examples/ota-provider-app/linux#software-image-header)
   that matches the version in step 1
4. Use this generated binary when supplying the image file to the OTA Provider
   application
5. Launch the original application (an OTA Requestor application generated prior
   to step 1) with the command line option `--autoApplyImage`

## Common Instructions

The Linux
[ota-provider-app](https://github.com/project-chip/connectedhomeip/tree/master/examples/ota-provider-app/linux)
can be used together with this reference application to test OTA features. Here
are some common instructions for building and commissioning the applications.

### OTA Provider application

#### Build the OTA Provider application

Follow instructions
[here](https://github.com/project-chip/connectedhomeip/tree/master/examples/ota-provider-app/linux#build)

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

#### Build the OTA Requestor application

Follow instructions
[here](https://github.com/project-chip/connectedhomeip/tree/master/examples/ota-requestor-app/linux#build)

#### Run the OTA Requestor application:

```
out/chip-ota-requestor-app --discriminator ${REQUESTOR_LONG_DISCRIMINATOR} --secured-device-port ${REQUESTOR_UDP_PORT} --KVS ${KVS_STORE_LOCATION} --periodicQueryTimeout ${TIME_IN_SECONDS} --autoApplyImage

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
-   `--autoApplyImage` is supplied to indicate the image should be immediately
    applied after download. If not supplied, the OTA update is complete after
    image download.

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
command will be denied. Logs similar to the following may be observed on the OTA
Provider application:

```
[1648244658368] [59686:7786439] CHIP: [DMG] AccessControl: checking f=1 a=c s=0x000000000000FACE t= c=0x0000_0029 e=0 p=o
[1648244658368] [59686:7786439] CHIP: [DMG] AccessControl: denied
```

If this is encountered, follow instructions
[here](https://github.com/project-chip/connectedhomeip/tree/master/examples/ota-provider-app/linux#access-control-requirements)
to install. Note that this only needs to be performed once. There is no need to
write this ACL entry again unless the KVS store has been removed.

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
out/chip-ota-provider-app --discriminator 22 --secured-device-port 5565 --KVS /tmp/chip_kvs_provider --filepath /tmp/ota-image.bin
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
out/chip-tool otasoftwareupdaterequestor announce-otaprovider 0xDEADBEEF 0 0 0 0x1234567890 0
```

The OTA Requestor application with node ID 0x1234567890 will process this
command and send a QueryImage command to the OTA Provider with node ID
`0xDEADBEEF`, as specified in the `AnnounceOTAProvider` command.

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
out/chip-ota-provider-app --discriminator 22 --secured-device-port 5565 --KVS /tmp/chip_kvs_provider --filepath /tmp/ota-image.bin
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
out/chip-tool otasoftwareupdaterequestor write default-otaproviders '[{"providerNodeID": 3735928559, "endpoint": 0}]' 0x1234567890 0
```

Every 60 seconds from when the OTA Requestor application has launched, the OTA
Requestor application with node ID 0x1234567890 will send a QueryImage command
to the OTA Provider with node ID `0xDEADBEEF`, as specified in the
`DefaultOTAProviders` attribute.

## DefaultOTAProviders attribute

The `DefaultOTAProviders` attribute represents a list of `ProviderLocation`
structs. Each entry in this list is a default OTA Provider per fabric. There can
not be more than one entry containing the same fabric.

To add more than one entry to the `DefaultOTAProviders` attribute, the OTA
Requestor app must be commissioned into multiple fabrics. At least one OTA
Provider app should be commissioned into each corresponding fabric that the OTA
Requestor app had been commissioned into.

The following example has two OTA Provider apps, each commissioned into a
different fabric (alpha and beta) and one OTA Requestor app commissioned into
both alpha and beta fabrics.

### In terminal 1:

**Build the OTA Provider application**

```
scripts/examples/gn_build_example.sh examples/ota-provider-app/linux/ out chip_config_network_layer_ble=false
```

**Run the first OTA Provider application**

```
out/chip-ota-provider-app --discriminator 22 --secured-device-port 5565 --KVS /tmp/chip_kvs_provider --filepath /tmp/ota-image.bin
```

### In terminal 2:

**Run the second OTA Provider application**

```
out/chip-ota-provider-app --discriminator 23 --secured-device-port 5566 --KVS /tmp/chip_kvs_provider2 --filepath /tmp/ota-image2.bin
```

### In terminal 3:

**Build the OTA Requestor application**

```
scripts/examples/gn_build_example.sh examples/ota-requestor-app/linux/ out chip_config_network_layer_ble=false
```

**Run the OTA Requestor application**

```
out/chip-ota-requestor-app --discriminator 18 --secured-device-port 5560 --KVS /tmp/chip_kvs_requestor --periodicQueryTimeout 10
```

### In terminal 4:

**Commission the first OTA Provider into the first fabric (alpha)**

```
out/chip-tool pairing onnetwork-long 0xC0FFEE 20202021 22
```

**Commission the second OTA Provider into the second fabric (beta)**

```
out/chip-tool pairing onnetwork-long 0xB0BA 20202021 23 --commissioner-name beta
```

**Commission the OTA Requestor application into the first fabric (alpha)**

```
out/chip-tool pairing onnetwork-long 0xDEB 20202021 18
```

**Open Basic Commissioning Window for the OTA Requestor application**

```
out/chip-tool administratorcommissioning open-basic-commissioning-window 600 0xDEB 0 --timedInteractionTimeoutMs 600
```

**Commission the OTA Requestor application into the second fabric (beta)**

```
out/chip-tool pairing onnetwork-long 0xB0B 20202021 18 --commissioner-name beta
```

**Write/Read DefaultOTAProviders on the first fabric (alpha)**

```
out/chip-tool otasoftwareupdaterequestor write default-otaproviders '[{"providerNodeID": 12648430, "endpoint": 0}]' 0xDEB 0
out/chip-tool otasoftwareupdaterequestor read default-otaproviders 0xDEB 0
```

**Write/Read DefaultOTAProviders on second fabric (beta)**

```
out/chip-tool otasoftwareupdaterequestor write default-otaproviders '[{"providerNodeID": 45242, "endpoint": 0}]' 0xB0B 0 --commissioner-name beta
out/chip-tool otasoftwareupdaterequestor read default-otaproviders 0xB0B 0 --commissioner-name beta
```

**Write ACL for the first OTA Provider application**

```
out/chip-tool accesscontrol write acl '[{"privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"privilege": 3, "authMode": 2, "subjects": null, "targets": [{"cluster": 41, "endpoint": null, "deviceType": null}]}]' 0xC0FFEE 0
```

**Write ACL for the second OTA Provider application**

```
out/chip-tool accesscontrol write acl '[{"privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"privilege": 3, "authMode": 2, "subjects": null, "targets": [{"cluster": 41, "endpoint": null, "deviceType": null}]}]' 0xB0BA 0 --commissioner-name beta
```

NOTE: For all operations, specify which fabric to use by passing in
`--commissioner-name`. The supported values are alpha, beta, and gamma. By
default, if none is supplied, alpha is used.
