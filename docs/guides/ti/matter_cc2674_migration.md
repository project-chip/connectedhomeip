# Running Matter Examples on the TI SimpleLink CC2674P10 and CC2674R10

The existing Matter project examples are based on LP_EM_CC1354P10_6. If using
the CC2674P10, the following migration steps are required. Developers are
strongly encouraged to start with a `cc13x4_26x4` example and migrate the
project accordingly. Example projects can be found in the following location:
`matter/examples/[application]/cc13x4_26x4`

## Dependencies

The following must be installed on your system before proceeding:

-   [SysConfig](https://www.ti.com/tool/SYSCONFIG) v1.16.2 or later

## Matter source code changes

The following are changes to the Matter source code files which should be
applied to convert a `matter/examples/[application]/cc13x4_26x4` project to the
CC2674P10 device

-   `examples/[application]/cc13x4_26x4/args.gni`, modify/add the following
    defines for the CC2674
    -   `ti_simplelink_board = "CC2674"`
    -   `ti_simplelink_device = "CC2674P10RGZ"`
-   `third_party/ti_simplelink_sdk/repo_cc13xx_cc26xx/source/ti/common/flash/no_rtos/extFlash/bsp.h`,
    modify the SPI GPIO pins to the value below:

```
#define BSP_IOID_FLASH_CS       IOID_20
#define BSP_SPI_MOSI            IOID_9
#define BSP_SPI_MISO            IOID_8
#define BSP_SPI_CLK_FLASH       IOID_10

```

The GPIO pin values for SPI will need to be adjusted based on your design.

## Configuring `chip.syscfg` in the SysConfig GUI

1. To open `matter/examples/[application]/cc13x4_26x4/chip.syscfg` in the GUI,
   add the following line to the top of the file:

```
  // @cliArgs --product <path to SDK>/.metadata/product.json --board /ti/boards/LP_EM_CC1354P10_6 --rtos freertos
```

2. Open the `syscfg` file using the standalone Sysconfig GUI
   (`sysconfig_gui.sh`) from the SysConfig installation folder.
3. Click on _Show Device View_ and then click _Switch_.
4. Select _Board_ as _None_ and _Device_ as _`CC2674P10RGZ`_, Unselect
   _`Lock PinMux`_, and click _Confirm_.
5. To fix errors, make the following module changes:
    - _RF Design_ and _RF Stacks -> BLE -> Radio_: click on _accepting the
      current value_, which should be _`LP_CC2674P10_RGZ`_ in the drop down menu
      for _Based On RF Design_
    - _TI DEVICES -> Device Configuration_: Clear
      _`XOSC Cap Array Modification`_
    - _TI DRIVERS -> RF_: Set _Global Event Mask_ as _None_ and _No. of Antenna
      Switch Control Pins_ as _0_
    - _TI DRIVERS -> UART2 -> `PinMux`_: Set _UART Peripheral_ to _UART0_, _TX
      Pin_ to _`DIO_13/19`_, and _RX Pin_ to _`DIO_12/18`_
    - _TI DRIVERS APPS -> Button_: Set _`PinMux`_ of _CONFIG_BTN_LEFT_ to
      _`DIO_15`_ and _CONFIG_BTN_RIGHT_ to _`DIO_14`_
    - _TI DRIVERS APPS -> LED_: Set _`PinMux`_ of _CONFIG_LED_RED_ to _`DIO_6`_
      and _CONFIG_LED_RIGHT_ to _`DIO_7`_
6. Save the SysConfig file (click on _Save As_) and ensure the file name matches
   the reference from `BUILD.gn` (default project name is `chip.syscfg`).
7. Open the new SysConfig file with a text editor and remove the generated
   arguments.
    ```
    /**
     * These arguments were used when this file was generated.  They will be automatically applied on subsequent loads
     * via the GUI or CLI.  Run CLI with '--help' for additional information on how to override these arguments.
     * @cliArgs --device "CC2674P10RGZ" --package "RGZ" --part "Default" --rtos "freertos" --product "simplelink_cc13xx_cc26xx_sdk@7.10.01.24"
     * @versions {"tool":"1.18.0+3130"}
     */
    ```
8. Move the `*.syscfg` file into the
   `matter/examples/[application]/cc13x4_26x4/` folder. Make sure that the
   `args.gni` parameters are aligned for the `CC2674P10RGZ` as detailed above,
   and build the example using the `README.md` instructions.

## Modifications required for the CC2674R10

After applying all items in the "Configuring `chip.syscfg` in the SysConfig GUI"
section, additional steps must also be applied to generate Matter project for
the CC2674R10.

-   `examples/[application]/cc13x4_26x4/args.gni` should have
    `ti_simplelink_board` as `CC2674` and `ti_simplelink_device = CC2674R10RGZ`.
-   `examples/[application]/cc13x4_26x4/chip.syscfg` opened with a Text Editor
    should change `ble.radioConfig.codeExportConfig.$name` to
    `ti_devices_radioconfig_code_export_param2` and `ble.rfDesign` to
    `LP_EM_CC1354P10_1`

Furthermore, the subsequent changes apply specifically for the CC2674R10 and
should be addressed from a SysConfig Editor.

1. Pins will need to be reconfigured as such:

    | SysConfig pin name | R10 `PinMux` |
    | ------------------ | ------------ |
    | UART_RX            | `DIO_2`      |
    | UART_TX            | `DIO_3`      |
    | CONFIG_BTN_LEFT    | `DIO_13`     |
    | CONFIG_BTN_RIGHT   | `DIO_14`     |
    | CONFIG_LED_RED     | `DIO_6`      |
    | CONFIG_LED_GREEN   | `DIO_7`      |

2. _Custom -> IEEE 802.15.4-2006, `250 kbps`, `OQPSK`, `DSSS = 1:8` -> Code
   Export Configuration_, acknowledge and dismiss the PA radio setup error
3. _Custom -> IEEE 802.15.4-2006, `250 kbps`, `OQPSK`, `DSSS = 1:8` -> RF
   Command Symbols_, change `CMD_RADIO_SETUP` from `RF_cmdRadioSetup` to
   `RF_cmdIeeeRadioSetup` and add the following functions from the drop-down:
   `CMD_TX_TEST`,`CMD_IEEE_ED_SCAN`, `CMD_IEEE_CSMA`, and `CMD_IEEE_RX_ACK`.

## Building examples for the CC1354P10-1

To migrate the CC1354P10-6 examples to the CC1354P10-1 platform, there are only
two steps:

1. `examples/[application]/cc13x4_26x4/args.gni` should have
   `ti_simplelink_board` as `CC1354P10-1`
2. `examples/[application]/cc13x4_26x4/chip.syscfg` opened with a Text Editor
   should change `ble.radioConfig.codeExportConfig.$name` to
   `ti_devices_radioconfig_code_export_param2` and `ble.rfDesign` to
   `LP_EM_CC1354P10_1`

After this, the example's `README.md` instructions can be followed to produce
the executable needed.
