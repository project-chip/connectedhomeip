#include "chip-zcl.h"
#include "gen-types.h"
#include <stdio.h>

// -----------------------------------------------------------------------------
// On/off cluster (0x0006).

// On/off cluster client (0x0006).
const ChipZclClusterSpec_t chipZclClusterOnOffClientSpec = {
    CHIP_ZCL_ROLE_CLIENT,
    CHIP_ZCL_MANUFACTURER_CODE_NULL,
    CHIP_ZCL_CLUSTER_ON_OFF,
};

// On/off cluster server (0x0006).
const ChipZclClusterSpec_t chipZclClusterOnOffServerSpec = {
    CHIP_ZCL_ROLE_SERVER,
    CHIP_ZCL_MANUFACTURER_CODE_NULL,
    CHIP_ZCL_CLUSTER_ON_OFF,
};
