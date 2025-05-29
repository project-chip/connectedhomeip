# NXP K32W0x1 OTA guide

## The Secondary Stage Bootloader (SSBL)

There are multiple SSBL binaries provided by the SDK:

| description                     | github SDK path                                                                              | package SDK path                                                                           |
| ------------------------------- | -------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------ |
| Default SSBL                    | NA                                                                                           | `boards/k32w061dk6/wireless_examples/framework/ssbl/binary/ssbl.bin`                       |
| SSBL with PDM in external flash | `examples/k32w061dk6/wireless_examples/framework/ssbl/binary/ssbl_ext_flash_pdm_support.bin` | `boards/k32w061dk6/wireless_examples/framework/ssbl/binary/ssbl_ext_flash_pdm_support.bin` |

The SSBL is also built alongside the reference application and it can be
configured according to the following table:

| gn arg                            | default                     | description                                                                              |
| --------------------------------- | --------------------------- | ---------------------------------------------------------------------------------------- |
| `ssbl_pdm_external_flash`         | true                        | Enable/disable PDM in external flash                                                     |
| `ssbl_multi_image_support`        | true                        | Enable/disable multi-image OTA feature                                                   |
| `ssbl_ota_entry_storage`          | "OTACustomStorage_ExtFlash" | Configure custom OTA entry storage type                                                  |
| `ssbl_simple_hash_verification`   | false                       | Enable/disable simple hash verification alternative to secure boot                       |
| `ssbl_optimize_spifi_flash`       | false                       | Optimize `SPIFI` flash driver size                                                       |
| `ssbl_spifi_dual_mode`            | false                       | Enable/disable `SPIFI` dual mode support (e.g. used by K32W041AM variant)                |
| `ssbl_version`                    | 0                           | Set SSBL version                                                                         |
| `ssbl_use_redlib`                 | false                       | Enable/disable usage of `redlib` NXP library. If false, the build will use `newlib` nano |
| `ssbl_ota_data_in_external_flash` | false                       | Enable/disable OTA support for application with sections stored in external flash        |

## Simple hash verification

When secure boot is not used, a simple hash can be appended at the end of the
image for integrity check. Applications should be built with
`chip_simple_hash_verification=1`.

## Writing the SSBL

Before writing the SSBL, it it recommanded to fully erase the internal flash.

Using DK6Programmer utility from Windows:

```
DK6Programmer.exe -V 5 -P 1000000 -s <COM_PORT> -e Flash
```

Using `dk6prog` from `SPSDK`:

```
$ dk6prog listdev
This is an experimental utility. Use with caution!

List of available devices:
DEVICE ID: DN038ZH3, VID: 0x403, PID: 0x6015, Serial number: DN038ZH3, Description: DK6 Carrier Board, Address: 9, Backend: Backend.PYFTDI
$ dk6prog -d DN038ZH3 erase 0 0x9de00

This is an experimental utility. Use with caution!

Erasing memory  [####################################]  100%
```

`chip-k32w0x-ssbl.bin` must be written at address 0 in the internal flash:

Using DK6Programmer utility from Windows:

```
DK6Programmer.exe -V2 -s <COM_PORT> -P 1000000 -Y -p FLASH@0x00="chip-k32w0x-ssbl.bin"
```

Using `dk6prog` from `SPSDK`:

```
$ dk6prog -d DN038ZH3 write 0 ~/path/to/bin/chip-k32w0x-ssbl.bin

This is an experimental utility. Use with caution!

Writing memory  [####################################]  100%
Written 7890 bytes to memory ID 0 at address 0x0
```

### Writing the PSECT

This is the list of all supported partitions:

