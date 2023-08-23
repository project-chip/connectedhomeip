# Matter Telink Shell Example Application

You can use this example as a reference for creating your own application.

![Telink B91 EVK](http://wiki.telink-semi.cn/wiki/assets/Hardware/B91_Generic_Starter_Kit_Hardware_Guide/connection_chart.png)

## Build and flash

1. Pull docker image from repository:

    ```bash
    $ docker pull ghcr.io/project-chip/chip-build-telink:10
    ```

2. Run docker container:

    ```bash
    $ docker run -it --rm -v ${CHIP_BASE}:/root/chip -v /dev/bus/usb:/dev/bus/usb --device-cgroup-rule "c 189:* rmw" ghcr.io/project-chip/chip-build-telink:10
    ```

    here `${CHIP_BASE}` is directory which contains CHIP repo files **!!!Pay
    attention that OUTPUT_DIR should contains ABSOLUTE path to output dir**

3. Activate the build environment:

    ```bash
    $ source ./scripts/activate.sh
    ```

4. In the example dir run (replace _<build_target>_ with your board name, for
   example, `tlsr9518adk80d` or `tlsr9528a`):

    ```bash
    $ west build -b <build_target>
    ```

5. Flash binary:

    ```
    $ west flash --erase
    ```

## Usage

### UART

To get output from device, connect UART to following pins:

| Name | Pin                           |
| :--: | :---------------------------- |
|  RX  | PB3 (pin 17 of J34 connector) |
|  TX  | PB2 (pin 16 of J34 connector) |
| GND  | GND                           |
