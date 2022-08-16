# Flashing EFR32 Examples in VS Code

This section covers flashing efr32 examples to target devices.

## Requirements

1. Simplicity Commander - The Standalone Utility to flash the target devices can
   be found in the following links:
   [installer for windows](https://www.silabs.com/documents/public/software/SimplicityCommander-Windows.zip),
   [installer for MAC](https://www.silabs.com/documents/public/software/SimplicityCommander-Mac.zip),
   [installer for Linux](https://www.silabs.com/documents/public/software/SimplicityCommander-Linux.zip).
   please ensure the proper directories are added to the PATH.
2. Python - Ensure that Python is installed and the proper directories are
   added to the PATH.

## Flashing

<ol>

### Step 1:

Use the shortcut (Ctrl + Shift + P) to trigger the command pallet.

### Step 2:

Search for "run build task" and select "Run build task" from the drop
down menu.

### Step 3:

Select "Flash efr32 example" from the options.

### Step 4:

On the next down menu, please choose the proper example to flash to your
target device. The example must have an "out" directory that contains a
binary file with the ".out" extension.

### Step 4:

The final prompt will ask the user to choose the target device. Please select the target device.

## ![](../../images/flash_efr32_example.gif)

</ol>

-----

[Table of Contents](../../README.md)
