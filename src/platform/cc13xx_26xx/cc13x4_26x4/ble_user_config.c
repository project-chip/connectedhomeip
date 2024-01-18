/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file
 *  This file contains user configurable variables for the BLE Application.
 *
 */

/*******************************************************************************
 * INCLUDES
 */

#include "ble_user_config.h"
#include "hal_types.h"
#ifndef FREERTOS
#include <ti/sysbios/BIOS.h>
#endif
#ifdef SYSCFG
#include "ti_ble_config.h"
#else
#include <gapbondmgr.h>
#endif

#ifndef CC23X0
#include "ble_overrides.h"
#include "ti_radio_config.h"

#include "ecc/ECCROMCC26XX.h"
#include <dmm/dmm_rfmap.h>
#include <ti/drivers/TRNG.h>
#include <ti/drivers/aesccm/AESCCMCC26XX.h>
#include <ti/drivers/aesecb/AESECBCC26XX.h>
#else
#include <ti/drivers/RNG.h>
#include <ti/drivers/aesccm/AESCCMCC23XX.h>
#include <ti/drivers/aesecb/AESECBCC23XX.h>
#endif

#include <ti/drivers/AESCCM.h>
#include <ti/drivers/AESECB.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>

#ifdef FREERTOS
#define Swi_restore SwiP_restore
#define Swi_disable SwiP_disable
#include <ti/drivers/dpl/SwiP.h>
#else
#include <ti/sysbios/knl/Swi.h>
#endif
#include <ti/drivers/utils/Random.h>

#ifndef CC23X0
#if !defined(DeviceFamily_CC26X1)
#include <driverlib/pka.h>
#if !defined(DeviceFamily_CC13X4) && !defined(DeviceFamily_CC26X4)
#include <driverlib/rf_bt5_coex.h>
#endif
#else
#include <driverlib/rom_ecc.h>
#endif
#endif

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

// Tx Power
#define NUM_TX_POWER_VALUES (RF_BLE_TX_POWER_TABLE_SIZE - 1)

#ifndef SYSCFG
// Default Tx Power Index
#if defined(CC13X2P)
#define DEFAULT_TX_POWER HCI_EXT_TX_POWER_0_DBM
#else // !CC13X2
#define DEFAULT_TX_POWER HCI_EXT_TX_POWER_0_DBM
#endif // CC13X2
#endif // SYSCFG

// Override NOP
#define OVERRIDE_NOP 0xC0000001

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

void driverTable_fnSpinlock(void);

/*******************************************************************************
 * GLOBAL VARIABLES
 */

#ifndef CC23X0
// Tx Power Table
txPwrTbl_t appTxPwrTbl = { (txPwrVal_t *) RF_BLE_txPowerTable,
                           NUM_TX_POWER_VALUES, // max
                           DEFAULT_TX_POWER };  // default
#endif
#if defined(CC13X2P) && defined(CC13X2P_2_LAUNCHXL)

// Tx Power Backoff Values (txPwrBackoff1MPhy,txPwrBackoff2MPhy ,txPwrBackoffCoded)
const txPwrBackoffVal_t TxPowerBackoffTable[] = {
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 0
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 1
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 2
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 3
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 4
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 5
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 6
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 7
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 8
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 9
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 10
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 11
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 12
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 13
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 14
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 15
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 16
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 17
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 18
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 19
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 20
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 21
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 22
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 23
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 24
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 25
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 26
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 27
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 28
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 29
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 30
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 31
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 32
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 33
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_17_DBM_P4_9_DBM, HCI_EXT_TX_POWER_P2_20_DBM },  // max tx power for channel 34
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_18_DBM_P4_10_DBM, HCI_EXT_TX_POWER_P2_20_DBM }, // max tx power for channel 35
    { HCI_EXT_TX_POWER_P2_19_DBM, HCI_EXT_TX_POWER_P2_18_DBM_P4_10_DBM, HCI_EXT_TX_POWER_P2_20_DBM }, // max tx power for channel 36
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 37
    { HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM },           // max tx power for channel 38
    { HCI_EXT_TX_POWER_P2_14_DBM_P4_6_DBM, HCI_EXT_TX_POWER_P2_20_DBM, HCI_EXT_TX_POWER_P2_20_DBM }
}; // max tx power for channel 39

txPwrBackoffTbl_t appTxPwrBackoffTbl = { TxPowerBackoffTable };

