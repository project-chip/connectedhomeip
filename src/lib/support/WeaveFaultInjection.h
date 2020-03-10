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
 *      Header file for the fault-injection utilities for Inet.
 */

#ifndef WEAVE_FAULT_INJECTION_H_
#define WEAVE_FAULT_INJECTION_H_

#include <Weave/Core/WeaveConfig.h>
#include <Weave/Core/WeaveWRMPConfig.h>
#include <Weave/Core/WeaveBDXConfig.h>
#include <Weave/Core/WeaveEventLoggingConfig.h>
#include <Weave/Core/WeaveTimeConfig.h>
#include <Weave/Core/WeaveTunnelConfig.h>
#include <Weave/Core/WeaveWRMPConfig.h>

#if WEAVE_CONFIG_TEST

#include <nlfaultinjection.hpp>

#include <Weave/Support/NLDLLUtil.h>

namespace nl {
namespace Weave {
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
    kFault_AllocExchangeContext,                /**< Fail the allocation of an ExchangeContext */
    kFault_DropIncomingUDPMsg,                  /**< Drop an incoming UDP message without any processing */
    kFault_DropOutgoingUDPMsg,                  /**< Drop an outgoing UDP message at the Weave Message layer */
    kFault_AllocBinding,                        /**< Fail the allocation of a Binding */
    kFault_SendAlarm,                           /**< Fail to send an alarm message */
    kFault_HandleAlarm,                         /**< Fail to handle an alarm message */
    kFault_FuzzExchangeHeaderTx,                /**< Fuzz a Weave Exchange Header after it has been encoded into the packet buffer;
                                                     when the fault is enabled, it expects an integer argument, which is an index into
                                                     a table of modifications that can be applied to the header. @see FuzzExchangeHeader */
#if WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING
    kFault_WRMDoubleTx,                         /**< Force WRMP to transmit the outgoing message twice */
    kFault_WRMSendError,                        /**< Fail a transmission in WRMP as if the max number of retransmission has been exceeded */
#endif // WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING
    kFault_BDXBadBlockCounter,                  /**< Corrupt the BDX Block Counter in the BDX BlockSend or BlockEOF message about to be sent */
    kFault_BDXAllocTransfer,                    /**< Fail the allocation of a BDXTransfer object */
#if WEAVE_CONFIG_ENABLE_SERVICE_DIRECTORY
    kFault_ServiceManager_ConnectRequestNew,    /**< Fail the allocation of a WeaveServiceManager::ConnectRequest */
    kFault_ServiceManager_Lookup,               /**< Fail the lookup of an endpoint id */
    kFault_ServiceDirectoryReplaceError,        /**< Fail the replacement of a ServiceDirectory entry */
#endif // WEAVE_CONFIG_ENABLE_SERVICE_DIRECTORY
    kFault_WDM_TraitInstanceNew,                /**< Fail the allocation of a WDM TraitInstanceInfo object */
    kFault_WDM_SubscriptionHandlerNew,          /**< Fail the allocation of a WDM SubscriptionHandler object */
    kFault_WDM_SubscriptionClientNew,           /**< Fail the allocation of a WDM SubscriptionClient object */
    kFault_WDM_BadSubscriptionId,               /**< Corrupt the SubscriptionId of an incoming notification */
    kFault_WDM_SendUnsupportedReqMsgType,       /**< Corrupt the message type of an outgoing SubscriptionRequest, so it is received as an unsupported
                                                     message by the responder */
    kFault_WDM_NotificationSize,                /**< Override the max payload size in a SubscriptionHandler; the size to be used can passed as
                                                     an argument to the fault */
    kFault_WDM_SendCommandExpired,              /**< Force the ExpiryTime of a WDM command to be in the past */
    kFault_WDM_SendCommandBadVersion,           /**< Alter the version of a WDM command being transmitted */
    kFault_WDM_SendUpdateBadVersion,            /**< Alter the version of a WDM update data element being transmitted */
    kFault_WDM_DelayUpdateResponse,             /**< Drop the message received after sending an UpdateRequest, which usually is the StatusReport;
                                                     this causes the NotificationRequest to be processed first */
    kFault_WDM_UpdateRequestTimeout,            /**< Inject an exchange timeout for the UpdateRequest */
    kFault_WDM_UpdateRequestSendErrorInline,    /**< Inject an inline Inet Send error for the UpdateRequest */
    kFault_WDM_UpdateRequestSendErrorAsync,     /**< Inject a WRM SendError for the UpdateRequest */
    kFault_WDM_UpdateRequestBadProfile,         /**< Inject an invalid Profile ID in the UpdateRequest */
    kFault_WDM_UpdateRequestDropMessage,        /**< Drop an outgoing WDM UpdateRequest message using the DropOutgoingUDPMsg fault */
    kFault_WDM_UpdateResponseBusy,              /**< Inject a status code busy in the StatusList */
    kFault_WDM_PathStoreFull,                   /**< Inject a WDM_PATH_STORE_FULL error */
    kFault_WDM_TreatNotifyAsCancel,             /**< Process a Notify request as a CancelSubscription request */
    kFault_CASEKeyConfirm,                      /**< Trigger a WEAVE_ERROR_KEY_CONFIRMATION_FAILED error in WeaveCASEEngine */
    kFault_SecMgrBusy,                          /**< Trigger a WEAVE_ERROR_SECURITY_MANAGER_BUSY when starting an authentication session */
#if WEAVE_CONFIG_ENABLE_TUNNELING
    kFault_TunnelQueueFull,                     /**< Trigger a WEAVE_ERROR_TUNNEL_SERVICE_QUEUE_FULL when enqueueing a packet in the Tunnel queue */
    kFault_TunnelPacketDropByPolicy,            /**< Trigger an explicit drop of the packet as if done by an application policy */
#endif // WEAVE_CONFIG_ENABLE_TUNNELING
#if CONFIG_NETWORK_LAYER_BLE
    kFault_WOBLESend,                           /**< Inject a GATT error when sending the first fragment of a Weave message over BLE */
#endif // CONFIG_NETWORK_LAYER_BLE
    kFault_NumItems,
} Id;

NL_DLL_EXPORT nl::FaultInjection::Manager &GetManager(void);

/**
 * The number of ways in which Weave Fault Injection fuzzers can
 * alter a byte in a payload.
 */
#define WEAVE_FAULT_INJECTION_NUM_FUZZ_VALUES 3

NL_DLL_EXPORT void FuzzExchangeHeader(uint8_t *p, int32_t arg);

} // namespace FaultInjection
} // namespace Weave
} // namespace nl

