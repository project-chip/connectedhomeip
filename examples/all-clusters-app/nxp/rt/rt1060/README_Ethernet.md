# RT1060 All-cluster Application for Matter over Ethernet - WARNING NOT SUPPORTED

<a name="building"></a>

## Building

First instructions from [README.md 'Building section'][readme_building_section] should be followed.

[readme_building_section]: README.md#building

-   Build the ethernet configuration for MIMXRT1060-EVKB board:

```
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rt1060$ gn gen --args="chip_enable_ethernet=true chip_config_network_layer_ble=false chip_inet_config_enable_ipv4=false" out/debug
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rt1060$ ninja -C out/debug
```

-   Build the ethernet configuration for EVK-MIMXRT1060 board:

```
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rt1060$ gn gen --args="chip_enable_ethernet=true chip_config_network_layer_ble=false chip_inet_config_enable_ipv4=false evkname=\"evkmimxrt1060\"" out/debug
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rt1060$ ninja -C out/debug
```

The resulting output file can be found in out/debug/chip-rt1060-all-cluster-example

<a name="flashdebug"></a>

## Flashing and debugging

To know how to flash and debug follow instructions from [README.md 'Flashing and debugging'][readme_flash_debug_section].

[readme_flash_debug_section]:README.md#flashdebug

## Testing the example

Follow instructions from [README.md 'Testing the example'][readme_test_example_section].

[readme_test_example_section]:README.md#testing-the-example
