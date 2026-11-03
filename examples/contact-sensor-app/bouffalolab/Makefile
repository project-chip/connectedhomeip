# =============================================================================
# SDK & Environment
# =============================================================================

SDK_DEMO_PATH ?= $(abspath .)
CHIP_ROOT      = ../../..
BL_SDK_BASE    = ${CHIP_ROOT}/third_party/bouffalolab/repo_bouffalo_sdk
export BL_SDK_BASE

CROSS_COMPILE ?= riscv64-unknown-elf-

# The SDK's ninja target shells out to ninja from a make recipe.  Do not pass
# GNU make jobserver MAKEFLAGS to ninja; ninja warns on pipe-based jobserver
# values such as "--jobserver-auth=3,4".
unexport MAKEFLAGS

# =============================================================================
# Target Chip & Board
# =============================================================================

CHIP  ?= bl616
BOARD ?= ${CHIP}dk

# =============================================================================
# Network Interface
# =============================================================================

CONFIG_WIFI     ?= y
CONFIG_THREAD   ?= n
CONFIG_ETHERNET ?= n

# =============================================================================
# Optional Features
# =============================================================================

CONFIG_SHELL                  ?= n
CONFIG_MFD                    ?= y
CONFIG_CHIP_ROTATING_DEVICE_ID ?= n
CONFIG_CHIP_HEAP_MONITOR       ?= n
CONFIG_COREDUMP                ?= n

# =============================================================================
# cmake Definition
# =============================================================================

cmake_definition += -DCHIP_ROOT=${CHIP_ROOT}
cmake_definition += -DBL_SDK_BASE=${BL_SDK_BASE}
cmake_definition += -DCONFIG_WIFI=y

cmake_definition += -DCONFIG_SHELL=${CONFIG_SHELL}
cmake_definition += -DCONFIG_MFD=${CONFIG_MFD}
cmake_definition += -DCONFIG_CHIP_ROTATING_DEVICE_ID=${CONFIG_CHIP_ROTATING_DEVICE_ID}
cmake_definition += -DCONFIG_CHIP_HEAP_MONITOR=${CONFIG_CHIP_HEAP_MONITOR}
cmake_definition += -DCONFIG_COREDUMP=${CONFIG_COREDUMP}

# =============================================================================
# SDK Build
# =============================================================================

include $(BL_SDK_BASE)/project.build

# =============================================================================
# Flash Target (with optional MFD)
# =============================================================================

ifneq ($(MFD_FILE),)
MFD_FILE_ABS := $(if $(wildcard $(MFD_FILE)),$(abspath $(MFD_FILE)),$(abspath $(CHIP_ROOT)/$(MFD_FILE)))
command_flash_without_mfd := $(command_flash)
override command_flash = test -f "$(MFD_FILE_ABS)" || { echo "[flash] MFD file not found: $(MFD_FILE)"; exit 1; }; \
	test -d "$(BUILD_DIR)/build_out" || { echo "[flash] BUILD_DIR not found: $(BUILD_DIR)"; exit 1; }; \
	echo "[flash] adding MFD bin: $(MFD_FILE_ABS)"; \
	echo "[flash] using build output: $(abspath $(BUILD_DIR)/build_out)"; \
	sed 's|\./build/build_out|./$(BUILD_DIR)/build_out|g' flash_prog_cfg.ini > flash_prog_cfg.mfd.ini; \
	trap 'rm -f flash_prog_cfg.mfd.ini' EXIT; \
	printf '\n[MFD]\nfiledir = $(MFD_FILE_ABS)\naddress = @partition\n' >> flash_prog_cfg.mfd.ini; \
	$(subst --config=flash_prog_cfg.ini,--config=flash_prog_cfg.mfd.ini,$(command_flash_without_mfd))
endif