#endif // defined(CC13X2P) && defined(CC13X2P_2_LAUNCHXL)

// Antenna board configurations (example for a 12-antenna board)
// Maximum number of antennas
#define ANTENNA_TABLE_SIZE 12
// BitMask of all the relevant GPIOs which needed for the antennas
#define ANTENNA_IO_MASK BV(27) | BV(28) | BV(29) | BV(30)

// Antenna GPIO configuration (should be adapted to the antenna board design)
antennaIOEntry_t antennaTbl[ANTENNA_TABLE_SIZE] = {
    0,                        // antenna 0 GPIO configuration (all GPIOs in ANTENNA_IO_MASK are LOW)
    BV(28),                   // antenna 1
    BV(29),                   // antenna 2
    BV(28) | BV(29),          // antenna 3
    BV(30),                   // antenna 4
    BV(28) | BV(30),          // antenna 5
    BV(27),                   // antenna 6
    BV(27) | BV(28),          // antenna 7
    BV(27) | BV(29),          // antenna 8
    BV(27) | BV(28) | BV(29), // antenna 9
    BV(27) | BV(30),          // antenna 10
    BV(27) | BV(28) | BV(30)  // antenna 11
};

#ifdef RTLS_CTE
// Antenna properties passes to the stack
cteAntProp_t appCTEAntProp = { ANTENNA_IO_MASK, ANTENNA_TABLE_SIZE, antennaTbl };
#endif

#ifdef CC23X0
ECCParams_CurveParams eccParams_NISTP256 = { .curveType  = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3,
                                             .length     = ECCParams_NISTP256_LENGTH,
                                             .prime      = ECC_NISTP256_prime.byte,
                                             .order      = ECC_NISTP256_order.byte,
                                             .a          = ECC_NISTP256_a.byte,
                                             .b          = ECC_NISTP256_b.byte,
                                             .generatorX = ECC_NISTP256_generatorX.byte,
                                             .generatorY = ECC_NISTP256_generatorY.byte,
                                             .cofactor   = 1 };
#elif !defined(DeviceFamily_CC26X1)
ECCParams_CurveParams eccParams_NISTP256 = { .curveType  = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3,
                                             .length     = NISTP256_PARAM_SIZE_BYTES,
                                             .prime      = NISTP256_prime.byte,
                                             .order      = NISTP256_order.byte,
                                             .a          = NISTP256_a.byte,
                                             .b          = NISTP256_b.byte,
                                             .generatorX = NISTP256_generator.x.byte,
                                             .generatorY = NISTP256_generator.y.byte,
                                             .cofactor   = 1 };
#else
ECCParams_CurveParams eccParams_NISTP256 = { .curveType  = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS_AN3,
                                             .length     = ECC_NISTP256_PARAM_LENGTH_BYTES,
                                             .prime      = ECC_NISTP256_prime.byte,
                                             .order      = ECC_NISTP256_order.byte,
                                             .a          = ECC_NISTP256_a.byte,
                                             .b          = ECC_NISTP256_b.byte,
                                             .generatorX = ECC_NISTP256_generatorX.byte,
                                             .generatorY = ECC_NISTP256_generatorY.byte,
                                             .cofactor   = 1 };
#endif

#if defined(USE_COEX)
// 2 structures generated by the sysconfig RF for coexistence configuration
extern rfCoreHal_bleCoExConfig_t coexConfig;
extern RF_CoexOverride_BLEUseCases coexConfigBle;
// from coexConfig struct we will get the coex enable and coex type (3 or 1 wire)
// from coexConfigBle struct we will get the priority and rx request per use case:
// initiator, connected, Broadcaster and Observer.
coexUseCaseConfig_t coexSysConfig = { &coexConfig, &coexConfigBle };
#endif

#ifdef ICALL_JT
#include <icall.h>

