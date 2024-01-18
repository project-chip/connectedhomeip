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
 *         @file  chipOBLeProfile.h
 *         This file contains the CHIPOBLE profile definitions and prototypes
 *         prototypes.
 */

#ifndef CHIPOBLEPROFILE_H
#define CHIPOBLEPROFILE_H

#ifdef __cplusplus
#include <ble/BleUUID.h>
#include <platform/internal/BLEManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

// Simple Keys Profile Services bit fields
#define CHIPOBLEPROFILE_SERVICE 0x00000001

#define CHIPOBLE_SERV_UUID 0xFFF6           // Service UUID
#define CHIPOBLEPROFILE_TX_CHAR_UUID 0x129D // GATT indications/read
#define CHIPOBLEPROFILE_RX_CHAR_UUID 0x119D // GATT Writes

#define CHIPOBLEPROFILE_SERV_UUID_BASE128(uuid)                                                                                    \
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, LO_UINT16(uuid), HI_UINT16(uuid), 0x00, 0x00

#define CHIPOBLEPROFILE_CHAR_UUID_BASE128(uuid)                                                                                    \
    HI_UINT16(uuid), LO_UINT16(uuid), 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18

#define CHIPOBLEPROFILE_TX_CHAR 0  // R uint8 - Profile Characteristic 1 (CHIPOBLE Tx) value
#define CHIPOBLEPROFILE_RX_CHAR 1  // W uint8 - Profile Characteristic 2 (CHIPOBLE Rx) value
#define CHIPOBLEPROFILE_CCCWrite 2 // Client Characteristic Configuration

#define CHIPOBLEPROFILE_CHAR_LEN (244)
#define CHIPOBLEPROFILE_MAX_DESCRIPTION_LEN (20)

/*********************************************************************
 * TYPEDEFS
 */

/* Callback when a characteristic value has changed */
typedef void (*chipOBleProfileChange_t)(uint8 paramID, uint16 len, uint16_t connHandle);

typedef struct
{
    chipOBleProfileChange_t pfnchipOBleProfileChange; // Called when characteristic value changes
} chipOBleProfileCBs_t;

/*********************************************************************
 * API FUNCTIONS
 */

/*
 * CHIPoBLEProfile_AddService- Initializes the CHIPoBLE Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */

bStatus_t CHIPoBLEProfile_AddService(uint32 services);

/*
 *    CHIPoBLEProfile_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
bStatus_t CHIPoBLEProfile_RegisterAppCBs(chipOBleProfileCBs_t * appCallbacks);

/*
 *    CHIPoBLEProfile_SetParameter - Set a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
bStatus_t CHIPoBLEProfile_SetParameter(uint8 param, uint8 len, void * value, uint8_t taskId);

/*
 *    CHIPoBLEProfile_GetParameter - Get a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
bStatus_t CHIPoBLEProfile_GetParameter(uint8 param, void * value, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* CHIPOBLEPROFILE_H */
