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
 *      Header file for the fault-injection utilities for CHIP.
 */

#pragma once

#include <lib/core/CHIPConfig.h>

#if CHIP_WITH_NLFAULTINJECTION
#ifdef NDEBUG
// TODO(#30453): After fixing the issue where CHIP_WITH_NLFAULTINJECTION is seemingly enabled on release builds,
// uncomment the line below.
// static_assert(false, "CHIP_WITH_NLFAULTINJECTION should NOT be enabled on release build");
#endif

#include <nlfaultinjection.hpp>

#include <lib/support/DLLUtil.h>

namespace chip {
namespace FaultInjection {

// X-Macro style enumeration of Fault Names and their ID.
// This list is used to generate the "Id" enum and the array of strings sFaultNames[]
// WARNING: When adding/modifying Faults to the below macro, make sure the changes are duplicated to the CHIPFaultId enum in the
// Python Module src/controller/python/chip/fault_injection/__init__.py

// BEGIN-IF-CHANGE-ALSO-CHANGE(src/controller/python/chip/fault_injection/__init__.py)
#define CHIP_FAULTS_ENUMERATE(X)                                                                                                   \
    X(AllocExchangeContext, 0) /**< Fail the allocation of an ExchangeContext */                                                   \
    X(DropIncomingUDPMsg, 1)   /**< Drop an incoming UDP message without any processing */                                         \
    X(DropOutgoingUDPMsg, 2)   /**< Drop an outgoing UDP message at the chip Message layer */                                      \
    X(AllocBinding, 3)         /**< Fail the allocation of a Binding */                                                            \
    X(SendAlarm, 4)            /**< Fail to send an alarm message */                                                               \
    X(HandleAlarm, 5)          /**< Fail to handle an alarm message */                                                             \
    X(FuzzExchangeHeaderTx, 6) /**< Fuzz an Exchange Header in the packet buffer; expects an int argument selecting a fuzzing      \
                                  method. @see FuzzExchangeHeader */                                                               \
    X(RMPDoubleTx, 7)          /**< Force RMP to transmit the outgoing message twice */                                            \
    X(RMPSendError, 8)         /**< Simulate a send error in RMP as if retransmission count exceeded */                            \
    X(BDXBadBlockCounter, 9)   /**< Corrupt the BDX Block Counter in a BlockSend or BlockEOF message */                            \
    X(BDXAllocTransfer, 10)    /**< Fail the allocation of a BDXTransfer object */                                                 \
    X(SecMgrBusy, 11)          /**< Simulate WEAVE_ERROR_SECURITY_MANAGER_BUSY on session start */                                 \
    X(IMInvoke_SeparateResponses, 12) /**< Respond to 2 InvokeRequestMessage commands with 2 separate responses */                 \
    X(IMInvoke_SeparateResponsesInvertResponseOrder, 13) /**< Respond with 2 InvokeResponses in reverse order of request */        \
    X(IMInvoke_SkipSecondResponse, 14)                   /**< Respond to 2 commands but drop the second response */                \
    X(ModifyWebRTCAnswerSessionId, 15)                   /**< Modify session ID in outgoing WebRTC Answer command */               \
    X(ModifyWebRTCOfferSessionId, 16)                    /**< Modify session ID in outgoing WebRTC Offer command */                \
    X(CASEServerBusy, 17)                                /**< Respond to CASE_Sigma1 with BUSY status */                           \
    X(CASESkipInitiatorResumeMIC, 18)                    /**< Send Sigma1 with resumptionID but omit initiatorResumeMIC */         \
    X(CASESkipResumptionID, 19)                          /**< Send Sigma1 with initiatorResumeMIC but omit resumptionID */         \
    X(CASECorruptInitiatorResumeMIC, 20)                 /**< Send invalid initiatorResumeMIC in Sigma1 */                         \
    X(CASECorruptDestinationID, 21)                      /**< Send Sigma1 with invalid DestinationID */                            \
    X(CASECorruptTBEData3Encrypted, 22)                  /**< Send CASE_Sigma3 with improperly generated TBEData3Encrypted */      \
    X(CASECorruptSigma3NOC, 23)                          /**< Send Sigma3 with invalid initiatorNOC */                             \
    X(CASECorruptSigma3ICAC, 24)                         /**< Send Sigma3 with invalid initiatorICAC */                            \
    X(CASECorruptSigma3Signature, 25)                    /**< Send Sigma3 with invalid signature */                                \
    X(CASECorruptSigma3InitiatorEphPubKey, 26)           /**< Send Sigma3 with invalid initiator ephemeral public key */           \
    X(CASECorruptSigma3ResponderEphPubKey, 27)           /**< Send Sigma3 with invalid responder ephemeral public key */           \
    X(CASECorruptTBEData2Encrypted, 28)                  /**< Send Sigma2 with improperly generated TBEData2Encrypted */           \
    X(CASECorruptSigma2NOC, 29)                          /**< Send Sigma2 with invalid responderNOC */                             \
    X(CASECorruptSigma2ICAC, 30)                         /**< Send Sigma2 with invalid responderICAC */                            \
    X(CASECorruptSigma2Signature, 31)                    /**< Send Sigma2 with invalid signature */

// END-IF-CHANGE-ALSO-CHANGE(src/controller/python/chip/fault_injection/__init__.py)
// WARNING: When adding/modifying Faults to the below macro, make sure the changes are duplicated to the CHIPFaultId enum in the
// Python Module src/controller/python/chip/fault_injection/__init__.py

/**
 * @brief   Fault injection points
 *
 * @details
 * Each point in the code at which a fault can be injected
 * is identified by a member of this enum.
 *
 * @note IMPORTANT: This enum must be kept in sync with the CHIPFaultId enum in
 * src/controller/python/chip/fault_injection/__init__.py
 * If you change values here, update them there as well.
 */

enum Id
{
#define _CHIP_FAULTS_ENUMERATE_DECL(FAULT, ...) kFault_##FAULT,
    CHIP_FAULTS_ENUMERATE(_CHIP_FAULTS_ENUMERATE_DECL) //
    kFault_NumItems                                    // marker value
};

// Exporting kNumChipFaultsFromEnum for usage in Python bindings
const size_t kNumChipFaultsFromEnum = kFault_NumItems;

static_assert(kFault_IMInvoke_SeparateResponses == 12, "Test plan specification and automation code relies on this value being 12");
static_assert(kFault_IMInvoke_SeparateResponsesInvertResponseOrder == 13,
              "Test plan specification and automation code relies on this value being 13");
static_assert(kFault_IMInvoke_SkipSecondResponse == 14,
              "Test plan specification and automation code relies on this value being 14");
static_assert(kFault_ModifyWebRTCAnswerSessionId == 15,
              "Test plan specification and automation code relies on this value being 15");
static_assert(kFault_ModifyWebRTCOfferSessionId == 16, "Test plan specification and automation code relies on this value being 16");

DLL_EXPORT nl::FaultInjection::Manager & GetManager();

DLL_EXPORT uint32_t GetFaultCounter(uint32_t faultID);

/**
 * The number of ways in which chip Fault Injection fuzzers can
 * alter a byte in a payload.
 */
#define CHIP_FAULT_INJECTION_NUM_FUZZ_VALUES 3

DLL_EXPORT void FuzzExchangeHeader(uint8_t * p, int32_t arg);

} // namespace FaultInjection
} // namespace chip