#ifndef CC23X0
// RF Driver API Table
rfDrvTblPtr_t rfDriverTableBLE[] = {
    (uint32) RF_open,
    (uint32) driverTable_fnSpinlock, // RF_close
#ifdef RF_SINGLEMODE
    (uint32) RF_postCmd,
#else                                // !RF_SINGLEMODE
    (uint32) driverTable_fnSpinlock, // RF_postCmd
#endif                               // RF_SINGLEMODE
    (uint32) driverTable_fnSpinlock, // RF_pendCmd
#ifdef RF_SINGLEMODE
    (uint32) RF_runCmd,
#else  // !RF_SINGLEMODE
    (uint32) driverTable_fnSpinlock, // RF_runCmd
#endif // RF_SINGLEMODE
    (uint32) RF_cancelCmd,
    (uint32) RF_flushCmd,
    (uint32) driverTable_fnSpinlock, // RF_yield
    (uint32) RF_Params_init,
    (uint32) RF_runImmediateCmd,
    (uint32) RF_runDirectCmd,
    (uint32) RF_ratCompare,
    (uint32) driverTable_fnSpinlock, // RF_ratCapture
    (uint32) RF_ratDisableChannel,
    (uint32) RF_getCurrentTime,
    (uint32) RF_getRssi,
    (uint32) RF_getInfo,
    (uint32) RF_getCmdOp,
    (uint32) RF_control,
    (uint32) driverTable_fnSpinlock, // RF_getTxPower
    (uint32) RF_setTxPower,          // RF_setTxPower
    (uint32) driverTable_fnSpinlock, // RF_TxPowerTable_findPowerLevel
    (uint32) driverTable_fnSpinlock, // RF_TxPowerTable_findValue
#ifndef RF_SINGLEMODE
    (uint32) RF_scheduleCmd,
    (uint32) RF_runScheduleCmd,
    (uint32) driverTable_fnSpinlock, // RF_requestAccess
#endif                               // !RF_SINGLEMODE
};

cryptoDrvTblPtr_t cryptoDriverTableBLE[] = { (uint32) AESCCM_init,
                                             (uint32) AESCCM_open,
                                             (uint32) AESCCM_close,
                                             (uint32) AESCCM_Params_init,
                                             (uint32) AESCCM_Operation_init,
                                             (uint32) AESCCM_oneStepEncrypt,
                                             (uint32) AESCCM_oneStepDecrypt,
                                             (uint32) AESECB_init,
                                             (uint32) AESECB_open,
                                             (uint32) AESECB_close,
                                             (uint32) AESECB_Params_init,
                                             (uint32) AESECB_Operation_init,
                                             (uint32) AESECB_oneStepEncrypt,
                                             (uint32) AESECB_oneStepDecrypt,
                                             (uint32) CryptoKeyPlaintext_initKey,
                                             (uint32) CryptoKeyPlaintext_initBlankKey };

// Swi APIs needed by BLE controller
rtosApiTblPtr_t rtosApiTable[] = { (uint32_t) Swi_disable, (uint32_t) Swi_restore };
#endif
// BLE Stack Configuration Structure
const stackSpecific_t bleStackConfig = { .maxNumConns        = MAX_NUM_BLE_CONNS,
                                         .maxNumPDUs         = MAX_NUM_PDU,
                                         .maxPduSize         = 0,
                                         .maxNumPSM          = L2CAP_NUM_PSM,
                                         .maxNumCoChannels   = L2CAP_NUM_CO_CHANNELS,
                                         .maxWhiteListElems  = MAX_NUM_WL_ENTRIES,
                                         .maxResolvListElems = CFG_MAX_NUM_RL_ENTRIES,
                                         .pfnBMAlloc         = &pfnBMAlloc,
                                         .pfnBMFree          = &pfnBMFree,
#ifndef CC23X0
                                         .rfDriverParams.powerUpDurationMargin = RF_POWER_UP_DURATION_MARGIN,
                                         .rfDriverParams.inactivityTimeout     = RF_INACTIVITY_TIMEOUT,
                                         .rfDriverParams.powerUpDuration       = RF_POWER_UP_DURATION,
                                         .rfDriverParams.pErrCb                = &(RF_ERR_CB),
#endif
                                         .eccParams         = &eccParams_NISTP256,
                                         .fastStateUpdateCb = NULL,
                                         .bleStackType      = 0,
#ifdef CC2652RB_LAUNCHXL
                                         .extStackSettings = EXTENDED_STACK_SETTINGS | CC2652RB_OVERRIDE_USED,
#else
  .extStackSettings                     = EXTENDED_STACK_SETTINGS,
#endif
#ifndef CC23X0
                                         .maxNumCteBuffers    = MAX_NUM_CTE_BUFS,
                                         .advReportIncChannel = ADV_RPT_INC_CHANNEL
#endif
};

uint16_t bleUserCfg_maxPduSize = MAX_PDU_SIZE;