```
0000000010000000 : SSBL partition

    00000000 -----------> Start Address
    1000 ---------------> 0x0010 Number of 512-bytes pages
    00 -----------------> 0x00 Bootable flag
    00 -----------------> 0x00 Image type (0x00 = SSBL)

00400000c9040101: Application partition

    00400000 -----------> 0x00004000 Start Address
    c904 ---------------> 0x04c9 Number of 512-bytes pages
    01 -----------------> 0x01 Bootable flag
    01 -----------------> 0x01 Image type (0x01 = Application)

00000010800000fe: Ext Flash text partition

    00000010 -----------> 0x10000000 Start Address (external flash)
    8000 ---------------> 0x0080 Number of 512-bytes pages
    00 -----------------> 0x00 Bootable flag
    fe -----------------> 0xFE Image type (0xFE = Ext Flash text)

00000110300200fc : OTA Image partition

    00000110 -----------> 0x10010000 Start Address
    3002----------------> 0x0230 Number of 512-bytes pages
    00 -----------------> 0x00 Bootable flag
    fc -----------------> 0xFC Image type (0xFC = OTA partition)

00000510100000fd: NVM partition

    00000510 -----------> 0x10050000 Start Address
    1000 ---------------> 0x0010 Number of 512-bytes pages
    00 -----------------> 0x00 Bootable flag
    fd -----------------> 0xFD Image type (0xFD = NVM partition)
```

First, image directory 0 (SSBL partition) must be written:

Using DK6Programmer utility from Windows:

```
DK6Programmer.exe -V5 -s <COM port> -P 1000000 -w image_dir_0=0000000010000000
```

Using `dk6prog` from `SPSDK`:

```
$ dk6prog -d DN038ZH3 write 0x160 [[0000000010000000]] 8 PSECT

This is an experimental utility. Use with caution!

Writing memory  [####################################]  100%
Written 8 bytes to memory ID PSECT at address 0x160
```

Here is the interpretation of the fields:

```
00000000 -> start address 0x00000000
1000     -> size = 0x0010 pages of 512-bytes (= 8kB)
00       -> not bootable (only used by the SSBL to support SSBL update)
00       -> SSBL Image Type
```

Second, image directory 1 (application partition) must be written:

Using DK6Programmer utility from Windows:

```
DK6Programmer.exe -V5 -s <COM port> -P 1000000 -w image_dir_1=00400000C9040101
```

Using `dk6prog` from `SPSDK`:

```
$ dk6prog -d DN038ZH3 write 0x168 [[00400000C9040101]] 8 PSECT

This is an experimental utility. Use with caution!

Writing memory  [####################################]  100%
Written 8 bytes to memory ID PSECT at address 0x168
```

Here is the interpretation of the fields:

```
00400000 -> start address 0x00004000
C904     -> 0x4C9 pages of 512-bytes (= 612.5kB)
01       -> bootable flag
01       -> image type for the application
```

Please note the user can write additional partitions by writing
`image_dir_2/3/4` with the wanted configuration. In case of using the `SPSDK`
tool, the appropriate offset must be calculated

## Removing SSBL Upgrade Region

The example also offers the possibility to remove SSBL upgrade region, for
reserving more space for application level.

A new flag `chip_reduce_ssbl_size` is introduced. In order to remove the SSBL
upgrade region, `chip_reduce_ssbl_size=true` must be provided to the build
system

The programming method will change:

-   Writing image directory 1 should change to Using DK6Programmer utility from
    Windows:

    ```
    DK6Programmer.exe -V5 -s <COM port> -P 1000000 -w image_dir_1=00200000D9040101
    ```

    Using `dk6prog` from `SPSDK`:

    ```
    $ dk6prog -d DN038ZH3 write 0x168 [[00200000D9040101]] 8 PSECT

    This is an experimental utility. Use with caution!

    Writing memory  [####################################]  100%
    Written 8 bytes to memory ID PSECT at address 0x168
    ```

    Here is the interpretation of the fields:

    ```
    00200000 -> start address 0x00002000
    D904     -> 0x4D9 pages of 512-bytes (= 620.5kB)
    01       -> bootable flag
    01       -> image type for the application
    ```

-   Matter application offset address should change to Using DK6Programmer
    utility from Windows:

    ```
    DK6Programmer.exe -V2 -s <COM_PORT> -P 1000000 -Y -p FLASH@0x2000="chip-k32w0x-contact-example.bin"
    ```

    Using `dk6prog` from `SPSDK`:

    ```
    $ dk6prog -d DN038ZH3 write 0x2000 ~/path/to/bin/chip-k32w0x-contact-example.bin

    This is an experimental utility. Use with caution!

    Writing memory  [####################################]  100%
    Written 596450 bytes to memory ID 0 at address 0x2000
    ```
