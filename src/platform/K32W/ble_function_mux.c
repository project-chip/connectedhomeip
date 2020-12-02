/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "gap_interface.h"
#include "ble_constants.h"
#include "gatt_database.h"
#include "fsl_os_abstraction.h"
#include "assert.h"

#include "gatt_db_dynamic.h"

#include "ble_function_mux.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
ble_func_mux_app_mode_t currentMode = kBleFuncMux_AppMode_Ota;

/**
 * Normal BLE variables.
 **/

/*! Allocate custom 128-bit UUIDs, if any */
#include "gatt_uuid_def_x.h"

#include "gatt_db_x_macros.h"
#include "gatt_db_macros.h"

/*! Allocate the arrays for Attribute Values */
#include "gatt_alloc_x.h"

/*! Declare the Attribute database */
static gattDbAttribute_t static_gattDatabase[] = {
#include "gatt_decl_x.h"
};

/*! Declare structure to compute the database size */
typedef struct sizeCounterStruct_tag {
#include "gatt_size_x.h"
} sizeCounterStruct_t;

/*! *********************************************************************************
 *  BLE Stack Configuration
 ********************************************************************************** */
#define gMaxBondedDevices_c             1

/* Security Manager */
#define smpEdiv                 0x1F99
#define mcEncryptionKeySize_c   16

typedef struct {
    uint16_t pdmId;
    uint16_t nbRamWrite;
    bleBondIdentityHeaderBlob_t  aBondingHeader;
    bleBondDataDynamicBlob_t     aBondingDataDynamic;
    bleBondDataStaticBlob_t      aBondingDataStatic;
    bleBondDataDeviceInfoBlob_t  aBondingDataDeviceInfo;
    bleBondDataDescriptorBlob_t  aBondingDataDescriptor[gcGapMaximumSavedCccds_c];
} bleBondDeviceEntry;

#if (gMaxBondedDevices_c > 0)
static bleBondDeviceEntry bondEntries[gMaxBondedDevices_c];
static osaMutexId_t bondingMutex;
#endif

/* LTK */
static uint8_t smpLtk[gcSmpMaxLtkSize_c] =
    {0xD6, 0x93, 0xE8, 0xA4, 0x23, 0x55, 0x48, 0x99,
     0x1D, 0x77, 0x61, 0xE6, 0x63, 0x2B, 0x10, 0x8E};

/* RAND*/
static uint8_t smpRand[gcSmpMaxRandSize_c] =
    {0x26, 0x1E, 0xF6, 0x09, 0x97, 0x2E, 0xAD, 0x7E};

/* IRK */
static uint8_t smpIrk[gcSmpIrkSize_c] =
    {0x0A, 0x2D, 0xF4, 0x65, 0xE3, 0xBD, 0x7B, 0x49,
     0x1E, 0xB4, 0xC0, 0x95, 0x95, 0x13, 0x46, 0x73};

/* CSRK */
static uint8_t smpCsrk[gcSmpCsrkSize_c] =
    {0x90, 0xD5, 0x06, 0x95, 0x92, 0xED, 0x91, 0xD7,
     0xA8, 0x9E, 0x2C, 0xDC, 0x4A, 0x93, 0x5B, 0xF9};

/*! Compute the database size at compile time */
#define localGattDbAttributeCount_d  ((sizeof(sizeCounterStruct_t))/4U)

/* Security Manager */
gapSmpKeys_t gSmpKeys = {
    .cLtkSize = mcEncryptionKeySize_c,
    .aLtk = (void *)smpLtk,
    .aIrk = (void *)smpIrk,
    .aCsrk = (void *)smpCsrk,
    .aRand = (void *)smpRand,
    .cRandSize = gcSmpMaxRandSize_c,
    .ediv = smpEdiv,
    .addressType = gBleAddrTypePublic_c,
    .aAddress = NULL
};

/**
 *  Needed by the BLE stack.
 **/
gattDbAttribute_t*  gattDatabase;
uint16_t            gGattDbAttributeCount_c;

uint16_t GattDb_GetIndexOfHandle(uint16_t handle);
void App_NvmInit(void);
void App_NvmErase(uint8_t mEntryIdx);
void App_NvmRead(uint8_t mEntryIdx, void* pBondHeader, void* pBondDataDynamic,
                 void* pBondDataStatic, void* pBondDataDeviceInfo,
                 void* pBondDataDescriptor, uint8_t mDescriptorIndex);
void App_NvmWrite(uint8_t mEntryIdx, void* pBondHeader, void* pBondDataDynamic,
                  void* pBondDataStatic, void* pBondDataDeviceInfo,
                  void* pBondDataDescriptor, uint8_t mDescriptorIndex);
