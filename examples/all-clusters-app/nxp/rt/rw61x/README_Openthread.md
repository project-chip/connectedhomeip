# Matter over Thread on RW612

## Hardware requirements

- `RD-RW612-BGA` board
- BLE/15.4 antenna

<a name="building"></a>

## Building

### Pre-build instructions
First instructions from [README.md 'Building section'][readme_building_section] should be followed.

[readme_building_section]: README.md#building

### Build instructions

-   Build the Openthread configuration with BLE commissioning.

```
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rw61x$ gn gen --args="chip_enable_openthread=true chip_inet_config_enable_ipv4=false chip_config_network_layer_ble=true is_sdk_package=true" out/debug
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rw61x$ ninja -C out/debug
```

The resulting output file can be found in out/debug/chip-rw61x-all-cluster-example.

## Flash Binaries
### Flashing and debugging RW612

To know how to flash and debug follow instructions from [README.md 'Flashing and debugging'][readme_flash_debug_section].

[readme_flash_debug_section]:README.md#flashing-and-debugging

## Raspberrypi Border Router setup

Instructions to start an openthread border router should be followed. In this section a mechanism to start the BR, without accessing the web interface, is described.

Start the docker BR image:

```
sudo docker run -it --rm --network host --privileged -v /dev/ttyACM0:/dev/radio connectedhomeip/otbr:sve2 --radio-url spinel+hdlc+uart:///dev/radio -B eth0
```

Get the docker ID of the BR image:
```
sudo docker container ls
```

Create a thread network on the border router (<container_id> should be replaced by the previously gotten BR docker ID):

```
sudo docker exec -it <container_id> sh -c "sudo ot-ctl dataset init new"; sudo docker exec -it <container_id> sh -c "sudo ot-ctl dataset channel 17"; sudo docker exec -it <container_id> sh -c "sudo ot-ctl dataset panid 0x1222"; sudo docker exec -it <container_id> sh -c "sudo ot-ctl dataset extpanid 1111111122222222"; sudo docker exec -it <container_id> sh -c "sudo ot-ctl dataset networkkey 00112233445566778899aabbccddeeaa"; sudo docker exec -it <container_id> sh -c "sudo ot-ctl dataset commit active"; sudo docker exec -it <container_id> sh -c "sudo ot-ctl ifconfig up"; sudo docker exec -it <container_id> sh -c "sudo ot-ctl thread start"; sudo docker exec -it <container_id> sh -c "sudo ot-ctl prefix add fd11:22::/64 pasor"; sudo docker exec -it <container_id> sh -c "sudo ot-ctl netdata register"
```

Get the dataset active of the thread network created :
```
sudo docker exec -it <container_id> sh -c "sudo ot-ctl dataset active -x"
```


## Testing the all cluster app example (with BLE commissioning support) - default configuration

The pairing "ble-thread" feature must be used and instructions from [README.md 'Testing the example'][readme_test_example_section] should be followed.

[readme_test_example_section]:README.md#testing-the-example

## Testing the all cluster app example (without BLE commissioning support) - only for testing purpose

For such test, having the Matter CLI is mandatory, instructions from [README.md 'Testing the all-clusters application with Matter CLI enabled'][readme_test_with_matter_cli_section] should be followed.

[readme_test_with_matter_cli_section]:README.md#testing-the-all-clusters-application-with-matter-cli-enabled

Then using the Matter CLI below commands to join an existing thread network should be entered, `networkkey` and `panid` should be changed depending on thread network configurations:

```
otcli networkkey 00112233445566778899aabbccddeeff
otcli panid 0x1234
otcli channel 17
otcli commit active
otcli ifconfig up
otcli thread start
```

Note: The pairing "onnetwork" feature should be used as the pairing/commissioning method.
