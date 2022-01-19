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
./chip-ota-requestor-app -d ${REQUESTOR_LONG_DISCRIMINATOR} -u ${REQUESTOR_UDP_PORT} -n ${PROVIDER_NODE_ID} -f ${PROVIDER_FABRIC_INDEX} -q ${DELAY_QUERY_SECONDS}
```

-   `{REQUESTOR_LONG_DISCRIMINATOR}` is the long discriminator specified for the
    ota-requestor-app for commissioning discovery
-   `{REQUESTOR_UDP_PORT}` is the UDP port that the ota-requestor-app listens on
    for secure connections
-   `${PROVIDER_IP_ADDRESS}` is the IP address of the ota-provider-app that has
    been resolved manually
-   `${PROVIDER_NODE_ID}` is the node ID of the ota-provider-app; this is a Test
    Mode parameter and should not be used in most scenarios
-   `${PROVIDER_FABRIC_INDEX}` is the fabric index of the ota-provider-app; this
    is a Test Mode parameter and should not be used in most scenarios
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

```
./chip-tool otasoftwareupdaterequestor announce-ota-provider ${PROVIDER_NODE_ID} 0 0 0 ${REQUESTOR_NODE_ID} 0
```

-   `${PROVIDER_NODE_ID}` is the node ID of the ota-provider-app assigned to it
    during the pairing step above
-   `${REQUESTOR_NODE_ID}` is the node ID of the ota-requestor-app assigned to
    it during the pairing step above

## Note

When the Provider, Requestor and chip-tool are run on the same Linux node the
user must issue `rm -r /tmp/chip_*` before starting the Provider and
`rm /tmp/chip_kvs` before starting the Requestor. These commands reset the
shared Key Value Store to a consistent state.

## Example

Building:

```
scripts/examples/gn_build_example.sh examples/chip-tool out/ chip_config_network_layer_ble=false && scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/debug chip_config_network_layer_ble=false  && scripts/examples/gn_build_example.sh examples/ota-requestor-app/linux out/debug chip_config_network_layer_ble=false
```

Running (in separate terminals as described above):

```
rm -r /tmp/chip_*
./out/debug/chip-ota-provider-app -f /tmp/ota.txt
./out/chip-tool pairing onnetwork 1 20202021
rm /tmp/chip_kvs
./out/debug/chip-ota-requestor-app -u 5560 -d 42
./out/chip-tool pairing onnetwork-long 2 20202021 42
./out/chip-tool otasoftwareupdaterequestor announce-ota-provider 1 0 0 0 2 0
```

## Current Features / Limitations

### Features

-   Code for running a full BDX download exists in BDX
-   Sends QueryImage command
-   Downloads a file over BDX served by an OTA Provider server
-   Supports various command line configurations

### Limitations

-   Stores the downloaded file in the directory this reference app is launched
    from
