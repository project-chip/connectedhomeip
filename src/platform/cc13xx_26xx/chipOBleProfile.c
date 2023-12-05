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
 *        @file  chipOBLeProfile.c
 *        This file contains the CHIPOBLE profile sample GATT service profile
 *        for use with the BLE Manager.
 */
/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include <icall.h>
#include <util.h>

#include "icall_ble_api.h"
#include "ti_ble_config.h"

#include "chipOBleProfile.h"

/*********************************************************************
 * GLOBAL VARIABLES
 */
// CHIPoBLE GATT Profile Service UUID
const uint8 chipOBleServUUID[ATT_BT_UUID_SIZE] = {
    // 0xF6, 0xFF
    LO_UINT16(CHIPOBLE_SERV_UUID), HI_UINT16(CHIPOBLE_SERV_UUID)
};

// CHIPoBLE Tx characteristic UUID
const uint8 chipOBleProfileTxCharUUID[ATT_UUID_SIZE] = {
    // 0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18
    CHIPOBLEPROFILE_CHAR_UUID_BASE128(CHIPOBLEPROFILE_TX_CHAR_UUID)
};

// CHIPoBLE Rx characteristic UUID
const uint8 chipOBleProfileRxCharUUID[ATT_UUID_SIZE] = {
    // 0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18
    CHIPOBLEPROFILE_CHAR_UUID_BASE128(CHIPOBLEPROFILE_RX_CHAR_UUID)
};

/*********************************************************************
 * Profile Attributes - variables
 */

// CHIPoBLE GATT Profile Service attribute
static const gattAttrType_t chipoBleProfile = { ATT_BT_UUID_SIZE, chipOBleServUUID };

// CHIPoBLE Tx Characteristic Properties
static uint8_t chipOBleProfileTxCharProps = GATT_PROP_READ | GATT_PROP_INDICATE;

// CHIPoBLE Tx Characteristic Value
static uint8_t chipOBleProfileTxCharVal[CHIPOBLEPROFILE_CHAR_LEN] = { 0x00 };

// CHIPoBLE Tx Characteristic User Description
static uint8_t chipOBleProfileTxdDataUserDesp[CHIPOBLEPROFILE_MAX_DESCRIPTION_LEN] = "ChipOBLE Tx Char";

static gattCharCfg_t * chipOBleProfileTxStateDataConfig;

// CHIPoBLE Rx Characteristic Properties
static uint8_t chipOBleProfileRxCharProps = GATT_PROP_WRITE;

// CHIPoBLE Rx Characteristic Value
static uint8_t chipOBleProfileRxCharVal[CHIPOBLEPROFILE_CHAR_LEN] = { 0x00 };

// CHIPoBLE Rx Characteristic User Description
static uint8_t chipOBleProfileRxdDataUserDesp[CHIPOBLEPROFILE_MAX_DESCRIPTION_LEN] = "ChipOBLE Rx Char";

/*********************************************************************
 * LOCAL VARIABLES
 */

static chipOBleProfileCBs_t * chipOBleProfile_AppCBs = NULL;

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t chipoBleProfileAttrTbl[] = {
    // CHIPoBLE Service
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
        GATT_PERMIT_READ,                         /* permissions */
        0,                                        /* handle */
        (uint8 *) &chipoBleProfile                /* pValue */
    },
    // CHIPoBLE Tx Characteristic Declaration
    { { ATT_BT_UUID_SIZE, characterUUID }, GATT_PERMIT_READ, 0, &chipOBleProfileTxCharProps },
    // CHIPoBLE Tx Characteristic Value
    { { ATT_UUID_SIZE, chipOBleProfileTxCharUUID }, GATT_PERMIT_READ, 0, chipOBleProfileTxCharVal },

    // CHIPoBLE Tx Characteristic configuration
    { { ATT_BT_UUID_SIZE, clientCharCfgUUID },
      GATT_PERMIT_READ | GATT_PERMIT_WRITE,
      0,
      (uint8 *) &chipOBleProfileTxStateDataConfig },

    // CHIPoBLE Tx Characteristic User Description
    { { ATT_BT_UUID_SIZE, charUserDescUUID }, GATT_PERMIT_READ, 0, chipOBleProfileTxdDataUserDesp },

    // CHIPoBLE Rx Characteristic Declaration
    { { ATT_BT_UUID_SIZE, characterUUID }, GATT_PERMIT_READ, 0, &chipOBleProfileRxCharProps },
    // CHIPoBLE Rx Characteristic Value
    { { ATT_UUID_SIZE, chipOBleProfileRxCharUUID }, GATT_PERMIT_WRITE, 0, chipOBleProfileRxCharVal },

    // CHIPoBLE Rx Characteristic User Description
    { { ATT_BT_UUID_SIZE, charUserDescUUID }, GATT_PERMIT_READ, 0, chipOBleProfileRxdDataUserDesp },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t CHIPoBLEProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t * pAttr, uint8_t * pValue, uint16_t * pLen,
                                            uint16_t offset, uint16_t maxLen, uint8_t method);
static bStatus_t CHIPoBLEProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t * pAttr, uint8_t * pValue, uint16_t len,
                                             uint16_t offset, uint8_t method);

/*********************************************************************
 * PROFILE CALLBACKS
 */

