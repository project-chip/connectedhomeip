Texas Instruments Inc. Matter: Factory Data Programming User Guide

Introduction:

This document describes how to use Factory data programming feature for the
Matter example applications from Texas Instruments.

## Background

Matter specification lists various information elements that are programmed at
factory. These values do not change and some are unique per device. This feature
enables customers developing matter products on TI devices to program this data
and use this as a starting point towards developing their factory programming
infrastructure for their Matter devices.

## Solution Overview:

TI matter examples allow to use the factory data in the following two ways:

-   **Example Out of Box Factory Data** : Use TI example DAC values to get
    started. This is intended to be used when just starting with matter or
    during the development until customer or product specific data is not
    required.
-   **Custom factory data** : Allows to configure custom factory data via a JSON
    file. The custom values are then processed by a script provided by TI and
    merged with the matter application to create a binary that can be flashed on
    to the devices.

### Solution Block Diagram

![Block Diagram](images/factory_data_overview.png)

Below is the high level description of each element

1. JSON: This file is located at src/platform/cc13xx_26xx. Developer
   would configure this per device. Elements in this file are from the
   specification.
2. Matter application hex with dummy factory data is any TI's matter example
   application
3. BIM is the Boot image manager that allows to write and read/program the OTA
   image received over the air for upgrade. This is built with the matter
   application. This does not require additional build steps from developers.
4. create_factory_data.py: processes JSON file and generates hex file with
   unique factory data values configured in the JSON file.
5. factory_data_trim.py: when using the custom factory data option, this script
   removes the dummy factory data which is required to be able to successfully
   compile the application.
6. oad_and_factory_data_merge_tool.py: Merges the factory data hex, matter application without
   factory data and bim to generate a functional hex that can be programmed on
   to the device.


For CC13x4 devices, the only difference in the flow is that there is no BIM being merged with the application and factory data file. 
## Flash memory layout

![Memory Layout 1](images/cc13x2_memmap.png)

![Memory Layout 2](images/cc13x4_memmap.png)

## How to use

Out of box factory data location is configured to be on second last page of
flash. For CC13x2, the starting address is 0xAC000. For CC13x4, the starting address is 0xFE800.
This can be configured in the linkercommand file.

To configure:

1. Linker command file: Set the start address for factory data in the linker command file being used by the application
   ![Linker snippet 1](images/ti_linker_factory_data_1.png)
   ![Linker snippet 2](images/ti_linker_factory_data_2.png)

2. create_factory_data.py: Set the address of the start of the factory data
   elements. Refer to the comments in the script.
   ![Factory Data creation script 1](images/ti_factory_data_mem_address_script.png)
   ![Factory Data creation script 2](images/ti_factory_data_mem_address_script_2.png)

3. In the example's args.gni, set 'custom_factory_data' to true       
              


It is recommended to keep a dedicated page (2 pages for CC13x4) for factory data.

### Formatting certs and keys

To format the DAC, private key and PAI as hex strings as shown in the Factory
Data JSON file, use the chip-cert tool located at src/tools/chip-cert and run
the _convert-cert_ command, and list -X, or X.509 DER hex encoded format, as the
output format. These strings can then be copied into the JSON file.

The SPAKE parameters should be converted from base-64 to hex as well before being
copied into the JSON file. 

### Creating images

The example application can be built using the instructions in the example's
README. The factory data from the JSON file will be formatted into a hex file
that will then be merged into the final executable. The final executable will be
named _{example-application}-bim.hex_ for CC13x2 and 
_{example-application}-and-factory-data.hex_ for CC13x4, and the factory data 
that was inputted into the JSON file will be named _{example-application}-factory-data.hex_.
