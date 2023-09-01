# Running Matter Examples on the TI SimpleLink CC2674P10 and CC2674R10

The existing Matter project examples are based on LP_EM_CC1354P10_6. If using 
the CC2674P10, the following migration steps are required. Developers are 
strongly encouraged to start with a `cc13x4_26x4` example and migrate the 
project accordingly. Example projects can be found in the following location: 
`matter/examples/[application]/cc13x4_26x4`

## Dependencies

The following must be installed on your system before proceeding:

* [SysConfig](https://www.ti.com/tool/SYSCONFIG) v1.16.2 or later
* [SIMPLELINK-LOWPOWER-F2-SDK](https://www.ti.com/tool/SIMPLELINK-LOWPOWER-SDK) v7.10.01.24

## Matter source code changes

The following are changes to the Matter source code files which should be 
applied to convert a `matter/examples/[application]/cc13x4_26x4` project to the
CC2674P10 device

* `examples/[application]/cc13x4_26x4/args.gni`, modify/add the following 
  defines for the CC2674
  - `ti_simplelink_board = CC2674`
  - `ti_simplelink_device = CC2674P10RGZ`
* `third_party/ti_simplelink_sdk/repo_cc13xx_cc26xx`, replace this folder 
  contents with the 7.10.01.24 version from 
  [TI's downloads page](https://www.ti.com/tool/download/SIMPLELINK-LOWPOWER-F2-SDK/7.10.01.24) which is required to add support SDK for the CC2674P10 device.

## Configuring `chip.syscfg` in the SysConfig GUI

1. To open `matter/examples/[application]/cc13x4_26x4/chip.syscfg` in the GUI, 
  add the following line to the top of the file: 
  ```
    // @cliArgs --product <path to SDK>/.metadata/product.json --board /ti/boards/LP_EM_CC1354P10_6 --rtos freertos
  ```
2. Open the `syscfg` file using the standalone Sysconfig GUI (`sysconfig_gui.sh`) 
  from the SysConfig installation folder.
3. Click on *Show Device View* and then click *Switch*.
4. Select *Board* as *None* and *Device* as *`CC2674P10RGZ`*, Unselect *`Lock PinMux`*, and click *Confirm*.
5. To fix errors, make the following module changes:
    * *RF Design* and *RF Stacks -> BLE -> Radio*: click on *accepting the 
      current value*, which should be *`LP_CC2674P10_RGZ`* in the drop down menu 
      for *Based On RF Design*
    * *TI DEVICES -> Device Configuration*: Clear *`XOSC Cap Array Modification`*
    * *TI DRIVERS -> RF*: Set *Global Event Mask* as *None* and *No. of Antenna 
      Switch Control Pins* as *0*
    * *TI DRIVERS -> UART2 -> `PinMux`*: Set *UART Peripheral* to *UART0*, 
      *TX Pin* to *`DIO_13/19`*, and *RX Pin* to *`DIO_12/18`*
    * *TI DRIVERS APPS -> Button*: Set *`PinMux`* of *CONFIG_BTN_LEFT* to *`DIO_15`* 
      and *CONFIG_BTN_RIGHT* to *`DIO_14`*
    * *TI DRIVERS APPS -> LED*: Set *`PinMux`* of *CONFIG_LED_RED* to *`DIO_6`* and 
      *CONFIG_LED_RIGHT* to *`DIO_7`* 
6. Save the SysConfig file (click on *Save As*) and ensure the file name matches 
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
8. Move the `*.syscfg` file into the `matter/examples/[application]/cc13x4_26x4/` 
  folder.  Make sure that the `args.gni` parameters are aligned for the 
  `CC2674P10RGZ` as detailed above, and build the example using the `README.md` 
  instructions. 

## Modifications required for the CC2674R10

After applying all items in the "Configuring `chip.syscfg` in the SysConfig GUI" 
section, additional steps must also be applied to generate Matter project for 
the CC2674R10.

* `examples/[application]/cc13x4_26x4/args.gni` should have 
  `ti_simplelink_board` as `CC2674` and `ti_simplelink_device = CC2674R10RGZ`.  
* `examples/[application]/cc13x4_26x4/chip.syscfg` opened with a Text Editor
  should change `ble.radioConfig.codeExportConfig.$name` to
  `ti_devices_radioconfig_code_export_param2` and `ble.rfDesign` to `LP_EM_CC1354P10_1`

Furthermore, the subsequent changes apply specifically for the CC2674R10 and 
should be addressed from a SysConfig Editor.
1. Pins will need to be reconfigured as such: 
    | SysConfig pin name | R10 `PinMux`|
    | ------------------ | ----------- |
    | UART_RX            | `DIO_2`     |
    | UART_TX            | `DIO_3`     |
    | CONFIG_BTN_LEFT    | `DIO_13`    |
    | CONFIG_BTN_RIGHT   | `DIO_14`    |
    | CONFIG_LED_RED     | `DIO_6`     |
    | CONFIG_LED_GREEN   | `DIO_7`     |
2. *Custom -> IEEE 802.15.4-2006, 250 kbps, OQPSK, DSSS = 1:8 -> Code Export Configuration*, 
  acknowledge and dismiss the PA radio setup error
3. *Custom -> IEEE 802.15.4-2006, 250 kbps, OQPSK, DSSS = 1:8 -> RF Command Symbols*,
  change `CMD_RADIO_SETUP` from `RF_cmdRadioSetup` to `RF_cmdIeeeRadioSetup` and
  add the following functions from the drop-down: `CMD_TX_TEST`, 
  `CMD_IEEE_ED_SCAN`, `CMD_IEEE_CSMA`, and `CMD_IEEE_RX_ACK`