#ifdef OSAL_SNV_EXTFLASH
const extflashDrvTblPtr_t extflashDriverTable[] = { (uint32) ExtFlash_open, (uint32) ExtFlash_close, (uint32) ExtFlash_read,
                                                    (uint32) ExtFlash_write, (uint32) ExtFlash_erase };
#endif // OSAL_SNV_EXTFLASH

#ifndef CC23X0
// Table for Driver can be found in icall_user_config.c
// if a driver is not to be used, then the pointer shoul dbe set to NULL,
// for this example, this is done in ble_user_config.h
const drvTblPtr_t driverTable = {
    .rfDrvTbl        = rfDriverTableBLE,
    .eccDrvTbl       = eccDriverTable,
    .cryptoDrvTbl    = cryptoDriverTableBLE,
    .trngDrvTbl      = trngDriverTable,
    .rtosApiTbl      = rtosApiTable,
    .nvintfStructPtr = &nvintfFncStruct,
#ifdef OSAL_SNV_EXTFLASH
    .extflashDrvTbl = extflashDriverTable,
#endif // OSAL_SNV_EXTFLASH
};
#endif

const boardConfig_t boardConfig = {
#ifndef CC23X0
    .rfFeModeBias  = RF_FE_MODE_AND_BIAS,
    .rfRegTbl      = (regOverride_t *) pOverrides_bleCommon,
    .rfRegTbl1M    = (regOverride_t *) pOverrides_ble1Mbps,
    .rfRegTbl2M    = (regOverride_t *) pOverrides_ble2Mbps,
    .rfRegTblCoded = (regOverride_t *) pOverrides_bleCoded,
    .txPwrTbl      = &appTxPwrTbl,
#if defined(CC13X2P) || defined(EM_CC1354P10_1_LP)
#if defined(CC13X2P_2_LAUNCHXL)
    .txPwrBackoffTbl = &appTxPwrBackoffTbl,
#else
    .txPwrBackoffTbl          = NULL,
#endif // defined(CC13X2P_2_LAUNCHXL)
#if defined(EM_CC1354P10_1_LP)
    .rfRegOverrideTxStdTblptr = NULL,
    .rfRegOverrideTx20TblPtr  = NULL,
#else
    .rfRegOverrideTxStdTblptr = (regOverride_t *) pOverrides_bleTxStd, // Default PA
    .rfRegOverrideTx20TblPtr  = (regOverride_t *) pOverrides_bleTx20,  // High power PA
#endif // EM_CC1354P10_1_LP
#endif // CC13X2P
#if defined(RTLS_CTE)
    .rfRegOverrideCtePtr = (regOverride_t *) (pOverrides_bleCommon + BLE_STACK_OVERRIDES_OFFSET + CTE_OVERRIDES_OFFSET),
    .cteAntennaPropPtr   = &appCTEAntProp,
#else
    .rfRegOverrideCtePtr  = NULL,
    .cteAntennaPropPtr    = NULL,
#endif
    .privOverrideOffset = BLE_STACK_OVERRIDES_OFFSET + PRIVACY_OVERRIDE_OFFSET,
#if defined(USE_COEX)
    .coexUseCaseConfigPtr = &coexSysConfig,
#else
    .coexUseCaseConfigPtr = NULL,
#endif
#endif
};

#else /* !(ICALL_JT) */

#ifndef CC23X0
// RF Driver API Table
rfDrvTblPtr_t rfDriverTable[] = {
    (uint32) RF_open,
    (uint32) driverTable_fnSpinlock, // RF_close
#ifdef RF_SINGLEMODE
    (uint32) RF_postCmd,
#else  // !RF_SINGLEMODE
    (uint32) driverTable_fnSpinlock, // RF_postCmd
#endif // RF_SINGLEMODE
    (uint32) driverTable_fnSpinlock, // RF_pendCmd
#ifdef RF_SINGLEMODE
    (uint32) RF_runCmd,
#else  // !RF_SINGLEMODE
    (uint32) driverTable_fnSpinlock, // RF_runCmd
#endif // RF_SINGLEMODE
    (uint32) RF_cancelCmd,
    (uint32) RF_flushCmd,
    (uint32) driverTable_fnSpinlock, // RF_yield
    (uint32) RF_Params_init,
    (uint32) RF_runImmediateCmd,
    (uint32) RF_runDirectCmd,
    (uint32) RF_ratCompare(uint32) driverTable_fnSpinlock, // RF_ratCapture
    (uint32) driverTable_fnSpinlock,                       // RF_ratDisableChannel
    (uint32) RF_getCurrentTime,
    (uint32) RF_getRssi,
    (uint32) RF_getInfo,
    (uint32) RF_getCmdOp,
    (uint32) RF_control,
    (uint32) driverTable_fnSpinlock, // RF_getTxPower
    (uint32) RF_setTxPower,          // RF_setTxPower
    (uint32) driverTable_fnSpinlock, // RF_TxPowerTable_findPowerLevel
    (uint32) driverTable_fnSpinlock, // RF_TxPowerTable_findValue
#ifndef RF_SINGLEMODE
    (uint32) RF_scheduleCmd,
    (uint32) RF_runScheduleCmd,
    (uint32) driverTable_fnSpinlock // RF_requestAccess
#endif // !RF_SINGLEMODE
};

