# ota-requestor-app (ESP32)
A prototype application that demonstrates device OTA requester.

For now, this application rely on the [Linux OTA Providor app](../linux)

### Setup Linux OTA Provider
- Build the linux OTA Provider
```
# Change to chip top level directory
$ cd ../../../ 
$ scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/debug chip_config_network_layer_ble=false
```
- Build CHIP Tool
```
$ scripts/examples/gn_build_example.sh examples/chip-tool out/debug
```
- In terminal 1 run linux ota provider. One can  build the [hello world example](https://github.com/espressif/esp-idf/tree/master/examples/get-started/hello_world) or use one from [here](https://shubhamdp.github.io/esp_ota/esp32/hello-world.bin)
```
$ ./out/debug/chip-ota-provider-app -f hello-world.bin
```
- In terminal 2 run the CHIP tool to provision OTA provider
```
$ ./out/debug/chip-tool pairing 12345 20202021
```

Now we have a OTA provider ready for use

### Building the Example Application
- Assuming that esp-idf is and chip environment is already set up if not please check [all-clusters-examples/esp32](../../all-clusters-app/esp32) this example.
- Build the Application
```
$ idf.py build
```
- Flash application
```
$ idf.py -p <SerialPort> flash monitor
```
- Provision OTA Requester
```
./out/debug/chip-tool pairing ble-wifi 12346 <ssid> <passphrase> 0 20202021 3840
```
- After commissioning is successful, query for OTA image. Head over to ESP32 console and fire the following command. This command start the OTA image tranfer in 10 seconds.
```
QueryImage <OtaProviderIpAddress> <OtaProviderNodeId>
```
- Once transfer is complete it applies OTA and boots up from another partition.

## Features
- Code for running a full BDX download exists in BDX
- Sends QueryImage command using command line
- Downloads a file over BDX served by an OTA Provider server

## Limitations
- Do not support ApplyUpdateRequest command
- Needs chip-tool to commission the OTA Provider device first because the Node ID and IP Address of the OTA Provider must be supplied to this reference application
- Does not verify QueryImageResponse message contents or status
- Does not support AnnounceOTAProvider command or OTA Requestor attributes
