# =============================================================================
# SDK & Environment
# =============================================================================

SDK_DEMO_PATH ?= $(abspath .)
CHIP_ROOT      = ../../..
BL_SDK_BASE    = ${CHIP_ROOT}/third_party/bouffalolab/repo_bouffalo_sdk
export BL_SDK_BASE

CROSS_COMPILE ?= riscv64-unknown-elf-

# =============================================================================
# Target Chip & Board
# =============================================================================

CHIP  ?= bl616
BOARD ?= ${CHIP}dk

# =============================================================================
# Network Interface (at least one must be y)
# =============================================================================

ifeq ($(origin CONFIG_WIFI)$(origin CONFIG_THREAD)$(origin CONFIG_ETHERNET),undefinedundefinedundefined)
  CONFIG_WIFI ?= y
else
  CONFIG_WIFI ?= n
endif
CONFIG_THREAD   ?= n
CONFIG_ETHERNET ?= n

ifeq ($(words $(filter y,$(CONFIG_WIFI) $(CONFIG_THREAD) $(CONFIG_ETHERNET))),0)
  $(error At least one network interface must be enabled: CONFIG_WIFI=y, CONFIG_THREAD=y, or CONFIG_ETHERNET=y)
endif

# =============================================================================
# Thread Configuration
# =============================================================================

CONFIG_OT_FTD ?= 1
CONFIG_OT_MTD ?= 0

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

# Basic
cmake_definition += -DCHIP_ROOT=${CHIP_ROOT}
cmake_definition += -DBL_SDK_BASE=${BL_SDK_BASE}

# Network interface
cmake_definition += -DCONFIG_WIFI=${CONFIG_WIFI}
cmake_definition += -DCONFIG_THREAD=${CONFIG_THREAD}
cmake_definition += -DCONFIG_ETHERNET=${CONFIG_ETHERNET}

# Optional features
cmake_definition += -DCONFIG_SHELL=${CONFIG_SHELL}
cmake_definition += -DCONFIG_MFD=${CONFIG_MFD}
cmake_definition += -DCONFIG_CHIP_ROTATING_DEVICE_ID=${CONFIG_CHIP_ROTATING_DEVICE_ID}
cmake_definition += -DCONFIG_CHIP_HEAP_MONITOR=${CONFIG_CHIP_HEAP_MONITOR}
cmake_definition += -DCONFIG_COREDUMP=${CONFIG_COREDUMP}

# Thread-specific
ifeq ($(CONFIG_THREAD),y)
  cmake_definition += -DCONFIG_OT_FTD=${CONFIG_OT_FTD}
  cmake_definition += -DCONFIG_OT_MTD=${CONFIG_OT_MTD}
  cmake_definition += -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE=\"openthread-core-proj-config.h\"
  cmake_definition += -DOPENTHREAD_CORE_CONFIG_PLATFORM_CHECK_FILE=\"openthread-core-platform-config-check.h\"
  ifeq ($(CONFIG_SHELL),y)
    cmake_definition += -DCONFIG_OT_CLI=shell
    cmake_definition += -DCONFIG_SHELL_CMD_SIZE=256
  endif
endif

# Ethernet-specific: eth_phy and lwip EMAC driver, needed by NetworkCommissioningEthernetDriver
ifeq ($(CONFIG_ETHERNET),y)
  cmake_definition += -DCONFIG_BSP_ETH_PHY=y
  cmake_definition += -DCONFIG_BSP_LWIP_NETIF_EMAC=y
endif

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
