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

/**
 * @brief   Fault injection points
 *
 * @details
 * Each point in the code at which a fault can be injected
 * is identified by a member of this enum.
 */
typedef enum
{
    kFault_AllocExchangeContext, /**< Fail the allocation of an ExchangeContext */
    kFault_DropIncomingUDPMsg,   /**< Drop an incoming UDP message without any processing */
    kFault_DropOutgoingUDPMsg,   /**< Drop an outgoing UDP message at the chip Message layer */
    kFault_AllocBinding,         /**< Fail the allocation of a Binding */
    kFault_SendAlarm,            /**< Fail to send an alarm message */
    kFault_HandleAlarm,          /**< Fail to handle an alarm message */
    kFault_FuzzExchangeHeaderTx, /**< Fuzz a chip Exchange Header after it has been encoded into the packet buffer;
                                      when the fault is enabled, it expects an integer argument, which is an index into
                                      a table of modifications that can be applied to the header. @see FuzzExchangeHeader */
    kFault_RMPDoubleTx,          /**< Force RMP to transmit the outgoing message twice */
    kFault_RMPSendError,         /**< Fail a transmission in RMP as if the max number of retransmission has been exceeded */
    kFault_BDXBadBlockCounter,   /**< Corrupt the BDX Block Counter in the BDX BlockSend or BlockEOF message about to be sent */
    kFault_BDXAllocTransfer,     /**< Fail the allocation of a BDXTransfer object */
    kFault_SecMgrBusy,           /**< Trigger a WEAVE_ERROR_SECURITY_MANAGER_BUSY when starting an authentication session */
    kFault_IMInvoke_SeparateResponses, /**< Validate incoming InvokeRequestMessage contains exactly 2 valid commands and respond
                                        with 2 InvokeResponseMessages */
    kFault_IMInvoke_SeparateResponsesInvertResponseOrder, /**< Validate incoming InvokeRequestMessage contains exactly 2 valid
                                        commands and respond with 2 InvokeResponseMessages where the response order is inverted
                                        compared to the request order */
    kFault_IMInvoke_SkipSecondResponse, /**< Validate incoming InvokeRequestMessage contains exactly 2 valid commands and respond
                                        with 1 InvokeResponseMessage, dropping the response to the second request */
#if CONFIG_NETWORK_LAYER_BLE
    kFault_CHIPOBLESend, /**< Inject a GATT error when sending the first fragment of a chip message over BLE */
#endif
    kFault_CASEServerBusy, /**< Respond to CASE_Sigma1 with a BUSY status */
    kFault_NumItems,
} Id;

static_assert(kFault_IMInvoke_SeparateResponses == 12, "Test plan specification and automation code relies on this value being 12");
static_assert(kFault_IMInvoke_SeparateResponsesInvertResponseOrder == 13,
              "Test plan specification and automation code relies on this value being 13");
static_assert(kFault_IMInvoke_SkipSecondResponse == 14,
              "Test plan specification and automation code relies on this value being 14");

DLL_EXPORT nl::FaultInjection::Manager & GetManager();

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
