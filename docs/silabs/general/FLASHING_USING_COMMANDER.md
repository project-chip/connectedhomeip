# Flashing the Matter Pre-Built Binary Using Simplicity Commander
- To Flash the application for EFR32 and SiWx917 SOC boards, please use the Simplicity Commander software.
- Before flashing the application for EFR32 Boards, We need to flash **bootloader images** as per board variants
    - **BRD4186C Board**
      - For MG24 + RS9116 :- Internal Bootloader (bootloader-storage-internal-single-512k-BRD4186C-gsdk4.1)
      - For MG24 + WF200 :- External Bootloader (bootloader-storage-spiflash-single-1024k-BRD4186C-gsdk4.1)
    - **BRD4187C Board**
      - For MG24 + RS9116 :- Internal Bootloader (bootloader-storage-internal-single-512k-BRD4187C-gsdk4.1)
      - For MG24 + WF200 :- External Bootloader (bootloader-storage-spiflash-single-1024k-BRD4187C-gsdk4.1)
   - Bootloader binaries are available in the respective path of the codebase **third_party/silabs/matter_support/matter/efr32/bootloader_binaries**
   - Bootloader binaries are also available on the [Matter Artifacts](./ARTIFACTS.md) page.
   - We recommend to flash latest bootloader binaries from codebase with every update.
 - **Note**: Bootloader binaries should be flashed using Simplicity Commander or Simplicity Studio.

## Flashing the Bootloader Binaries for EFx32 Board using Simplicity Commander

1. Open Simplicity Commander application on your machine.

2. In the Simplicity Commander window, click Select Kit and choose your radio board.

![Silicon Labs - design](./images/commander-select-board.png)

3. In the navigation pane, go to the Flash section.

4. Above "Reload tab" your board will be displayed, click Browse next to the Binary File field and locate the appropriate bootloader binary for your board.

![Silicon Labs - design](./images/select-flash-option-efr32-commander.png)

5. Click Flash, the bootloader will be flashed and the Log Window will display a "Flashing completed Successfully" message.

![Silicon Labs - design](./images/simplicity-commander-flash-bootloader.png)

## Flashing the EFR32 Matter Pre-Built Binary using Simplicity Commander

1. Open Simplicity Commander application in your desktop.

2. In the Simplicity Commander window, click Select Kit and choose your radio board.

![Silicon Labs - design](./images/commander-select-board.png)

3. In the navigation pane, go to the Flash section.
   
4. Above beside "Reload tab" board will be displayed, click Browse next to the Binary File field and locate binary.

![Silicon Labs - design](./images/select-flash-option-efr32-commander.png)

5. Click Flash, the binary will be flashed and the Log Window will display a "Flashing completed Successfully" message.

![Silicon Labs - design](./images/commander-flash-success-efr32.png)


## Flashing the SiWx917 SOC Matter Pre-Built Binary using Simplicity Commander

1. Open Simplicity Commander application on your machine.

2. In the Simplicity Commander window, click Select Kit and choose your radio board.

![Silicon Labs - design](./images/commander-select-board.png)

3. In the navigation pane, go to the Flash section.

4. Above beside "Reload tab" board will be displayed, click Browse next to the Binary File field and locate binary.

![Silicon Labs - design](../wifi/images/select-flash-option-soc-commander.png)

5. Click Flash, the binary will be flashed and the Log Window will display a "Flashing completed Successfully" message.

![Silicon Labs - design](../wifi/images/commander-flash-success-soc.png)