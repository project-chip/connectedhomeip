/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      Implementation of the fault-injection utilities for CHIP.
 */
#include "CHIPFaultInjection.h"

#include <string.h>

namespace chip {
namespace FaultInjection {

static nl::FaultInjection::Record sFaultRecordArray[kFault_NumItems];
static int32_t sFault_CHIPNotificationSize_Arguments[1];
static int32_t sFault_FuzzExchangeHeader_Arguments[1];
static class nl::FaultInjection::Manager sChipFaultInMgr;
static const nl::FaultInjection::Name sManagerName = "chip";
const nl::FaultInjection::Name sFaultNames[]       = {
    "AllocExchangeContext",
    "DropIncomingUDPMsg",
    "DropOutgoingUDPMsg",
    "AllocBinding",
    "SendAlarm",
    "HandleAlarm",
    "FuzzExchangeHeaderTx",
    "RMPDoubleTx",
    "RMPSendError",
    "BDXBadBlockCounter",
    "BDXAllocTransfer",
    "SecMgrBusy",
    "IMInvoke_SeparateResponses",
    "IMInvoke_SeparateResponsesInvertResponseOrder",
    "IMInvoke_SkipSecondResponse",
    "ModifyWebRTCAnswerSessionId",
    "ModifyWebRTCOfferSessionId",
    "CASEServerBusy",
    "CASESkipInitiatorResumeMIC",
    "CASESkipResumptionID",
    "CASECorruptInitiatorResumeMIC",
    "CASECorruptDestinationID",
    "CASECorruptTBEData3Encrypted",
    "CASECorruptSigma3NOC",
    "CASECorruptSigma3ICAC",
    "CASECorruptSigma3Signature",
    "CASECorruptSigma3InitiatorEphPubKey",
    "CASECorruptSigma3ResponderEphPubKey",
};

static_assert(kFault_NumItems == sizeof(sFaultNames) / sizeof(sFaultNames[0]),
              "The last member of the Id enum (kFault_NumItems) should equal the length of sFaultNames[] ");

const size_t kNumChipFaultsFromEnum = kFault_NumItems;

/**
 * Get the singleton FaultInjection::Manager for CHIP faults
 */
nl::FaultInjection::Manager & GetManager()
{
    if (0 == sChipFaultInMgr.GetNumFaults())
    {
        sChipFaultInMgr.Init(kFault_NumItems, sFaultRecordArray, sManagerName, sFaultNames);
        memset(&sFault_CHIPNotificationSize_Arguments, 0, sizeof(sFault_CHIPNotificationSize_Arguments));
        memset(&sFault_FuzzExchangeHeader_Arguments, 0, sizeof(sFault_FuzzExchangeHeader_Arguments));
        sFaultRecordArray[kFault_FuzzExchangeHeaderTx].mArguments = sFault_FuzzExchangeHeader_Arguments;
        sFaultRecordArray[kFault_FuzzExchangeHeaderTx].mLengthOfArguments =
            static_cast<uint8_t>(sizeof(sFault_FuzzExchangeHeader_Arguments) / sizeof(sFault_FuzzExchangeHeader_Arguments[0]));
    }
    return sChipFaultInMgr;
}

/**
 * Get the number of times the fault injection point location got checked. This is useful for verifying that the code path
 * containing the fault injection was actually executed.
 * Note: The count includes all checks, even if the fault was not triggered.
 */
uint32_t GetFaultCounter(uint32_t faultID)
{
    return GetManager().GetFaultRecords()[faultID].mNumTimesChecked;
}

/**
 * Fuzz a byte of a CHIP Exchange Header
 *
 * @param[in] p     Pointer to the encoded Exchange Header
 * @param[in] arg   An index from 0 to (CHIP_FAULT_INJECTION_NUM_FUZZ_VALUES * 5 -1)
 *                  that specifies the byte to be corrupted and the value to use.
 */
DLL_EXPORT void FuzzExchangeHeader(uint8_t * p, int32_t arg)
{
    // CHIP is little endian; this function alters the
    // least significant byte of the header fields.
    const uint8_t offsets[] = {
        0, // flags and version
        1, // MessageType
        2, // ExchangeId
        4, // ProfileId
        8  // AckMessageCounter
    };
    const uint8_t values[CHIP_FAULT_INJECTION_NUM_FUZZ_VALUES] = { 0x1, 0x2, 0xFF };
    size_t offsetIndex                                         = 0;
    size_t valueIndex                                          = 0;
    size_t numOffsets                                          = sizeof(offsets) / sizeof(offsets[0]);
    offsetIndex                                                = static_cast<uint32_t>(arg) % (numOffsets);
    valueIndex = (static_cast<uint32_t>(arg) / numOffsets) % CHIP_FAULT_INJECTION_NUM_FUZZ_VALUES;
    p[offsetIndex] ^= values[valueIndex];
}

} // namespace FaultInjection
} // namespace chip
