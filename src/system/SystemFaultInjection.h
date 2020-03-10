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
 *      Header file for the fault-injection utilities for Weave System Layer.
 */

#ifndef SYSTEMFAULTINJECTION_H
#define SYSTEMFAULTINJECTION_H

#include <SystemLayer/SystemConfig.h>

#if WEAVE_SYSTEM_CONFIG_TEST

#include <nlfaultinjection.hpp>

#include <Weave/Support/NLDLLUtil.h>

namespace nl {
namespace Weave {
namespace System {
namespace FaultInjection {

using ::nl::FaultInjection::Manager;

/**
 * @brief   Fault injection points
 *
 * @details
 * Each point in the code at which a fault can be injected
 * is identified by a member of this enum.
 */
typedef enum
{
    kFault_PacketBufferNew,             /**< Fail the allocation of a PacketBuffer */
    kFault_TimeoutImmediate,            /**< Override the timeout value of a timer being started with 0 */
    kFault_AsyncEvent,                  /**< Inject asynchronous events; when the fault is enabled, it expects
                                             one integer argument, which is passed to application to signal the event
                                             to be injected; @see WEAVE_SYSTEM_FAULT_INJECT_ASYNC_EVENT */
    kFault_NumberOfFaultIdentifiers,
} Id;

NL_DLL_EXPORT Manager& GetManager(void);

/**
 * Callback to the application that returns how many asynchronous events the application could
 * inject at the time of the call.
 *
 * @return The number of events
 */
typedef int32_t (*GetNumEventsAvailableCb)(void);

/**
 * Callback to the application to inject the asynchronous event specified by argument.
 *
 * @param[in]   aEventIndex     An index (0 to the value returned by GetNumEventsAvailableCb -1)
 *                              that identifies the event to be injected.
 */
typedef void (*InjectAsyncEventCb)(int32_t aEventIndex);

/**
 * Store the GetNumEventsAvailableCb and InjectAsyncEventCb callbacks used by
 * @see WEAVE_SYSTEM_FAULT_INJECT_ASYNC_EVENT
 *
 * @param[in] aGetNumEventsAvailable    A GetNumEventsAvailableCb
 * @param[in] aInjectAsyncEvent         An InjectAsyncEventCb
 *
 */
NL_DLL_EXPORT void SetAsyncEventCallbacks(GetNumEventsAvailableCb aGetNumEventsAvailable, InjectAsyncEventCb aInjectAsyncEvent);

/**
 * @see WEAVE_SYSTEM_FAULT_INJECT_ASYNC_EVENT
 */
NL_DLL_EXPORT void InjectAsyncEvent(void);


} // namespace FaultInjection
} // namespace System
} // namespace Weave
} // namespace nl

/**
 * Execute the statements included if the System fault is
 * to be injected.
 *
 * @param[in] aFaultID      A System fault-injection id
 * @param[in] aStatements   Statements to be executed if the fault is enabled.
 */
#define WEAVE_SYSTEM_FAULT_INJECT(aFaultId, aStatement) \
        nlFAULT_INJECT(::nl::Weave::System::FaultInjection::GetManager(), aFaultId, aStatement)

/**
 * This macro implements the injection of asynchronous events.
 *
 * It polls the application by calling the GetNumEventsAvailableCb callback
 * to know if there are asynchronous events that can be injected.
 * If there are any, it instances kFault_AsyncEvent.
 * If the fault is to be injected, the code injected calls the InjectAsyncEventCb
 * callback passing the integer argument stored in the fault Record.
 * If the fault is not configured (and therefore no arguments are stored in the Record)
 * the macro stores the return value of GetNumEventsAvailableCb into the Records arguments,
 * so that the application can log it from a callback installed into the fault.
 */
#define WEAVE_SYSTEM_FAULT_INJECT_ASYNC_EVENT() \
    do { \
        nl::Weave::System::FaultInjection::InjectAsyncEvent(); \
    } while (0)


#else // WEAVE_SYSTEM_CONFIG_TEST

#define WEAVE_SYSTEM_FAULT_INJECT(aFaultId, aStatement)

#define WEAVE_SYSTEM_FAULT_INJECT_ASYNC_EVENT()

#endif // WEAVE_SYSTEM_CONFIG_TEST

#endif // SYSTEMFAULTINJECTION_H