// ECC Driver API Table
eccDrvTblPtr_t eccDriverTable[] = { (uint32) ECDH_init,
                                    (uint32) ECDH_Params_init,
                                    (uint32) ECDH_open,
                                    (uint32) ECDH_close,
                                    (uint32) ECDH_OperationGeneratePublicKey_init,
                                    (uint32) ECDH_OperationComputeSharedSecret_init,
                                    (uint32) ECDH_generatePublicKey,
                                    (uint32) ECDH_computeSharedSecret };

// Crypto Driver API Table
cryptoDrvTblPtr_t cryptoDriverTable[] = { (uint32) AESCCM_init,
                                          (uint32) AESCCM_open,
                                          (uint32) AESCCM_close,
                                          (uint32) AESCCM_Params_init,
                                          (uint32) AESCCM_Operation_init,
                                          (uint32) AESCCM_oneStepEncrypt,
                                          (uint32) AESCCM_oneStepDecrypt,
                                          (uint32) AESECB_init,
                                          (uint32) AESECB_open,
                                          (uint32) AESECB_close,
                                          (uint32) AESECB_Params_init,
                                          (uint32) AESECB_Operation_init,
                                          (uint32) AESECB_oneStepEncrypt,
                                          (uint32) AESECB_oneStepDecrypt,
                                          (uint32) CryptoKeyPlaintext_initKey,
                                          (uint32) CryptoKeyPlaintext_initBlankKey };

trngDrvTblPtr_t trngDriverTable[] = { (uint32) TRNG_init, (uint32) TRNG_open, (uint32) TRNG_generateEntropy, (uint32) TRNG_close };
#endif

#endif /* ICALL_JT */

/*******************************************************************************
 * @fn          RegisterAssertCback
 *
 * @brief       This routine registers the Application's assert handler.
 *
 * input parameters
 *
 * @param       appAssertHandler - Application's assert handler.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void RegisterAssertCback(assertCback_t appAssertHandler)
{
    appAssertCback = appAssertHandler;

#ifdef EXT_HAL_ASSERT
    // also set the Assert callback pointer used by halAssertHandlerExt
    // Note: Normally, this pointer will be intialized by the stack, but in the
    //       event HAL_ASSERT is used by the Application, we initialize it
    //       directly here.
    halAssertCback = appAssertHandler;
#endif // EXT_HAL_ASSERT

    return;
}

/*******************************************************************************
 * @fn          driverTable_fnSpinLock
 *
 * @brief       This routine is used to trap calls to unpopulated indexes of
 *              driver function pointer tables.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void driverTable_fnSpinlock(void)
{
    volatile uint8 i = 1;

    while (i)
        ;
}

/*******************************************************************************
 * @fn          DefaultAssertCback
 *
 * @brief       This is the Application default assert callback, in the event
 *              none is registered.
 *
 * input parameters
 *
 * @param       assertCause    - Assert cause as defined in hal_assert.h.
 * @param       assertSubcause - Optional assert subcause (see hal_assert.h).
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void DefaultAssertCback(uint8 assertCause, uint8 assertSubcause)
{
#ifdef HAL_ASSERT_SPIN
    driverTable_fnSpinlock();
#endif // HAL_ASSERT_SPIN

    return;
}

// Application Assert Callback Function Pointer
assertCback_t appAssertCback = DefaultAssertCback;

/*******************************************************************************
 */
