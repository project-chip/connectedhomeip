# OEMiROT GN Build - STM32WBA65I-DK1

## Matter Lighting Application

User guide for building the Matter Lighting NonSecure application with GN builds for OEMiROT (secure boot with TrustZone).

---

## Flash Memory Layout

| Address | Component | Build Tool |
|---------|-----------|------------|
| 0x08000000 | OEMiROT Boot | STM32CubeIDE (X-CUBE-MATTER) |
| 0x0801C000 | Secure App | STM32CubeIDE (X-CUBE-MATTER) |
| 0x0801E400 | NonSecure App | GN Build **<-- THIS GUIDE** |

---

## Prerequisites

### Required Tools

- ARM GCC Toolchain (arm-none-eabi-gcc in PATH)
- GN and Ninja build system
- Python 3
- STM32CubeProgrammer

### Pre-compiled from X-CUBE-MATTER (STM32CubeIDE)

| Component | Project Location | Output |
|-----------|------------------|--------|
| OEMiROT Boot | `ROT_Lighting_OEMiRoT/OEMiROT_Boot` | `bl2.bin` |
| Secure App | `ROT_Lighting_OEMiRoT/OEMiROT_Appli_TrustZone/Secure_nsclib` | `secure_nsclib.o` |
| Scripts | `ROT_Provisioning_Lighting_OEMiRoT/OEMiROT/` | `regression.sh`, `provisioning.sh` |

---

## Build Instructions

### Step 1: Build with GN

```bash
cd connectedhomeip
source scripts/activate.sh
```

Modify in `args.gni`: `chip_enable_ota_requestor = true` and compile the project:

```bash
./scripts/build/build_examples.py --target stm32-STM32WBA65I-DK1-light build
```

### Step 2: Copy Binary

> **IMPORTANT**: The signing XML files expect a hardcoded name: `oemirot_tz_ns_app.bin`

```bash
XCUBE_PATH="path/to/Projects/STM32WBA65I-DK1"

cp out/oemirot/STM32WBA65I-DK1-lighting.bin \
   ${XCUBE_PATH}/Applications/ROT_Lighting_OEMiRoT/OEMiROT_Appli_TrustZone/Binary/oemirot_tz_ns_app.bin
```

### Step 3: Sign the Binary

#### Option A: Using CLI

```bash
PROV_PATH="${XCUBE_PATH}/ROT_Provisioning_Lighting_OEMiRoT"
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
4. Load `OEMiROT_NS_Code_Init_Image.xml` and click **Generate** (for provisioning)

### Step 4: Flash the Board

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

**CHIPProjectConfig.h** and **oemirot/../app_conf.h**:
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

| Parameter | Description |
|-----------|-------------|
| `-v` | Vendor ID (e.g., 0xFFF1 for test) |
| `-p` | Product ID (e.g., 0x8004) |
| `-vn` | Version number (e.g., `CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION`) |
| `-vs` | Version string (e.g., `X_CUBE_MATTER_VERSION`) |
| `-da` | Digest algorithm (sha256) |

### Step 6: Use the OTA image

The final OTA image is: `my-firmware.ota`

Use this file with your OTA provider (e.g., chip-tool, ota-provider-app).

---

## Quick Reference

| Item | Value |
|------|-------|
| Build flag | `chip_enable_ota_requestor=true` |
| Expected binary name | `oemirot_tz_ns_app.bin` (hardcoded in XML) |
| Regression | MUST run before each provisioning (resets option bytes) |

**Output binaries after signing:**
- `oemirot_tz_ns_app_enc_sign.bin` - for OTA updates
- `oemirot_tz_ns_app_init_sign.bin` - for initial provisioning

---

## Signing Keys

Location: `ROT_Provisioning_Lighting_OEMiRoT/OEMiROT/Keys/`

| Key | Description |
|-----|-------------|
| `OEMiRoT_Authentication_NS.pem` | NonSecure signing key |
| `OEMiRoT_Authentication_S.pem` | Secure signing key |
| `OEMiRoT_Encryption.pem` | Encryption key (private) |
| `OEMiRoT_Encryption_Pub.pem` | Encryption key (public) |

> **WARNING**: These keys are for DEVELOPMENT ONLY. Generate your own keys for production!

---

## Troubleshooting

### Error: "secure_nsclib.o not found"
- Build Secure project in STM32CubeIDE first
- Copy `secure_nsclib.o` to `ROT_Lighting_OEMiRoT/OEMiROT_Appli_TrustZone/Secure_nsclib`

### Error: "Image verification failed" at boot
- Verify binary signed with correct keys
- Run `regression.sh` then `provisioning.sh`

### Error: "undefined reference to _sidata"
- Linker script issue, verify preprocessing worked

### Error: Signing fails "file not found"
- Binary must be named exactly: `oemirot_tz_ns_app.bin`
- Binary must be in: `Applications/ROT_Lighting_OEMiRoT/OEMiROT_Appli_TrustZone/Binary/`

---

## References

- [X-CUBE-MATTER](https://www.st.com/en/embedded-software/x-cube-matter.html)
- [Wiki pages](https://wiki.st.com/stm32mcu/wiki/Category:Matter)
