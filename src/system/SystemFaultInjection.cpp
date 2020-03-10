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
 *      Implementation of the fault-injection utilities for Weave System Layer.
 */

#include <string.h>
#include <nlassert.h>
#include <SystemLayer/SystemFaultInjection.h>

#if WEAVE_SYSTEM_CONFIG_TEST

#include "SystemLayerPrivate.h"

namespace nl {
namespace Weave {
namespace System {
namespace FaultInjection {

using ::nl::FaultInjection::Record;
using ::nl::FaultInjection::Manager;
using ::nl::FaultInjection::Name;

static Record sFaultRecordArray[kFault_NumberOfFaultIdentifiers];
static Manager sManager;
static int32_t sFault_AsyncEvent_Arguments[1];
static const Name sManagerName = "WeaveSys";
static const Name sFaultNames[] = {
    "PacketBufferNew",
    "TimeoutImmediate",
    "AsyncEvent",
};

static int32_t (*sGetNumEventsAvailable)(void);
static void (*sInjectAsyncEvent)(int32_t index);

Manager& GetManager(void)
{
    if (0 == sManager.GetNumFaults())
    {
        sManager.Init(kFault_NumberOfFaultIdentifiers,
                      sFaultRecordArray,
                      sManagerName,
                      sFaultNames);

        memset(&sFault_AsyncEvent_Arguments, 0, sizeof(sFault_AsyncEvent_Arguments));
        sFaultRecordArray[kFault_AsyncEvent].mArguments = sFault_AsyncEvent_Arguments;
        sFaultRecordArray[kFault_AsyncEvent].mLengthOfArguments =
            static_cast<uint16_t>(sizeof(sFault_AsyncEvent_Arguments)/sizeof(sFault_AsyncEvent_Arguments[0]));
    }

    return sManager;
}

void InjectAsyncEvent(void)
{
    int32_t numEventsAvailable = 0;
    nl::Weave::System::FaultInjection::Id faultID = kFault_AsyncEvent;

    if (sGetNumEventsAvailable)
    {
        numEventsAvailable = sGetNumEventsAvailable();

        if (numEventsAvailable)
        {
            nl::FaultInjection::Manager &mgr = nl::Weave::System::FaultInjection::GetManager();
            const nl::FaultInjection::Record *record = &(mgr.GetFaultRecords()[faultID]);

            if (record->mNumArguments == 0)
            {
                int32_t maxEventIndex = numEventsAvailable - 1;

                mgr.StoreArgsAtFault(faultID, 1, &maxEventIndex);
            }

            nlFAULT_INJECT_WITH_ARGS(mgr, faultID,
                                    // Code executed with the Manager's lock:
                                        int32_t index = 0;
                                        if (numFaultArgs > 0)
                                        {
                                            index = faultArgs[0];
                                        }
                                    ,
                                    // Code executed without the Manager's lock:
                                        if (sInjectAsyncEvent)
                                        {
                                            sInjectAsyncEvent(index);
                                        }
                                    );
        }
    }
}

void SetAsyncEventCallbacks(int32_t (*aGetNumEventsAvailable)(void), void (*aInjectAsyncEvent)(int32_t index))
{
    sGetNumEventsAvailable = aGetNumEventsAvailable;
    sInjectAsyncEvent = aInjectAsyncEvent;
}


} // namespace FaultInjection
} // namespace System
} // namespace Weave
} // namespace nl

#endif // WEAVE_SYSTEM_CONFIG_TEST