/*******************************************************************************
 * Functions needed by the BLE stack
 ******************************************************************************/
extern bleResult_t GattDb_Init(void)
{
    App_NvmInit();

    switch (currentMode)
    {
        case kBleFuncMux_AppMode_Ota:
        {
            static bool_t mAlreadyInit = FALSE;
            if (mAlreadyInit)
            {
                return gBleAlreadyInitialized_c;
            }
            mAlreadyInit = TRUE;

            gGattDbAttributeCount_c = localGattDbAttributeCount_d;
            gattDatabase = static_gattDatabase;

            /*! Attribute-specific initialization by X-Macro expansion */
            #include "gatt_init_x.h"

            return gBleSuccess_c;
        }

        default:
            return gGattDbInvalidHandle_c;
    }
}

uint16_t GattDb_GetIndexOfHandle(uint16_t handle)
{
    uint16_t init = (handle >= gGattDbAttributeCount_c) ?
                               (gGattDbAttributeCount_c - 1) : handle;
    for (uint16_t j = init; j != 0xFFFF && gattDatabase[j].handle >= handle; j--)
    {
        if (gattDatabase[j].handle == handle)
        {
            return j;
        }
    }
    return gGattDbInvalidHandleIndex_d;
}

void App_NvmInit(void)
{
#if (gMaxBondedDevices_c > 0)
    {
        /* Init the bonded device list */
        uint16_t i = 0;

        FLib_MemSet(bondEntries, 0, sizeof(bondEntries));
        bondingMutex = OSA_MutexCreate();
        assert(bondingMutex != NULL);
        for (i=0; i<gMaxBondedDevices_c; i++)
        {
            bondEntries[i].pdmId = i;
            bondEntries[i].nbRamWrite = 0;
        }
    }
#endif
}

void App_NvmRead
(
    uint8_t  mEntryIdx,
    void*    pBondHeader,
    void*    pBondDataDynamic,
    void*    pBondDataStatic,
    void*    pBondDataDeviceInfo,
    void*    pBondDataDescriptor,
    uint8_t  mDescriptorIndex
)
{
#if (gMaxBondedDevices_c > 0)

    {
        OSA_MutexLock(bondingMutex, osaWaitForever_c);
        if(mEntryIdx < (uint8_t)gMaxBondedDevices_c)
        {
            if (pBondHeader != NULL)
            {
                FLib_MemCpy(pBondHeader, (void*)&bondEntries[mEntryIdx].aBondingHeader,
                            sizeof(bondEntries[mEntryIdx].aBondingHeader));
            }

            if (pBondDataDynamic != NULL)
            {
                FLib_MemCpy(pBondDataDynamic, (void*)&bondEntries[mEntryIdx].aBondingDataDynamic,
                            sizeof(bondEntries[mEntryIdx].aBondingDataDynamic));
            }

            if (pBondDataStatic != NULL)
            {
                FLib_MemCpy(pBondDataStatic, (void*)&bondEntries[mEntryIdx].aBondingDataStatic,
                            sizeof(bondEntries[mEntryIdx].aBondingDataStatic));
            }

            if (pBondDataDeviceInfo != NULL)
            {
                FLib_MemCpy(pBondDataDeviceInfo, (void*)&bondEntries[mEntryIdx].aBondingDataDeviceInfo,
                            sizeof(bondEntries[mEntryIdx].aBondingDataDeviceInfo));
            }

            if (pBondDataDescriptor != NULL && mDescriptorIndex<gcGapMaximumSavedCccds_c)
            {
                FLib_MemCpy(pBondDataDescriptor,
                           (void*)&bondEntries[mEntryIdx].aBondingDataDescriptor[mDescriptorIndex],
                            gBleBondDataDescriptorSize_c);
            }
        }
        OSA_MutexUnlock(bondingMutex);
    }
#else
    NOT_USED(mEntryIdx);
    NOT_USED(pBondHeader);
    NOT_USED(pBondDataDynamic);
    NOT_USED(pBondDataStatic);
    NOT_USED(pBondDataDeviceInfo);
    NOT_USED(pBondDataDescriptor);
    NOT_USED(mDescriptorIndex);
#endif
}

