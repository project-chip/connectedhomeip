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
 *      Header file for the fault-injection utilities for CHIP System Layer.
 */

#pragma once

#include <system/SystemConfig.h>

#if CHIP_WITH_NLFAULTINJECTION

#include <nlfaultinjection.hpp>

#include <lib/support/DLLUtil.h>

namespace chip {
namespace System {
namespace FaultInjection {

using nl::FaultInjection::Manager;

/**
 * @brief   Fault injection points
 *
 * @details
 * Each point in the code at which a fault can be injected
 * is identified by a member of this enum.
 */
typedef enum
{
    kFault_PacketBufferNew,  /**< Fail the allocation of a PacketBuffer */
    kFault_TimeoutImmediate, /**< Override the timeout value of a timer being started with 0 */
    kFault_AsyncEvent,       /**< Inject asynchronous events; when the fault is enabled, it expects
                                  one integer argument, which is passed to application to signal the event
                                  to be injected; @see CHIP_SYSTEM_FAULT_INJECT_ASYNC_EVENT */
    kFault_NumberOfFaultIdentifiers,
} Id;

DLL_EXPORT nl::FaultInjection::Manager & GetManager();

/**
 * Callback to the application that returns how many asynchronous events the application could
 * inject at the time of the call.
 *
 * @return The number of events
 */
typedef int32_t (*GetNumEventsAvailableCb)();

/**
 * Callback to the application to inject the asynchronous event specified by argument.
 *
 * @param[in]   aEventIndex     An index (0 to the value returned by GetNumEventsAvailableCb -1)
 *                              that identifies the event to be injected.
 */
typedef void (*InjectAsyncEventCb)(int32_t aEventIndex);

/**
 * Store the GetNumEventsAvailableCb and InjectAsyncEventCb callbacks used by
 * @see CHIP_SYSTEM_FAULT_INJECT_ASYNC_EVENT
 *
 * @param[in] aGetNumEventsAvailable    A GetNumEventsAvailableCb
 * @param[in] aInjectAsyncEvent         An InjectAsyncEventCb
 *
 */
DLL_EXPORT void SetAsyncEventCallbacks(GetNumEventsAvailableCb aGetNumEventsAvailable, InjectAsyncEventCb aInjectAsyncEvent);

/**
 * @see CHIP_SYSTEM_FAULT_INJECT_ASYNC_EVENT
 */
DLL_EXPORT void InjectAsyncEvent();

} // namespace FaultInjection
} // namespace System
} // namespace chip

/**
 * Execute the statements included if the System fault is
 * to be injected.
 *
 * @param[in] aFaultID      A System fault-injection id
 * @param[in] aStatements   Statements to be executed if the fault is enabled.
 */
#define CHIP_SYSTEM_FAULT_INJECT(aFaultId, aStatement)                                                                             \
    nlFAULT_INJECT(::chip::System::FaultInjection::GetManager(), aFaultId, aStatement)

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
#define CHIP_SYSTEM_FAULT_INJECT_ASYNC_EVENT()                                                                                     \
    do                                                                                                                             \
    {                                                                                                                              \
        chip::System::FaultInjection::InjectAsyncEvent();                                                                          \
    } while (0)

#else // CHIP_WITH_NLFAULTINJECTION

#define CHIP_SYSTEM_FAULT_INJECT(aFaultId, aStatement)

#define CHIP_SYSTEM_FAULT_INJECT_ASYNC_EVENT()

#endif // CHIP_WITH_NLFAULTINJECTION
