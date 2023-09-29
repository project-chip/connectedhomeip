# Matter Telink Shell Example Application

You can use this example as a reference for creating your own application.

![Telink B91 EVK](http://wiki.telink-semi.cn/wiki/assets/Hardware/B91_Generic_Starter_Kit_Hardware_Guide/connection_chart.png)

## Build and flash

1. Run the Docker container:

    ```bash
    $ docker run -it --rm -v $PWD:/host -w /host ghcr.io/project-chip/chip-build-telink:$(wget -q -O - https://raw.githubusercontent.com/project-chip/connectedhomeip/master/.github/workflows/examples-telink.yaml 2> /dev/null | grep chip-build-telink | awk -F: '{print $NF}')
    ```

    Compatible docker image version can be found in next file:

    ```bash
    $ .github/workflows/examples-telink.yaml
    ```

2. Activate the build environment:

    ```bash
    $ source ./scripts/activate.sh
    ```

3. In the example dir run (replace _<build_target>_ with your board name, for
   example, `tlsr9518adk80d` or `tlsr9528a`):

    ```bash
    $ west build -b <build_target>
    ```

4. Flash binary:

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