void App_NvmWrite
(
    uint8_t  mEntryIdx,
    void*    pBondHeader,
    void*    pBondDataDynamic,
    void*    pBondDataStatic,
    void*    pBondDataDeviceInfo,
    void*    pBondDataDescriptor,
    uint8_t  mDescriptorIndex
)
{
#if (gMaxBondedDevices_c > 0)
    {
        OSA_MutexLock(bondingMutex, osaWaitForever_c);
        if(mEntryIdx < (uint8_t)gMaxBondedDevices_c)
        {
            if (pBondHeader != NULL)
            {
                FLib_MemCpy((void*)&bondEntries[mEntryIdx].aBondingHeader, pBondHeader,
                             sizeof(bondEntries[mEntryIdx].aBondingHeader));
                bondEntries[mEntryIdx].nbRamWrite++;
            }

            if (pBondDataDynamic != NULL)
            {
                FLib_MemCpy((void*)&bondEntries[mEntryIdx].aBondingDataDynamic, pBondDataDynamic,
                             sizeof(bondEntries[mEntryIdx].aBondingDataDynamic));
                bondEntries[mEntryIdx].nbRamWrite++;
            }

            if (pBondDataStatic != NULL)
            {
                FLib_MemCpy((void*)&bondEntries[mEntryIdx].aBondingDataStatic, pBondDataStatic,
                             sizeof(bondEntries[mEntryIdx].aBondingDataStatic));
                bondEntries[mEntryIdx].nbRamWrite++;
            }

            if (pBondDataDeviceInfo != NULL)
            {
                FLib_MemCpy((void*)&bondEntries[mEntryIdx].aBondingDataDeviceInfo, pBondDataDeviceInfo,
                             sizeof(bondEntries[mEntryIdx].aBondingDataDeviceInfo));
                bondEntries[mEntryIdx].nbRamWrite++;
            }

            if (pBondDataDescriptor != NULL && mDescriptorIndex<gcGapMaximumSavedCccds_c)
            {
                FLib_MemCpy((void*)&bondEntries[mEntryIdx].aBondingDataDescriptor[mDescriptorIndex],
                             pBondDataDescriptor, gBleBondDataDescriptorSize_c);
                bondEntries[mEntryIdx].nbRamWrite++;
            }
        }
        OSA_MutexUnlock(bondingMutex);
    }
#else
    NOT_USED(mEntryIdx);
    NOT_USED(pBondHeader);
    NOT_USED(pBondDataDynamic);
    NOT_USED(pBondDataStatic);
    NOT_USED(pBondDataDeviceInfo);
    NOT_USED(pBondDataDescriptor);
    NOT_USED(mDescriptorIndex);
#endif
}

void App_NvmErase(uint8_t mEntryIdx)
{
#if (gMaxBondedDevices_c > 0)
    {
        OSA_MutexLock(bondingMutex, osaWaitForever_c);
        if(mEntryIdx < (uint8_t)gMaxBondedDevices_c)
        {
            /* Check if a write is required */
            if (!FLib_MemCmpToVal(&bondEntries[mEntryIdx].aBondingHeader, 0,
                                  sizeof(bondEntries[mEntryIdx].aBondingHeader)) ||
                !FLib_MemCmpToVal(&bondEntries[mEntryIdx].aBondingDataDynamic, 0,
                                  sizeof(bondEntries[mEntryIdx].aBondingDataDynamic)) ||
                !FLib_MemCmpToVal(&bondEntries[mEntryIdx].aBondingDataStatic, 0,
                                  sizeof(bondEntries[mEntryIdx].aBondingDataStatic)) ||
                !FLib_MemCmpToVal(&bondEntries[mEntryIdx].aBondingDataDeviceInfo, 0,
                                  sizeof(bondEntries[mEntryIdx].aBondingDataDeviceInfo)) ||
                !FLib_MemCmpToVal(&bondEntries[mEntryIdx].aBondingDataDescriptor[0], 0,
                                  sizeof(bondEntries[mEntryIdx].aBondingDataDescriptor)))
            {
                FLib_MemSet(&bondEntries[mEntryIdx].aBondingHeader, 0,
                            sizeof(bondEntries[mEntryIdx].aBondingHeader));
                FLib_MemSet(&bondEntries[mEntryIdx].aBondingDataDynamic, 0,
                            sizeof(bondEntries[mEntryIdx].aBondingDataDynamic));
                FLib_MemSet(&bondEntries[mEntryIdx].aBondingDataStatic, 0,
                            sizeof(bondEntries[mEntryIdx].aBondingDataStatic));
                FLib_MemSet(&bondEntries[mEntryIdx].aBondingDataDeviceInfo, 0,
                            sizeof(bondEntries[mEntryIdx].aBondingDataDeviceInfo));
                FLib_MemSet(&bondEntries[mEntryIdx].aBondingDataDescriptor[0], 0,
                            sizeof(bondEntries[mEntryIdx].aBondingDataDescriptor));
                bondEntries[mEntryIdx].nbRamWrite++;
            }
        }
        OSA_MutexUnlock(bondingMutex);
    }
#else
    NOT_USED(mEntryIdx);
#endif
}
