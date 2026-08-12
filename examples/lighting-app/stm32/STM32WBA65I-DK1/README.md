# Matter STM32 Lighting Example over thread

This example demonstrates the Matter Lighting application on stm32 platform.

---

-   [Matter STM32 Lighting Example over thread](#matter-stm32-lighting-example-over-thread)
    -   [Building and Commissioning](#building-and-commissioning)
    -   [Cluster Control](#cluster-control)
    -   [Indicate current state of lightbulb](#indicate-current-state-of-lightbulb)
    -   [OTA and `OEMiROT` application](#ota-and-oemirot-application)
        -   [Linux / Ubuntu: prepare the X-CUBE-MATTER shell scripts](#linux--ubuntu-prepare-the-x-cube-matter-shell-scripts)
        -   [Pre-compiled from X-CUBE-MATTER (STM32CubeIDE)](#pre-compiled-from-x-cube-matter-stm32cubeide)
    -   [Build Instructions](#build-instructions)
        -   [Step 1: Copy the Secure veneers (`secure_nsclib.o`)](#step-1-copy-the-secure-veneers-secure_nsclibo)
        -   [Step 2: Build with GN](#step-2-build-with-gn)
        -   [Step 3: Copy Binary](#step-3-copy-binary)
        -   [Step 4: Sign the Binary](#step-4-sign-the-binary)
            -   [Option A: Using CLI](#option-a-using-cli)
            -   [Option B: Using GUI](#option-b-using-gui)
        -   [Step 5: Flash the Board](#step-5-flash-the-board)
    -   [OTA Update (N+1 Image)](#ota-update-n1-image)
        -   [Step 1: Increment the software version](#step-1-increment-the-software-version)
        -   [Step 2: Rebuild with GN](#step-2-rebuild-with-gn)
        -   [Step 3: Copy and sign for OTA](#step-3-copy-and-sign-for-ota)
        -   [Step 4: Create Matter binary with ST header](#step-4-create-matter-binary-with-st-header)
        -   [Step 5: Create the Matter OTA file (.ota)](#step-5-create-the-matter-ota-file-ota)
        -   [Step 6: Use the OTA image](#step-6-use-the-ota-image)
    -   [Quick Reference](#quick-reference)

---

## Building and Commissioning

Please refer
[Building and Commissioning](../../../docs/platforms/stm32/stm32_getting_started_guide.md#building-the-example-application)
guides to get started

```
./scripts/build/build_examples.py --target stm32-$stm32_BOARD-lighting build
```

## Cluster Control

After successful commissioning, use `chip-tool` to control the board

-   OnOff Cluster
    ```
    ./chip-tool onoff on <NODE ID> 1
    ./chip-tool onoff off <NODE ID> 1
    ./chip-tool onoff toggle <NODE ID> 1
    ```
-   LevelControl Cluster

    ```
    ./chip-tool levelcontrol move-to-level 128 10 0 0 <NODE ID> 1
    ```

-   ColorControl Cluster
    ```
    ./chip-tool colorcontrol move-to-hue-and-saturation 240 100 0 0 0 <NODE ID> 1
    ```

## Indicate current state of lightbulb

The LCD screen displays "BLE connected" when the BLE rendezvous started. The LCD
screen displays "Network Join" when the board joins thread network. The LCD
screen displays "LED ON" + the light level when the controller enable the light
ON and the line is erased when the controller disable the light.

## OTA and `OEMiROT` application

All the commands below use `XCUBE_PATH`, which points to the board folder of the
extracted
[X-CUBE-MATTER](https://www.st.com/en/embedded-software/x-cube-matter.html)
package. Export it once in your shell:

```bash
XCUBE_PATH="path/to/Projects/STM32WBA65I-DK1"
PROV_PATH="${XCUBE_PATH}/ROT_Provisioning_Lighting_OEMiRoT"
```

### Linux / Ubuntu: prepare the X-CUBE-MATTER shell scripts

The X-CUBE-MATTER package is delivered for Windows: all `.sh` files use CRLF
line endings and have no execute permission. On Ubuntu this breaks the
STM32CubeIDE post-build step and the provisioning scripts with errors such as:

-   `Permission denied` when the build calls `postbuild.sh`
-   `postbuild.sh: not found` / `/bin/bash^M: bad interpreter` (caused by the
    CRLF shebang)

Run once, from the root of the extracted X-CUBE-MATTER package:

```bash
sudo apt install -y dos2unix
find . -name "*.sh" -exec dos2unix {} \; -exec chmod +x {} \;
```

> This must be done **before** building `OEMiROT_Boot` /
> `OEMiROT_Appli_TrustZone` in STM32CubeIDE, and before running `regression.sh`
> / `provisioning.sh`.

### Pre-compiled from X-CUBE-MATTER (STM32CubeIDE)

The bootloader and the Secure application are **not** built by GN: they must be
compiled from the X-CUBE-MATTER package with STM32CubeIDE, in this order.

1.  **OEMiROT Boot** (bootloader)

    -   In STM32CubeIDE: **File > Import... > Existing Projects into
        Workspace**, then select
        `${XCUBE_PATH}/Applications/ROT_Lighting_OEMiRoT/OEMiROT_Boot/STM32CubeIDE`
    -   Right-click the `OEMiROT_Boot` project > **Build Project**
    -   Produces `bl2.bin`

2.  **Secure App** (`OEMiROT_Appli_TrustZone`, Secure part)

    -   Import the same way the project located in
        `${XCUBE_PATH}/Applications/ROT_Lighting_OEMiRoT/OEMiROT_Appli_TrustZone/STM32CubeIDE`
        and select the **Secure** configuration
    -   Right-click the Secure project > **Build Project**
    -   Produces `secure_nsclib.o` (Secure Gateway veneers) in
        `OEMiROT_Appli_TrustZone/Secure_nsclib`, which is required by the
        NonSecure GN build (see
        [Step 1](#step-1-copy-the-secure-veneers-secure_nsclibo))

> On Linux, run the `dos2unix` / `chmod +x` step above **before** launching
> these builds, otherwise the post-build scripts will fail.

| Component    | Project Location                                             | Output                             |
| ------------ | ------------------------------------------------------------ | ---------------------------------- |
| OEMiROT Boot | `ROT_Lighting_OEMiRoT/OEMiROT_Boot`                          | `bl2.bin`                          |
| Secure App   | `ROT_Lighting_OEMiRoT/OEMiROT_Appli_TrustZone/Secure_nsclib` | `secure_nsclib.o`                  |
| Scripts      | `ROT_Provisioning_Lighting_OEMiRoT/OEMiROT/`                 | `regression.sh`, `provisioning.sh` |

## Build Instructions

### Step 1: Copy the Secure veneers (`secure_nsclib.o`)

The GN build links against the Secure Gateway veneers produced by the Secure
project built in STM32CubeIDE. Copy the object file into the GN platform folder:

```bash
cp ${XCUBE_PATH}/Applications/ROT_Lighting_OEMiRoT/OEMiROT_Appli_TrustZone/Secure_nsclib/secure_nsclib.o \
   connectedhomeip/examples/platform/stm32/STM32WBA65I-DK1/oemirot/Secure_nsclib/
```

> Repeat this step every time the Secure application is rebuilt, otherwise the
> NonSecure application will call outdated veneers.

### Step 2: Build with GN

```bash
cd connectedhomeip
source scripts/activate.sh
```

Modify in `args.gni`: `chip_enable_ota_requestor = true` and compile the
project:

```bash
./scripts/build/build_examples.py --target stm32-STM32WBA65I-DK1-light build
```

### Step 3: Copy Binary

> **IMPORTANT**: The signing XML files expect a hardcoded name:
> `oemirot_tz_ns_app.bin`

```bash
cp out/oemirot/STM32WBA65I-DK1-lighting.bin \
   ${XCUBE_PATH}/Applications/ROT_Lighting_OEMiRoT/OEMiROT_Appli_TrustZone/Binary/oemirot_tz_ns_app.bin
```

### Step 4: Sign the Binary

#### Option A: Using CLI

```bash
TPC="path/to/STM32CubeProgrammer/bin/STM32TrustedPackageCreator_CLI"

# Sign for OTA (encrypted + signed)
${TPC} -pb "${PROV_PATH}/OEMiROT/Images/OEMiROT_NS_Code_Image.xml"

# Sign for initial provisioning (signed only)
${TPC} -pb "${PROV_PATH}/OEMiROT/Images/OEMiROT_NS_Code_Init_Image.xml"
```

#### Option B: Using GUI

1. Open STM32TrustedPackageCreator (from STM32CubeProgrammer installation)
2. Go to **Security** tab -> **Image Gen** window
3. Load `OEMiROT_NS_Code_Image.xml` and click **Generate** (for OTA)
4. Load `OEMiROT_NS_Code_Init_Image.xml` and click **Generate** (for
   provisioning)

### Step 5: Flash the Board

```bash
cd ${PROV_PATH}/OEMiROT

# REQUIRED: Reset option bytes before EACH provisioning
./regression.sh

# Flash all components
./provisioning.sh
```

---

## OTA Update (N+1 Image)

To generate an OTA update image with incremented version:

### Step 1: Increment the software version

Edit the version in your application config:

**`CHIPProjectConfig.h`** and **`oemirot/../app_conf.h`**:

```c
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION         2
#define X_CUBE_MATTER_VERSION  "x.y.z"
```

### Step 2: Rebuild with GN

```bash
./scripts/build/build_examples.py --target stm32-STM32WBA65I-DK1-light build
```

### Step 3: Copy and sign for OTA

```bash
cp out/oemirot/STM32WBA65I-DK1-lighting.bin \
   ${XCUBE_PATH}/Applications/ROT_Lighting_OEMiRoT/OEMiROT_Appli_TrustZone/Binary/oemirot_tz_ns_app.bin

# Sign for OTA (encrypted + signed) - generates oemirot_tz_ns_app_enc_sign.bin
${TPC} -pb "${PROV_PATH}/OEMiROT/Images/OEMiROT_NS_Code_Image.xml"
```

### Step 4: Create Matter binary with ST header

Use `CreateMatterBin.py` from X-CUBE-MATTER (`Utilities/OTA_Tools/`):

```bash
python CreateMatterBin.py -bin1 oemirot_tz_ns_app_enc_sign.bin -o my-firmware.bin
```

This adds the ST header required for OTA.

### Step 5: Create the Matter OTA file (.ota)

Tools are located in: `Utilities/OTA_Tools/`

```bash
python ota_image_tool.py create -v 0xFFF1 -p 0x8004 -vn 2 -vs "1.1" -da sha256 \
       my-firmware.bin my-firmware.ota
```

| Parameter | Description                                                         |
| --------- | ------------------------------------------------------------------- |
| `-v`      | Vendor ID (e.g., 0xFFF1 for test)                                   |
| `-p`      | Product ID (e.g., 0x8004)                                           |
| `-vn`     | Version number (e.g., `CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION`) |
| `-vs`     | Version string (e.g., `X_CUBE_MATTER_VERSION`)                      |
| `-da`     | Digest algorithm (sha256)                                           |

### Step 6: Use the OTA image

The final OTA image is: `my-firmware.ota`

Use this file with your OTA provider (e.g., chip-tool, ota-provider-app).

---

## Quick Reference

| Item                 | Value                                                   |
| -------------------- | ------------------------------------------------------- |
| Build flag           | `chip_enable_ota_requestor=true`                        |
| Expected binary name | `oemirot_tz_ns_app.bin` (hardcoded in XML)              |
| Regression           | MUST run before each provisioning (resets option bytes) |

**Output binaries after signing:**

-   `oemirot_tz_ns_app_enc_sign.bin` - for OTA updates
-   `oemirot_tz_ns_app_init_sign.bin` - for initial provisioning

**Useful links:**

-   [X-CUBE-MATTER | Product - STMicroelectronics](https://www.st.com/en/embedded-software/x-cube-matter.html)
-   [Discover Matter - stm32mcu wiki](https://wiki.st.com/stm32mcu/wiki/Connectivity:Introduction_to_Matter)
