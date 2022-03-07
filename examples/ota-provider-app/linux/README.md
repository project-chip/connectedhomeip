# ota-provider-app

This is a reference application that implements an example of an OTA Provider
Cluster Server.

## Building

Suggest doing the following:

```
scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/debug chip_config_network_layer_ble=false
```

## Usage

| Command Line Options                                                  | Description                                                                                                                                                                                                                                                                                                                                                                           |
| --------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| -f/--filepath <file>                                                  | Path to a file containing an OTA image                                                                                                                                                                                                                                                                                                                                                |
| -o/--otaImageList <file>                                              | Path to a file containing a list of OTA images                                                                                                                                                                                                                                                                                                                                        |
| -q/--queryImageStatus <updateAvailable \| busy \| updateNotAvailable> | Value for the Status field in the QueryImageResponse                                                                                                                                                                                                                                                                                                                                  |
| -x/--ignoreQueryImage <num_times_to_ignore>                           | The number of times to ignore the QueryImage Command and not send a response                                                                                                                                                                                                                                                                                                          |  |
| -y/--ignoreApplyUpdate <num_times_to_ignore>                          | The number of times to ignore the ApplyUpdate Request and not send a response                                                                                                                                                                                                                                                                                                         |
| -a/--applyUpdateAction <proceed \| awaitNextAction \| discontinue>    | Value for the Action field in the ApplyUpdateResponse                                                                                                                                                                                                                                                                                                                                 |
| -t/--delayedActionTimeSec <time>                                      | Value in seconds for the DelayedActionTime field in the QueryImageResponse and ApplyUpdateResponse                                                                                                                                                                                                                                                                                    |
| -u/--userConsentState <granted \| denied \| deferred>                 | Current user consent state which results in various values for Status field in QueryImageResponse <br> Note that -q/--queryImageStatus overrides this option <li> granted: Status field in QueryImageResponse is set to updateAvailable <li> denied: Status field in QueryImageResponse is set to updateNotAvailable <li> deferred: Status field in QueryImageResponse is set to busy |
| -s/--softwareVersion <version>                                        | Value for the SoftwareVersion field in the QueryImageResponse <br> Note that -o/--otaImageList overrides this option                                                                                                                                                                                                                                                                  |
| -S/--softwareVersionStr <version string>                              | Value for the SoftwareVersionString field in the QueryImageResponse <br> Note that -o/--otaImageList overrides this option                                                                                                                                                                                                                                                            |
| -c/--UserConsentNeeded                                                | If provided, and value of RequestorCanConsent field in QueryImage Command is true, <br> then value of UserConsentNeeded field in the QueryImageResponse is set to true. <br> Else, value of UserConsentNeeded is false.                                                                                                                                                               |

**Using `--filepath` and `--otaImageList`**

-   The two options cannot be supplied together
-   If neither option is supplied, the application will respond with
    `NotAvailable` status
-   If `--filepath` is supplied, the application will automatically serve that
    file to the OTA Requestor (SoftwareVersion will be requester software
    version + 1)
-   If `--otaImageList` is supplied, the application will parse the JSON file
    and extract all required data. The most recent/valid software version will
    be selected and the corresponding OTA file will be sent to the OTA Requestor

An example of the `--otaImageList` file contents:

```
{ "foo": 1, // ignored by parser
  "deviceSoftwareVersionModel":
  [
      { "vendorId": 1, "productId": 1, "softwareVersion": 10, "softwareVersionString": "1.0.0", "cDVersionNumber": 18, "softwareVersionValid": true, "minApplicableSoftwareVersion": 0, "maxApplicableSoftwareVersion": 100, "otaURL": "/tmp/ota.txt" },
      { "vendorId": 1, "productId": 1, "softwareVersion": 20, "softwareVersionString": "1.0.1", "cDVersionNumber": 18, "softwareVersionValid": false, "minApplicableSoftwareVersion": 0, "maxApplicableSoftwareVersion": 100, "otaURL": "/tmp/ota.txt" },
      { "vendorId": 1, "productId": 1, "softwareVersion": 30, "softwareVersionString": "1.0.2", "cDVersionNumber": 18, "softwareVersionValid": true, "minApplicableSoftwareVersion": 0, "maxApplicableSoftwareVersion": 100, "otaURL": "/tmp/ota.txt" },
      { "vendorId": 1, "productId": 1, "softwareVersion": 40, "softwareVersionString": "1.1.0", "cDVersionNumber": 18, "softwareVersionValid": true, "minApplicableSoftwareVersion": 0, "maxApplicableSoftwareVersion": 100, "otaURL": "/tmp/ota.txt" },
      { "vendorId": 1, "productId": 1, "softwareVersion": 50, "softwareVersionString": "1.1.1", "cDVersionNumber": 18, "softwareVersionValid": false, "minApplicableSoftwareVersion": 0, "maxApplicableSoftwareVersion": 100, "otaURL": "/tmp/ota.txt" }
  ]
}
```

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
being 0xDEADBEEF on endpoint 0.

## Current Limitations

-   Synchronous BDX transfer only
-   Does not check VID/PID
-   No configuration for `AwaitNextAction`
-   Only one transfer at a time (does not check incoming `UpdateTokens`)
