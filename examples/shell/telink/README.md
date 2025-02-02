# Matter Telink Shell Example Application

You can use this example as a reference for creating your own application.

![Telink B91 EVK](http://wiki.telink-semi.cn/wiki/assets/Hardware/B91_Generic_Starter_Kit_Hardware_Guide/connection_chart.png)

## Supported devices

The example supports building and running on the following devices:

| Board/SoC                                                                                                                                                              | Build target                                                  | Zephyr Board Info                                                                                              |
| ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------- |
| [B91](https://wiki.telink-semi.cn/wiki/Hardware/B91_Generic_Starter_Kit_Hardware_Guide) [TLSR9518ADK80D](https://wiki.telink-semi.cn/wiki/chip-series/TLSR951x-Series) | `tlsr9518adk80d`, `tlsr9518adk80d-mars`, `tlsr9518adk80d-usb` | [TLSR9518ADK80D](https://github.com/telink-semi/zephyr/blob/develop/boards/riscv/tlsr9518adk80d/doc/index.rst) |
| [B92](https://wiki.telink-semi.cn/wiki/Hardware/B92_Generic_Starter_Kit_Hardware_Guide) [TLSR9528A](https://wiki.telink-semi.cn/wiki/chip-series/TLSR952x-Series)      | `tlsr9528a`, `tlsr9528a_retention`                            | [TLSR9528A](https://github.com/telink-semi/zephyr/blob/develop/boards/riscv/tlsr9528a/doc/index.rst)           |
| [W91](https://wiki.telink-semi.cn/wiki/Hardware/W91_Generic_Starter_Kit_Hardware_Guide) [TLSR9118BDK40D](https://wiki.telink-semi.cn/wiki/chip-series/TLSR911x-Series) | `tlsr9118bdk40d`                                              | [TLSR9118BDK40D](https://github.com/telink-semi/zephyr/blob/develop/boards/riscv/tlsr9118bdk40d/doc/index.rst) |

## Build and flash

1. Run the Docker container:

    ```bash
    $ docker run -it --rm -v $PWD:/host -w /host ghcr.io/project-chip/chip-build-telink:$(wget -q -O - https://raw.githubusercontent.com/project-chip/connectedhomeip/master/.github/workflows/examples-telink.yaml 2> /dev/null | grep chip-build-telink | awk -F: '{print $NF}')
    ```

    You can find the compatible Docker image version in the file:

    ```bash
    $ .github/workflows/examples-telink.yaml
    ```

2. Activate the build environment:

    ```bash
    $ source ./scripts/activate.sh -p all,telink
    ```

3. Build the example (replace _<build_target>_ with your board name, see
   [Supported devices](#supported-devices)):

    ```bash
    $ west build -b <build_target>
    ```

    Also use key `-DFLASH_SIZE`, if your board has memory size different from 2
    MB, for example, `-DFLASH_SIZE=1m` or `-DFLASH_SIZE=4m`:

    ```bash
    $ west build -b <build_target> -- -DFLASH_SIZE=4m
    ```

    You can find the target built file called **_zephyr.bin_** under the
    **_build/zephyr_** directory.

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

Baud rate: 115200 bits/s