/**
 * Execute the statements included if the chip fault is
 * to be injected.
 *
 * @param[in] aFaultID      A chip fault-injection id
 * @param[in] aStatements   Statements to be executed if the fault is enabled.
 */
#define CHIP_FAULT_INJECT(aFaultID, aStatements) nlFAULT_INJECT(chip::FaultInjection::GetManager(), aFaultID, aStatements)

/**
 * Execute the statements included if the chip fault is
 * to be injected. Also, if there are no arguments stored in the
 * fault, save aMaxArg into the record so it can be printed out
 * to the debug log by a callback installed on purpose.
 *
 * @param[in] aFaultID      A chip fault-injection id
 * @param[in] aMaxArg       The max value accepted as argument by the statements to be injected
 * @param[in] aProtectedStatements   Statements to be executed if the fault is enabled while holding the
 *                          Manager's lock
 * @param[in] aUnprotectedStatements   Statements to be executed if the fault is enabled without holding the
 *                          Manager's lock
 */
#define CHIP_FAULT_INJECT_MAX_ARG(aFaultID, aMaxArg, aProtectedStatements, aUnprotectedStatements)                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        nl::FaultInjection::Manager & mgr          = chip::FaultInjection::GetManager();                                           \
        const nl::FaultInjection::Record * records = mgr.GetFaultRecords();                                                        \
        if (records[aFaultID].mNumArguments == 0)                                                                                  \
        {                                                                                                                          \
            int32_t arg = aMaxArg;                                                                                                 \
            mgr.StoreArgsAtFault(aFaultID, 1, &arg);                                                                               \
        }                                                                                                                          \
        nlFAULT_INJECT_WITH_ARGS(mgr, aFaultID, aProtectedStatements, aUnprotectedStatements);                                     \
    } while (0)

/**
 * Execute the statements included if the chip fault is
 * to be injected.
 *
 * @param[in] aFaultID      A chip fault-injection id
 * @param[in] aProtectedStatements   Statements to be executed if the fault is enabled while holding the
 *                          Manager's lock
 * @param[in] aUnprotectedStatements   Statements to be executed if the fault is enabled without holding the
 *                          Manager's lock
 */
#define CHIP_FAULT_INJECT_WITH_ARGS(aFaultID, aProtectedStatements, aUnprotectedStatements)                                        \
    nlFAULT_INJECT_WITH_ARGS(chip::FaultInjection::GetManager(), aFaultID, aProtectedStatements, aUnprotectedStatements);

#define CHIP_FAULT_INJECTION_EXCH_HEADER_NUM_FIELDS 4
#define CHIP_FAULT_INJECTION_EXCH_HEADER_NUM_FIELDS_RMP 5

#else // CHIP_WITH_NLFAULTINJECTION

#define CHIP_FAULT_INJECT(aFaultID, aStatements)
#define CHIP_FAULT_INJECT_WITH_ARGS(aFaultID, aProtectedStatements, aUnprotectedStatements)
#define CHIP_FAULT_INJECT_MAX_ARG(aFaultID, aMaxArg, aProtectedStatements, aUnprotectedStatements)

#endif // CHIP_WITH_NLFAULTINJECTION