// CHIPoBLE Service Callbacks
// Note: When an operation on a characteristic requires authorization and
// pfnAuthorizeAttrCB is not defined for that characteristic's service, the
// Stack will report a status of ATT_ERR_UNLIKELY to the client.  When an
// operation on a characteristic requires authorization the Stack will call
// pfnAuthorizeAttrCB to check a client's authorization prior to calling
// pfnReadAttrCB or pfnWriteAttrCB, so no checks for authorization need to be
// made within these functions.
const gattServiceCBs_t chipOBleProfileCBs = {
    CHIPoBLEProfile_ReadAttrCB,  // Read callback function pointer
    CHIPoBLEProfile_WriteAttrCB, // Write callback function pointer
    NULL                         // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      CHIPoBLEProfile_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t CHIPoBLEProfile_AddService(uint32 services)
{
    uint8 status;

    // Allocate Client Characteristic Configuration tables
    chipOBleProfileTxStateDataConfig = (gattCharCfg_t *) ICall_malloc((uint_least16_t) (sizeof(gattCharCfg_t) * MAX_NUM_BLE_CONNS));
    if (chipOBleProfileTxStateDataConfig == NULL)
    {
        return bleMemAllocError;
    }

    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg(LL_CONNHANDLE_INVALID, chipOBleProfileTxStateDataConfig);

    if (services & CHIPOBLEPROFILE_SERVICE)
    {
        // Register GATT attribute list and CBs with GATT Server App
        status = GATTServApp_RegisterService(chipoBleProfileAttrTbl, GATT_NUM_ATTRS(chipoBleProfileAttrTbl),
                                             GATT_MAX_ENCRYPT_KEY_SIZE, &chipOBleProfileCBs);
    }
    else
    {
        status = SUCCESS;
    }

    return status;
}

/*********************************************************************
 * @fn      CHIPoBLEProfile_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t CHIPoBLEProfile_RegisterAppCBs(chipOBleProfileCBs_t * appCallbacks)
{
    if (appCallbacks)
    {
        chipOBleProfile_AppCBs = appCallbacks;

        return SUCCESS;
    }
    else
    {
        return bleAlreadyInRequestedMode;
    }
}

/*********************************************************************
 * @fn      CHIPoBLEProfile_SetParameter
 *
 * @brief   Set a CHIPoBLE Characteristic value
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to write
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
extern uint8 gattAppTaskID;
bStatus_t CHIPoBLEProfile_SetParameter(uint8 param, uint8 len, void * value, uint8_t taskId)
{
    bStatus_t ret = SUCCESS;
    switch (param)
    {
    case CHIPOBLEPROFILE_TX_CHAR:

        VOID memcpy(chipOBleProfileTxCharVal, value, len);

        // See if Indications has been enabled
        ret = GATTServApp_ProcessCharCfg(chipOBleProfileTxStateDataConfig, chipOBleProfileTxCharVal, FALSE, chipoBleProfileAttrTbl,
                                         GATT_NUM_ATTRS(chipoBleProfileAttrTbl), taskId, CHIPoBLEProfile_ReadAttrCB);
        break;
    default:
        ret = INVALIDPARAMETER;
        break;
    }

    return ret;
}

/*********************************************************************
 * @fn      CHIPoBLEProfile_GetParameter
 *
 * @brief   Get a CHIPoBLE Characteristic value
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t CHIPoBLEProfile_GetParameter(uint8 param, void * value, uint16_t len)
{
    bStatus_t ret = SUCCESS;
    switch (param)
    {
    case CHIPOBLEPROFILE_RX_CHAR:
        if (len != 0)
        {
            VOID memcpy(value, chipOBleProfileRxCharVal, len);
        }
        else
        {
            VOID memcpy(value, chipOBleProfileRxCharVal, CHIPOBLEPROFILE_CHAR_LEN);
        }
        break;
    case CHIPOBLEPROFILE_CCCWrite:
        *((uint8 *) value) = chipOBleProfileTxStateDataConfig->value;
        break;
    default:
        ret = INVALIDPARAMETER;
        break;
    }

    return ret;
}

/*********************************************************************
 * @fn          CHIPoBLEProfile_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t CHIPoBLEProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t * pAttr, uint8_t * pValue, uint16_t * pLen,
                                            uint16_t offset, uint16_t maxLen, uint8_t method)
{
    bStatus_t status = SUCCESS;
    uint8 len        = maxLen;

    if (offset + maxLen > CHIPOBLEPROFILE_CHAR_LEN)
        len = CHIPOBLEPROFILE_CHAR_LEN - offset;

    *pLen = len;
    VOID osal_memcpy(pValue, (pAttr->pValue) + offset, len);

    return status;
}

/*********************************************************************
 * @fn      CHIPoBLEProfile_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t CHIPoBLEProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t * pAttr, uint8_t * pValue, uint16_t len,
                                             uint16_t offset, uint8_t method)
{
    bStatus_t status = SUCCESS;
    uint8 notifyApp  = 0xFF;

    if (pAttr->type.len == ATT_UUID_SIZE)
    {
        if (!memcmp(pAttr->type.uuid, chipOBleProfileRxCharUUID, pAttr->type.len))
        {
            VOID osal_memcpy((pAttr->pValue) + offset, pValue, len);

            // Send notification to BLE Event handler
            notifyApp = CHIPOBLEPROFILE_RX_CHAR;
        }
        else
        {
            status = ATT_ERR_ATTR_NOT_FOUND;
        }
    }

    else if ((pAttr->type.len == ATT_BT_UUID_SIZE) &&
             (BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]) == GATT_CLIENT_CHAR_CFG_UUID))
    {

        notifyApp = CHIPOBLEPROFILE_CCCWrite;

        status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len, offset, GATT_CLIENT_CFG_INDICATE);
    }
    else
    {
        // unsupported length
        status = ATT_ERR_INVALID_HANDLE;
    }

    // If a characteristic value changed then callback function to notify application of change
    if ((notifyApp != 0xFF) && chipOBleProfile_AppCBs && chipOBleProfile_AppCBs->pfnchipOBleProfileChange)
    {
        chipOBleProfile_AppCBs->pfnchipOBleProfileChange(notifyApp, len, connHandle);
    }

    return status;
}