/**
 * Execute the statements included if the Weave fault is
 * to be injected.
 *
 * @param[in] aFaultID      A Weave fault-injection id
 * @param[in] aStatements   Statements to be executed if the fault is enabled.
 */
#define WEAVE_FAULT_INJECT( aFaultID, aStatements ) \
        nlFAULT_INJECT(nl::Weave::FaultInjection::GetManager(), aFaultID, aStatements)

/**
 * Execute the statements included if the Weave fault is
 * to be injected. Also, if there are no arguments stored in the
 * fault, save aMaxArg into the record so it can be printed out
 * to the debug log by a callback installed on purpose.
 *
 * @param[in] aFaultID      A Weave fault-injection id
 * @param[in] aMaxArg       The max value accepted as argument by the statements to be injected
 * @param[in] aProtectedStatements   Statements to be executed if the fault is enabled while holding the
 *                          Manager's lock
 * @param[in] aUnprotectedStatements   Statements to be executed if the fault is enabled without holding the
 *                          Manager's lock
 */
#define WEAVE_FAULT_INJECT_MAX_ARG( aFaultID, aMaxArg, aProtectedStatements, aUnprotectedStatements ) \
    do { \
        nl::FaultInjection::Manager &mgr = nl::Weave::FaultInjection::GetManager(); \
        const nl::FaultInjection::Record *records = mgr.GetFaultRecords(); \
        if (records[aFaultID].mNumArguments == 0) \
        { \
            int32_t arg = aMaxArg; \
            mgr.StoreArgsAtFault(aFaultID, 1, &arg); \
        } \
        nlFAULT_INJECT_WITH_ARGS(mgr, aFaultID, aProtectedStatements, aUnprotectedStatements ); \
    } while (0)

/**
 * Execute the statements included if the Weave fault is
 * to be injected.
 *
 * @param[in] aFaultID      A Weave fault-injection id
 * @param[in] aProtectedStatements   Statements to be executed if the fault is enabled while holding the
 *                          Manager's lock
 * @param[in] aUnprotectedStatements   Statements to be executed if the fault is enabled without holding the
 *                          Manager's lock
 */
#define WEAVE_FAULT_INJECT_WITH_ARGS( aFaultID, aProtectedStatements, aUnprotectedStatements ) \
        nlFAULT_INJECT_WITH_ARGS(nl::Weave::FaultInjection::GetManager(), aFaultID,  \
                                 aProtectedStatements, aUnprotectedStatements );

#define WEAVE_FAULT_INJECTION_EXCH_HEADER_NUM_FIELDS 4
#define WEAVE_FAULT_INJECTION_EXCH_HEADER_NUM_FIELDS_WRMP 5

#else // WEAVE_CONFIG_TEST

#define WEAVE_FAULT_INJECT( aFaultID, aStatements )
#define WEAVE_FAULT_INJECT_WITH_ARGS( aFaultID, aProtectedStatements, aUnprotectedStatements )
#define WEAVE_FAULT_INJECT_MAX_ARG( aFaultID, aMaxArg, aProtectedStatements, aUnprotectedStatements )

#endif // WEAVE_CONFIG_TEST


#endif // WEAVE_FAULT_INJECTION_H_
