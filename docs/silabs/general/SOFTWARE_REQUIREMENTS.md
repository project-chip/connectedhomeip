# Matter Software Requirements

## System Agnostic Software Requirements (Windows/Mac/Linux):

1. SSH Client ([PuTTY](https://www.putty.org/), Terminal, or similar):
    
    SSH client is used to communicate with the Raspberry Pi over a secure
    shell.

2. [Raspberry Pi Disk Imager](https://www.raspberrypi.com/software/)
    
    Raspberry Pi Disk Imager is used to flash the SD Card that contains the
    operating system for the Raspberry Pi. Note that, for the Thread demo, the
    operating system is the demo image.

3. Flash Tools

    - [Flash tool for EFR32 and SiWx917 Devices](../general/FLASH_SILABS_DEVICE.md)

        Simplicity Commander standalone can be used to flash Silicon Labs hardware with firmware images for the RCP and the Matter Accessory Device.

    - [Additional Flash tools for SiWx917 SoC Device](../general/FLASH_SILABS_SiWx917_SOC_DEVICE.md)

        The Ozone Debugger can be configured to flash the application for the SiWx917 SoC device.
	
        > **Note:** 
        > The SiWx917 SoC device is not yet officially added in Ozone, but it can be configured in Ozone by following the [SiWx917 Ozone Configuration Guide](../wifi/SiWx917_Enablement_For_Ozone.md).

4. [Tera Term](https://osdn.net/projects/ttssh2/releases/)

    Tera Term is the terminal emulator for Microsoft Windows that supports serial port, telnet and SSH connections.

5. [Git Version Control ](https://git-scm.com/downloads)[only required for
   building images and development]

    Clone the Silicon Labs Matter repo, then enter and sync all the necessary
    submodules with the following commands:

    ```shell
    $ git clone https://github.com/SiliconLabs/matter.git
    ```

    ```shell
    $ cd matter
    ```

    ```shell
    $ ./scripts/checkout_submodules.py --shallow --recursive --platform efr32
    ```

6. If you are using an EFR32MG2x device you will require a bootloader to run the
   demo applications. When you flash your application image be sure to include a
   bootloader as well (if one is not already present). Bootloader images are provided on the [Matter Artifacts page](../general/ARTIFACTS.md).

## Windows-Specific Software Requirements:

&emsp; These requirements are in addition to those mentioned above, for Windows
only.

&emsp; A Unix-like command line:

-   [Windows Subsystem for Linux (WSL)](https://docs.microsoft.com/en-us/windows/wsl/about)
    **OR**
-   [VirtualBox](https://www.virtualbox.org/) with
    [Ubuntu 20.04.x LTS](https://ubuntu.com/download/desktop)

    > WSL or Virtual Box are used to emulate a virtual Linux machine, which is
    > useful for accessing the Linux command line tools

## Mac-Specific Requirements (building own images):

&emsp; These requirements are in addition to those mentioned above for Mac OS
users building their own images.

1. Install Homebrew

    ```shell
    $ /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    ```

2. Install pkg-config, openssl, git-lfs, python3

    ```shell
    $ brew install pkg-config openssl git-lfs python3
    ```

3. Install ARM GNU Toolchain (https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads/12-2-rel1)

    Use the x86_64-arm-none-eabi package for Intel-based Macs and arm64-arm-none-eabi for M1/M2-based Macs.

4. Add ARM GCC toolchain to the search path within `bootstrap.sh` by adding this
   line of code:

    ```shell
    $ export PATH="/Applications/ArmGNUToolchain/12.2.rel1/arm-none-eabi/bin:$PATH"
    ```
    
    Depending on your Mac processor, you will have to complete the following additional steps.

### Apple Silicon (M1/M2) Software Requirements:

Users may also have to specify which `pkg-config`/`openssl` to use by adding
these lines of code to `bootstrap.sh`:

```shell
$ export PATH="/opt/homebrew/opt/openssl@3/bin:$PATH"
```

```shell
$ export PKG_CONFIG_PATH="/opt/homebrew/opt/openssl@3/lib/pkgconfig"
```

### Intel Software Requirements:

```shell
$ cd /usr/local/lib/pkgconfig
```

```shell
$ ln -s ../../Cellar/openssl@1.1/1.1.1g/lib/pkgconfig/* .
```

`openssl@1.1/1.1.1g` may need to be replaced with the actual version of OpenSSL
installed by Brew.

If using MacPorts, the command 

```shell
$ port install openssl
```

is sufficient to satisfy this
dependency.

## Linux Specific Requirements:

On Debian-based Linux distributions such as Ubuntu, these dependencies can be
satisfied with the following:

```shell
$ sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev
```

## Wi-Fi Specific Requirements:

### Software Packages Required for Wi-Fi EFR32 NCP Devices

1. Gecko SDK package.
2. WiseConnect SDK v2.x for RS9116 NCP.
3. Wiseconnect SDK v3.x for SiWx917 NCP.
4. [Firmware for RS9116 NCP](../general/ARTIFACTS.md#rs9116-firmware)
5. [Firmware for SiWx917 NCP](../general/ARTIFACTS.md#siwx917-firmware-for-siwx917-ncp)

### Software Packages Required for Wi-Fi SiWx917 SoC Devices

1. GeckoSDK package.
2. WiSeConnect SDK v3.x.
3. [Firmware for SoC](../general/ARTIFACTS.md#siwx917-firmware-for-siwx917-soc)

**Note:**
Before you run the demo or development on the Wi-Fi, please be sure that you update the NCP or SiWx917 firmware to the latest version.

Pre-Built firmware images are available in the following path of github repository:
   1. For RS9116 NCP images: under `third_party/silabs/wiseconnect-wifi-bt-sdk/firmware`
   2. For SiWx917 NCP & SoC images: under `third_party/silabs/wifi-sdk/connectivity_firmware`

1. [Setting up TeraTerm](https://docs.silabs.com/SiWx917/wiseconnect/2.0/tera-term-setup)
2. [Updating the RS9116 or SiWx917 NCP Firmware](../wifi/RUN_DEMO.md#steps-to-update-the-firmware-on-ncp-boards-using-tera-term)
3. [Updating the SiWx917 SOC Firmware Using Simplicity Commander](../wifi/RUN_DEMO_SiWx917_SoC.md#steps-to-update-the-firmware-on-siwx917-soc-radio-board-using-simplicity-commander)
