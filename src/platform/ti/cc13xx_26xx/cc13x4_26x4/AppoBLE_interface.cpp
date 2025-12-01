/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *  This file contains the Application Over BLE interface used
 *  to manage additional BLE advertisements and connections to
 *  work in tandem with the MatterOverBLE GATT service.
 *
 */

/*****************************************************************************/
#ifdef TI_APPOBLE_ENABLE
#include <AppConfig.h>
#include <AppEvent.h>
#include <AppTask.h>
#include <AppoBLE.h>
extern "C" {

#if defined(ti_log_Log_ENABLE)
#include "ti_drivers_config.h"
#include "ti_log_config.h"
#define PLAT_LOG(...) Log_printf(LogModule_App1, Log_DEBUG, __VA_ARGS__);

#endif
}

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

uint8_t AppoBLE_interface::AddUpdateAppoBLEAdvSet(uint8_t advIndex, GapAdv_params_t advParams, uint8_t * advData,
                                                  uint8_t advDataLen, uint8_t * scanRspData, uint8_t scanRspDataLen,
                                                  GapAdv_eventMaskFlags_t evtMask)
{
    AppoBLE_interface::AppoBLEAdvApi_msg msg;

    msg.advIndex       = advIndex;
    msg.advParams      = advParams;
    msg.advData        = advData;
    msg.advDataLen     = advDataLen;
    msg.scanRspData    = scanRspData;
    msg.scanRspDataLen = scanRspDataLen;
    msg.evtMask        = evtMask;

    PLAT_LOG("SendAppoBLEAdvApi: From AppoBLE Layer");

    return (BLEMgrImpl().SendAppoBLEAdvApi(AppoBLE_interface::AppoBLE_api_type::AppoBLE_ADV_ADD_UPDATE, &msg));
}

/* Advertising data must be static, not freed when removed. */
uint8_t AppoBLE_interface::RemoveAppoBLEAdvSet(uint8_t advHandle)
{
    AppoBLE_interface::AppoBLEAdvApi_msg msg;

    msg.advIndex = advHandle;

    return (BLEMgrImpl().SendAppoBLEAdvApi(AppoBLE_interface::AppoBLE_api_type::AppoBLE_ADV_REMOVE, &msg));
}

uint8_t AppoBLE_interface::SetAppoBLEAdvInterval(uint8_t advHandle, uint32_t intervalMax, uint32_t intervalMin)
{
    return BLEMgrImpl().setAdvertisingInterval(advHandle, intervalMax, intervalMin);
}

uint8_t AppoBLE_interface::SetAppoBLEAdvertisingEnabled(uint8_t advHandle, uint8_t enable)
{
    return BLEMgrImpl().setAdvertisingEnabled(enable, advHandle);
}

uint8_t AppoBLE_interface::IsAppoBLEAdvertisingEnabled(uint8_t advHandle)
{
    return BLEMgrImpl().isAdvertisingEnabled(advHandle);
}

void AppoBLE_interface::GetAppoBLEDeviceName(char * buf, size_t bufSize)
{
    BLEMgrImpl().getDeviceName(buf, bufSize);
}

uint8_t AppoBLE_interface::SetAppoBLEDeviceName(const char * deviceName)
{
    return BLEMgrImpl().setDeviceName(deviceName);
}

void AppoBLE_interface::SetAppoBLEPairingPassCode(uint32_t passcode)
{
    BLEMgrImpl().SetAppoBLEPairingPassCode(passcode);
}

/* Pass-through API's to BLEPlatformDelegate defined API's */
uint8_t AppoBLE_interface::SendAppoBLEIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                                 uint8_t * srcBuf, uint16_t srcBufLen)
{
    return (BLEMgrImpl().SendIndication(conId, svcId, charId, srcBuf, srcBufLen));
}

uint8_t AppoBLE_interface::SendAppoBLEWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                   const ChipBleUUID * charId, uint8_t * srcBuf, uint16_t srcBufLen)
{
    return (BLEMgrImpl().SendWriteRequest(conId, svcId, charId, srcBuf, srcBufLen));
}
uint8_t AppoBLE_interface::SendAppoBLEReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                  const ChipBleUUID * charId, uint8_t * dstBuf, uint16_t dstBufLen)
{
    return (BLEMgrImpl().SendReadRequest(conId, svcId, charId, dstBuf, dstBufLen));
}

ConnectivityManager::CHIPoBLEServiceMode AppoBLE_interface::GetMatteroBLEServiceMode(BLE_CONNECTION_OBJECT conId)
{
    return BLEMgrImpl().getMatteroBLEServiceMode(conId);
}

BLE_CONNECTION_OBJECT AppoBLE_interface::AppoBLE_GetValidConnection(uint8_t numConnSearch)
{
    BLE_CONNECTION_OBJECT conId;
    uint8_t connFound = 0;
    uint8_t i;

    for (i = 0; (i < MAX_NUM_BLE_CONNS) && (connFound != numConnSearch); i++)
    {
        conId = BLEMgrImpl().GetConnection(i);
        if ((conId != NULL) && (*((uint16_t *) conId) != LL_CONNHANDLE_INVALID))
        {
            connFound++;
        }
    }

    if (connFound != numConnSearch)
    {
        conId = NULL;
    }

    return conId;
}

bStatus_t AppoBLE_interface::ScheduleAppoBLEFxn(pfnCallInBleMgrCtx_t FxnPtr, void * arg)
{
    return (BLEMgrImpl().SendGenericFxnReq(FxnPtr, arg));
}

bStatus_t AppoBLE_interface::EnqueueAppoBLEMsg(uint32_t serviceEvent, void * pData)
{
    return (BLEMgrImpl().EnqueueAppoBLEMsg(serviceEvent, pData));
}

extern "C" bStatus_t ScheduleAppoBLEFxn(pfnCallInBleMgrCtx_t FxnPtr, void * arg)
{
    return (BLEMgrImpl().SendGenericFxnReq(FxnPtr, arg));
}
#endif
