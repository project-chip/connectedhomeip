/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
 *    All rights reserved.
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
 *    @file
 *          Provides an implementation for BLE Host NVM functions
 */
#include "assert.h"
#include "ble_constants.h"
#include "fsl_os_abstraction.h"
#include "gap_interface.h"
#include "gatt_database.h"

#include "ble_constants.h"
#include "gatt_db_dynamic.h"

#include "ble_function_mux.h"

/* Security Manager */
#define smpEdiv 0x1F99
#define mcEncryptionKeySize_c 16

/* LTK */
static uint8_t smpLtk[gcSmpMaxLtkSize_c] = { 0xD6, 0x93, 0xE8, 0xA4, 0x23, 0x55, 0x48, 0x99,
                                             0x1D, 0x77, 0x61, 0xE6, 0x63, 0x2B, 0x10, 0x8E };

/* RAND*/
static uint8_t smpRand[gcSmpMaxRandSize_c] = { 0x26, 0x1E, 0xF6, 0x09, 0x97, 0x2E, 0xAD, 0x7E };

/* IRK */
static uint8_t smpIrk[gcSmpIrkSize_c] = { 0x0A, 0x2D, 0xF4, 0x65, 0xE3, 0xBD, 0x7B, 0x49,
                                          0x1E, 0xB4, 0xC0, 0x95, 0x95, 0x13, 0x46, 0x73 };

/* CSRK */
static uint8_t smpCsrk[gcSmpCsrkSize_c] = { 0x90, 0xD5, 0x06, 0x95, 0x92, 0xED, 0x91, 0xD7,
                                            0xA8, 0x9E, 0x2C, 0xDC, 0x4A, 0x93, 0x5B, 0xF9 };

gapSmpKeys_t gSmpKeys = {
    .cLtkSize  = mcEncryptionKeySize_c,
    .aLtk      = (void *) smpLtk,
    .aIrk      = (void *) smpIrk,
    .aCsrk     = (void *) smpCsrk,
    .aRand     = (void *) smpRand,
    .cRandSize = gcSmpMaxRandSize_c,
    .ediv      = smpEdiv,
};

/*******************************************************************************
 * Functions needed by the BLE stack
 ******************************************************************************/
void App_NvmRead(uint8_t mEntryIdx, void * pBondHeader, void * pBondDataDynamic, void * pBondDataStatic, void * pBondDataDeviceInfo,
                 void * pBondDataDescriptor, uint8_t mDescriptorIndex)
{
    NOT_USED(mEntryIdx);
    NOT_USED(pBondHeader);
    NOT_USED(pBondDataDynamic);
    NOT_USED(pBondDataStatic);
    NOT_USED(pBondDataDeviceInfo);
    NOT_USED(pBondDataDescriptor);
    NOT_USED(mDescriptorIndex);
}

void App_NvmWrite(uint8_t mEntryIdx, void * pBondHeader, void * pBondDataDynamic, void * pBondDataStatic,
                  void * pBondDataDeviceInfo, void * pBondDataDescriptor, uint8_t mDescriptorIndex)
{
    NOT_USED(mEntryIdx);
    NOT_USED(pBondHeader);
    NOT_USED(pBondDataDynamic);
    NOT_USED(pBondDataStatic);
    NOT_USED(pBondDataDeviceInfo);
    NOT_USED(pBondDataDescriptor);
    NOT_USED(mDescriptorIndex);
}

void App_NvmErase(uint8_t mEntryIdx)
{
    NOT_USED(mEntryIdx);
}
