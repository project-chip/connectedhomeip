# Building in VS Code

This section covers building the binaries required for [ Flashing ](FLASH.md) and
[ Debugging ](DEBUG.md) the target Matter Accessory Devices.

<a name="linMac"></a>

## Linux/Mac

### Step 1:

Use the shortcut (Ctrl + Shift + P) to trigger the command pallet.

### Step 2:

Search for "run build task" and select the "Run Build Task" option.

### Step 3:

A new drop down menu will appear. Select "Build efr32 example".

### Step 4:

On the next menu select an EFR32 example to build.

### Step 5:

Select the appropriate target board.

![](../../images/build_efr32_example.gif)

## Windows

Building for Windows requires one extra step before running the build example
task.

### Step 1:

Press (Ctrl + Shift + P) to trigger the command pallet.

### Step 2:

Search for "run build task" and select "Run build task" from the drop-down menu.

### Step 3:

On the next menu, select "Windows setup" from the options.

### Step 4:

The next menu contains two options, "setup" and
"cleanup". Select _setup_ in order to initiate the setup process. This step installs crucial
packages that are required to build the examples such as openssl. if openssl
is not detected, an installation will be initiated in a new window. Follow the installation prompts in order for the setup to be successful. Select _cleanup_ to
clean up the environment after building the examples. 

### Step 5:

This step is identical to the steps for building on Linux/Mac, Refer to
that [ section ](#linMac).

## ![](../../images/win_run_setup.gif)
### Note for Windows Developers
During bootstrap, an error might occur. Check if pip.exe is installed in ./environment/gn_out/python-venv/Scripts. If it does not exist, install pip.exe in that directory and rebuild the example.

-----

[Table of Contents](../../README.md)
