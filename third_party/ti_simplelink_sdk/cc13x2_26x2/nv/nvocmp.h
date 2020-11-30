/******************************************************************************

 @file  nvocmp.h

 @brief NV definitions for CC26xx devices - On-Chip Multi-Page Flash Memory

 Group: CMCU, LPC
 Target Device: cc13x2_26x2

 ******************************************************************************

 Copyright (c) 2017-2020, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************


 *****************************************************************************/

#ifndef NVOCMP_H
#define NVOCMP_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef NV_LINUX
#include <ti/drivers/NVS.h>
#endif

#include "nvintf.h"

//*****************************************************************************
// Constants and definitions
//*****************************************************************************

// NV driver item ID definitions
#define NVOCMP_NVID_DIAG {NVINTF_SYSID_NVDRVR, 1, 0}

//*****************************************************************************
// Typedefs
//*****************************************************************************

// NV driver diagnostic data
typedef struct
{
    uint32_t compacts;  // Number of page compactions
    uint16_t resets;    // Number of driver resets (power on)
    uint16_t available; // Number of available bytes after last compaction
    uint16_t active;    // Number of active items after last compaction
    uint16_t deleted;   // Number of items not transferred during compaction
    uint16_t badCRC;    // Number of bad CRCs encountered
}
NVOCMP_diag_t;

// Low Voltage Check Callback function, voltage is measured voltage value
typedef void (*lowVoltCbFptr)(uint32_t voltage);
//*****************************************************************************
// Functions
//*****************************************************************************

/**
 * @fn      NVOCMP_loadApiPtrs
 *
 * @brief   Global function to return function pointers for NV driver API that
 *          are supported by this module, NULL for functions not supported.
 *
 * @param   pfn - pointer to caller's structure of NV function pointers
 *
 * @return  none
 */
extern void NVOCMP_loadApiPtrs(NVINTF_nvFuncts_t *pfn);

/**
 * @fn      NVOCMP_loadApiPtrsExt
 *
 * @brief   Global function to return function pointers for NV driver API that
 *          are supported by this module, NULL for functions not supported.
 *          This function also loads the 'extended' API function pointers.
 *
 * @param   pfn - pointer to caller's structure of NV function pointers
 *
 * @return  none
 */
extern void NVOCMP_loadApiPtrsExt(NVINTF_nvFuncts_t *pfn);

/**
 * @fn      NVOCMP_loadApiPtrsMin
 *
 * @brief   Global function to return function pointers for NV driver API that
 *          are supported by this module, NULL for functions not supported.
 *          This function loads the minimum necessary API functions.
 *          This should allow smaller code size.
 *
 * @param   pfn - pointer to caller's structure of NV function pointers
 *
 * @return  none
 */
extern void NVOCMP_loadApiPtrsMin(NVINTF_nvFuncts_t *pfn);

/**
 * @fn      NVOCMP_setCheckVoltage
 *
 * @brief   Global function to allow user to provide a voltage check function
 *          for the driver to use. If a pointer is provided, the driver will
 *          call the provided function before flash erases and writes. The
 *          provided function should return true when the battery voltage is
 *          sufficient and vice versa. The user can withdraw their function
 *          by passing a NULL pointer to this function.
 *
 * @param   funcPtr - pointer to a function which returns a bool.
 *
 * @return  none
 */
extern void NVOCMP_setCheckVoltage(void *funcPtr);

/**
 * @fn      NVOCMP_setLowVoltageCb
 *
 * @brief   Global function to allow user to provide a low voltage callback function
 *          for the driver to use. If a pointer is provided, the driver will
 *          call the provided function when low voltage detected.
 *
 * @param   funcPtr - pointer to a function.
 *
 * @return  none
 */
extern void NVOCMP_setLowVoltageCb(lowVoltCbFptr funcPtr);

// Exception function can be defined to handle NV corruption issues
// If none provided, NV module attempts to proceed ignoring problem
#if !defined (NVOCMP_EXCEPTION)
#define NVOCMP_EXCEPTION(pg, err)
#endif

#ifdef NVDEBUG
// NVS Handle for debug
extern NVS_Handle nvsHandle;
#endif

#ifdef __cplusplus
}
#endif

#endif /* NVOCMP_H */
