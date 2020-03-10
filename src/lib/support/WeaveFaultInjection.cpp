/*
 *
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      Implementation of the fault-injection utilities for Weave.
 */

#include <string.h>
#include <nlassert.h>

#include <Weave/Support/WeaveFaultInjection.h>

#if WEAVE_CONFIG_TEST

namespace nl {
namespace Weave {
namespace FaultInjection {

static nl::FaultInjection::Record sFaultRecordArray[kFault_NumItems];
static int32_t sFault_WDMNotificationSize_Arguments[1];
static int32_t sFault_FuzzExchangeHeader_Arguments[1];
static class nl::FaultInjection::Manager sWeaveFaultInMgr;
static const nl::FaultInjection::Name sManagerName = "Weave";
static const nl::FaultInjection::Name sFaultNames[] = {
    "AllocExchangeContext",
    "DropIncomingUDPMsg",
    "DropOutgoingUDPMsg",
    "AllocBinding",
    "SendAlarm",
    "HandleAlarm",
    "FuzzExchangeHeaderTx",
#if WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING
    "WRMDoubleTx",
    "WRMSendError",
#endif // WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING
    "BDXBadBlockCounter",
    "BDXAllocTransfer",
#if WEAVE_CONFIG_ENABLE_SERVICE_DIRECTORY
    "SMConnectRequestNew",
    "SMLookup",
    "SMCacheReplaceEntryError",
#endif // WEAVE_CONFIG_ENABLE_SERVICE_DIRECTORY
    "WDMTraitInstanceNew",
    "WDMSubscriptionHandlerNew",
    "WDMSubscriptionClientNew",
    "WDMBadSubscriptionId",
    "WDMSendUnsupportedReqMsgType",
    "WDMNotificationSize",
    "WDMSendCommandExpired",
    "WDMSendCommandBadVersion",
    "WDMSendUpdateBadVersion",
    "WDMDelayUpdateResponse",
    "WDMUpdateRequestTimeout",
    "WDMUpdateRequestSendErrorInline",
    "WDMUpdateRequestSendErrorAsync",
    "WDMUpdateRequestBadProfile",
    "WDMUpdateRequestDropMessage",
    "WDMUpdateResponseBusy",
    "WDMPathStoreFull",
    "WDMTreatNotifyAsCancel",
    "CASEKeyConfirm",
    "SecMgrBusy",
#if WEAVE_CONFIG_ENABLE_TUNNELING
    "TunnelQueueFull",
    "TunnelPacketDropByPolicy",
#endif // WEAVE_CONFIG_ENABLE_TUNNELING
#if CONFIG_NETWORK_LAYER_BLE
    "WOBLESend",
#endif // CONFIG_NETWORK_LAYER_BLE
};


/**
 * Get the singleton FaultInjection::Manager for Inet faults
 */
nl::FaultInjection::Manager &GetManager(void)
{
    if (0 == sWeaveFaultInMgr.GetNumFaults())
    {
        sWeaveFaultInMgr.Init(kFault_NumItems,
                              sFaultRecordArray,
                              sManagerName,
                              sFaultNames);
        memset(&sFault_WDMNotificationSize_Arguments, 0, sizeof(sFault_WDMNotificationSize_Arguments));
        sFaultRecordArray[kFault_WDM_NotificationSize].mArguments = sFault_WDMNotificationSize_Arguments;
        sFaultRecordArray[kFault_WDM_NotificationSize].mLengthOfArguments =
            static_cast<uint8_t>(sizeof(sFault_WDMNotificationSize_Arguments)/sizeof(sFault_WDMNotificationSize_Arguments[0]));

        memset(&sFault_FuzzExchangeHeader_Arguments, 0, sizeof(sFault_FuzzExchangeHeader_Arguments));
        sFaultRecordArray[kFault_FuzzExchangeHeaderTx].mArguments = sFault_FuzzExchangeHeader_Arguments;
        sFaultRecordArray[kFault_FuzzExchangeHeaderTx].mLengthOfArguments =
            static_cast<uint8_t>(sizeof(sFault_FuzzExchangeHeader_Arguments)/sizeof(sFault_FuzzExchangeHeader_Arguments[0]));

    }
    return sWeaveFaultInMgr;
}

/**
 * Fuzz a byte of a Weave Exchange Header
 *
 * @param[in] p     Pointer to the encoded Exchange Header
 * @param[in] arg   An index from 0 to (WEAVE_FAULT_INJECTION_NUM_FUZZ_VALUES * 5 -1)
 *                  that specifies the byte to be corrupted and the value to use.
 */
NL_DLL_EXPORT void FuzzExchangeHeader(uint8_t *p, int32_t arg)
{
    // Weave is little endian; this function alters the
    // least significant byte of the header fields.
    const uint8_t offsets[] = {
        0, // flags and version
        1, // MessageType
        2, // ExchangeId
        4, // ProfileId
        8  // AckMsgId
    };
    const uint8_t values[WEAVE_FAULT_INJECTION_NUM_FUZZ_VALUES] = { 0x1, 0x2, 0xFF };
    size_t offsetIndex = 0;
    size_t valueIndex = 0;
    size_t numOffsets = sizeof(offsets)/sizeof(offsets[0]);
    offsetIndex = arg % (numOffsets);
    valueIndex = (arg / numOffsets) % WEAVE_FAULT_INJECTION_NUM_FUZZ_VALUES;
    p[offsetIndex] ^= values[valueIndex];
}

} // namespace FaultInjection
} // namespace Weave
} // namespace nl

#endif // WEAVE_CONFIG_TEST
