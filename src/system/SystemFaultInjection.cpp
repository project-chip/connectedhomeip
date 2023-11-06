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

#include <system/SystemConfig.h>

/**
 *    @file
 *      Implementation of the fault-injection utilities for CHIP System Layer.
 */
/* module header, also carries config, comes first */
#include <system/SystemFaultInjection.h>

#include <nlassert.h>
#include <string.h>

namespace chip {
namespace System {
namespace FaultInjection {

using nl::FaultInjection::Manager;
using nl::FaultInjection::Name;
using nl::FaultInjection::Record;

static Record sFaultRecordArray[kFault_NumberOfFaultIdentifiers];
static Manager sManager;
static int32_t sFault_AsyncEvent_Arguments[1];
static const Name sManagerName  = "CHIPSys";
static const Name sFaultNames[] = {
    "PacketBufferNew",
    "TimeoutImmediate",
    "AsyncEvent",
};

static int32_t (*sGetNumEventsAvailable)();
static void (*sInjectAsyncEvent)(int32_t index);

Manager & GetManager()
{
    if (0 == sManager.GetNumFaults())
    {
        sManager.Init(kFault_NumberOfFaultIdentifiers, sFaultRecordArray, sManagerName, sFaultNames);

        memset(&sFault_AsyncEvent_Arguments, 0, sizeof(sFault_AsyncEvent_Arguments));
        sFaultRecordArray[kFault_AsyncEvent].mArguments = sFault_AsyncEvent_Arguments;
        sFaultRecordArray[kFault_AsyncEvent].mLengthOfArguments =
            static_cast<uint16_t>(sizeof(sFault_AsyncEvent_Arguments) / sizeof(sFault_AsyncEvent_Arguments[0]));
    }

    return sManager;
}

void InjectAsyncEvent()
{
    int32_t numEventsAvailable               = 0;
    chip::System::FaultInjection::Id faultID = kFault_AsyncEvent;

    if (sGetNumEventsAvailable)
    {
        numEventsAvailable = sGetNumEventsAvailable();

        if (numEventsAvailable)
        {
            FaultInjection::Manager & mgr         = chip::System::FaultInjection::GetManager();
            const FaultInjection::Record * record = &(mgr.GetFaultRecords()[faultID]);

            if (record->mNumArguments == 0)
            {
                int32_t maxEventIndex = numEventsAvailable - 1;

                mgr.StoreArgsAtFault(faultID, 1, &maxEventIndex);
            }

            nlFAULT_INJECT_WITH_ARGS(
                mgr, faultID,
                // Code executed with the Manager's lock:
                int32_t index = 0;
                if (numFaultArgs > 0) { index = faultArgs[0]; },
                // Code executed without the Manager's lock:
                if (sInjectAsyncEvent) { sInjectAsyncEvent(index); });
        }
    }
}

void SetAsyncEventCallbacks(int32_t (*aGetNumEventsAvailable)(), void (*aInjectAsyncEvent)(int32_t index))
{
    sGetNumEventsAvailable = aGetNumEventsAvailable;
    sInjectAsyncEvent      = aInjectAsyncEvent;
}

} // namespace FaultInjection
} // namespace System
} // namespace chip
