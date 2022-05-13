# ota-provider-app

This is a reference application that implements an example of an OTA Provider
Cluster Server.

## Build

Suggest doing the following:

```
scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/debug chip_config_network_layer_ble=false
```

## Usage

| Command Line Options                                                     | Description                                                                                                                                                                                                                                                                                                                                                                                                                            |
| ------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| -a, --applyUpdateAction \<proceed \| awaitNextAction \| discontinue\>    | Value for the Action field in the first ApplyUpdateResponse.<br>For all subsequent responses, the value of proceed will be used.                                                                                                                                                                                                                                                                                                       |
| -c, --userConsentNeeded                                                  | If supplied, value of the UserConsentNeeded field in the QueryImageResponse is set to true. This is only applicable if value of the RequestorCanConsent field in QueryImage Command is true.<br>Otherwise, value of the UserConsentNeeded field is false.                                                                                                                                                                              |
| -f, --filepath \<file path\>                                             | Path to a file containing an OTA image                                                                                                                                                                                                                                                                                                                                                                                                 |
| -i, --imageUri \<uri\>                                                   | Value for the ImageURI field in the QueryImageResponse. If none is supplied, a valid URI is generated.                                                                                                                                                                                                                                                                                                                                 |
| -o, --otaImageList \<file path\>                                         | Path to a file containing a list of OTA images                                                                                                                                                                                                                                                                                                                                                                                         |
| -p, --delayedApplyActionTimeSec \<time in seconds\>                      | Value for the DelayedActionTime field in the first ApplyUpdateResponse.<br>For all subsequent responses, the value of zero will be used.                                                                                                                                                                                                                                                                                               |
| -q, --queryImageStatus \<updateAvailable \| busy \| updateNotAvailable\> | Value for the Status field in the first QueryImageResponse.<br>For all subsequent responses, the value of updateAvailable will be used.                                                                                                                                                                                                                                                                                                |
| -t, --delayedQueryActionTimeSec <time>                                   | Value for the DelayedActionTime field in the first QueryImageResponse.<br>For all subsequent responses, the value of zero will be used.                                                                                                                                                                                                                                                                                                |
| -u, --userConsentState \<granted \| denied \| deferred\>                 | The user consent state for the first QueryImageResponse. For all subsequent responses, the value of granted will be used.<br>Note that --queryImageStatus overrides this option.<li> granted: Status field in the first QueryImageResponse is set to updateAvailable <li> denied: Status field in the first QueryImageResponse is set to updateNotAvailable <li> deferred: Status field in the first QueryImageResponse is set to busy |
| -x, --ignoreQueryImage \<ignore count\>                                  | The number of times to ignore the QueryImage Command and not send a response                                                                                                                                                                                                                                                                                                                                                           |
| -y, --ignoreApplyUpdate \<ignore count\>                                 | The number of times to ignore the ApplyUpdate Request and not send a response                                                                                                                                                                                                                                                                                                                                                          |
| -P, --pollInterval <milliseconds>                                        | Poll interval for the BDX transfer.                                                                                                                                                                                                                                                                                                                                                                                                    |

**Using `--filepath` and `--otaImageList`**

-   The two options cannot be supplied together
-   At least one option must be supplied
-   If `--filepath` is supplied, the application will automatically serve that
    file to the OTA Requestor
-   If `--otaImageList` is supplied, the application will parse the JSON file
    and extract all required data. The most recent/valid software version will
    be selected and the corresponding OTA file will be sent to the OTA Requestor
-   The SoftwareVersion and SoftwareVersionString sent in the QueryImageResponse
    is derived from the OTA image header. Please note that if the version in the
    `--otaImageList` JSON file does not match that in the image header, the
    application will terminate.

An example of the `--otaImageList` file contents:

```
{ "foo": 1, // ignored by parser
  "deviceSoftwareVersionModel":
  [
      { "vendorId": 1, "productId": 1, "softwareVersion": 10, "softwareVersionString": "1.0.0", "cDVersionNumber": 18, "softwareVersionValid": true, "minApplicableSoftwareVersion": 0, "maxApplicableSoftwareVersion": 100, "otaURL": "/tmp/ota_v10.bin" },
      { "vendorId": 1, "productId": 1, "softwareVersion": 20, "softwareVersionString": "1.0.1", "cDVersionNumber": 18, "softwareVersionValid": false, "minApplicableSoftwareVersion": 0, "maxApplicableSoftwareVersion": 100, "otaURL": "/tmp/ota_v20.bin" },
      { "vendorId": 1, "productId": 1, "softwareVersion": 30, "softwareVersionString": "1.0.2", "cDVersionNumber": 18, "softwareVersionValid": true, "minApplicableSoftwareVersion": 0, "maxApplicableSoftwareVersion": 100, "otaURL": "/tmp/ota_v30.bin" },
      { "vendorId": 1, "productId": 1, "softwareVersion": 40, "softwareVersionString": "1.1.0", "cDVersionNumber": 18, "softwareVersionValid": true, "minApplicableSoftwareVersion": 0, "maxApplicableSoftwareVersion": 100, "otaURL": "/tmp/ota_v40.bin" },
      { "vendorId": 1, "productId": 1, "softwareVersion": 50, "softwareVersionString": "1.1.1", "cDVersionNumber": 18, "softwareVersionValid": false, "minApplicableSoftwareVersion": 0, "maxApplicableSoftwareVersion": 100, "otaURL": "/tmp/ota_v50.bin" }
  ]
}
```

## Software Image Header

All Matter software images must contain a header as defined in section 11.21.1
of the specification. The
[ota_image_tool](https://github.com/project-chip/connectedhomeip/blob/master/src/app/ota_image_tool.py)
is available for generating the required header on a software image.

All images supplied to the OTA Provider application (via `--filepath` or
`--otaImageList`) must contain the software image header. The OTA Provider
application will use the software version specified in the header to set the
`SoftwareVersion` field of the QueryImageResponse. For instance, if the image
supplied represents an image with software version 2, the tool can be used as
follows:

```
src/app/ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn 2 -vs "2.0" -da sha256 firmware.bin firmware.ota
```

Please see this
[section](https://github.com/project-chip/connectedhomeip/tree/master/examples/ota-requestor-app/linux#generate-images)
for information on building an OTA Requestor application with a specific
software version.

## Access Control Requirements

Commissioner or Administrator SHOULD install necessary ACL entries at
commissioning time or later to enable processing of QueryImage commands from OTA
Requestors on their fabric, otherwise that OTA Provider will not be usable by
OTA Requestors.

Since the ACL attribute contains a list of ACL entries, writing of the attribute
should not just contain the values for the new entry. Any existing entries
should be read and included as part of the write. Below is an example of how to
write the ACL attribute with two entries:

```
out/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": [{"cluster": 41, "endpoint": null, "deviceType": null}]}]' 0xDEADBEEF 0
```

-   Entry 1: This is the original entry created as part of commissioning which
    grants administer privilege to the node ID 112233 (default controller node
    ID) for all clusters on every endpoint
-   Entry 2: This is the new entry being added which grants operate privileges
    to all nodes for the OTA Provider cluster (0x0029) on every endpoint

In the example above, the provider is on fabric index 1 with provider node ID
being `0xDEADBEEF` on endpoint 0.

## Current Limitations

-   Synchronous BDX transfer only
-   Does not check VID/PID
-   Only one transfer at a time (does not check incoming `UpdateTokens`)
