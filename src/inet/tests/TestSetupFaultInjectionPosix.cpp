/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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
 *      This file sets up fault injection for all POSIX CHIP Inet layer library test
 *      applications and tools.
 *
 *      NOTE: These do not comprise a public part of the CHIP API and
 *            are subject to change without notice.
 *
 */

#include "TestInetCommonOptions.h"
#include "TestSetupFaultInjection.h"
#include <inet/InetFaultInjection.h>
#include <lib/support/CHIPFaultInjection.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <stdio.h>
#include <system/SystemFaultInjection.h>

struct RestartCallbackContext
{
    int mArgc;
    char ** mArgv;
};

static void RebootCallbackFn();
static void PostInjectionCallbackFn(nl::FaultInjection::Manager * aManager, nl::FaultInjection::Identifier aId,
                                    nl::FaultInjection::Record * aFaultRecord);

static struct RestartCallbackContext sRestartCallbackCtx;
static nl::FaultInjection::Callback sFuzzECHeaderCb;
static nl::FaultInjection::Callback sAsyncEventCb;

// clang-format off
static nl::FaultInjection::GlobalContext sFaultInjectionGlobalContext = {
    {
        RebootCallbackFn,
        PostInjectionCallbackFn
    }
};
// clang-format on

bool gSigusr1Received = false;

static void RebootCallbackFn()
{
    size_t i;
    size_t j = 0;
    chip::Platform::ScopedMemoryBuffer<char *> lArgv;
    if (!lArgv.Alloc(static_cast<size_t>(sRestartCallbackCtx.mArgc + 2)))
    {
        printf("** failed to allocate memory **\n");
        return;
    }

    if (gSigusr1Received)
    {
        printf("** skipping restart case after SIGUSR1 **\n");
        return;
    }

    for (i = 0; sRestartCallbackCtx.mArgv[i] != nullptr; i++)
    {
        if (strcmp(sRestartCallbackCtx.mArgv[i], "--faults") == 0)
        {
            // Skip the --faults argument for now
            i++;
            continue;
        }
        lArgv[j++] = sRestartCallbackCtx.mArgv[i];
    }

    lArgv[j] = nullptr;

    for (size_t idx = 0; lArgv[idx] != nullptr; idx++)
    {
        printf("argv[%d]: %s\n", static_cast<int>(idx), lArgv[idx]);
    }

    // Need to close any open file descriptor above stdin/out/err.
    // There is no portable way to get the max fd number.
    // Given that CHIP's test apps don't open a large number of files,
    // FD_SETSIZE should be a reasonable upper bound (see the documentation
    // of select).
    for (int fd = 3; fd < FD_SETSIZE; fd++)
    {
        close(fd);
    }

    printf("********** Restarting *********\n");
    fflush(stdout);
    execvp(lArgv[0], lArgv.Get());
}

static void PostInjectionCallbackFn(nl::FaultInjection::Manager * aManager, nl::FaultInjection::Identifier aId,
                                    nl::FaultInjection::Record * aFaultRecord)
{
    uint16_t numargs = aFaultRecord->mNumArguments;
    uint16_t i;

    printf("***** Injecting fault %s_%s, instance number: %" PRIu32 "; reboot: %s", aManager->GetName(),
           aManager->GetFaultNames()[aId], aFaultRecord->mNumTimesChecked, aFaultRecord->mReboot ? "yes" : "no");
    if (numargs)
    {
        printf(" with %u args:", numargs);

        for (i = 0; i < numargs; i++)
        {
            printf(" %" PRIi32, aFaultRecord->mArguments[i]);
        }
    }

    printf("\n");
}

static bool PrintFaultInjectionMaxArgCbFn(nl::FaultInjection::Manager & mgr, nl::FaultInjection::Identifier aId,
                                          nl::FaultInjection::Record * aFaultRecord, void * aContext)
{
    const char * faultName = mgr.GetFaultNames()[aId];

    if (gFaultInjectionOptions.PrintFaultCounters && aFaultRecord->mNumArguments)
    {
        printf("FI_instance_params: %s_%s_s%" PRIu32 " maxArg: %" PRIi32 ";\n", mgr.GetName(), faultName,
               aFaultRecord->mNumTimesChecked, aFaultRecord->mArguments[0]);
    }

    return false;
}

static bool PrintCHIPFaultInjectionMaxArgCbFn(nl::FaultInjection::Identifier aId, nl::FaultInjection::Record * aFaultRecord,
                                              void * aContext)
{
    nl::FaultInjection::Manager & mgr = chip::FaultInjection::GetManager();

    return PrintFaultInjectionMaxArgCbFn(mgr, aId, aFaultRecord, aContext);
}

static bool PrintSystemFaultInjectionMaxArgCbFn(nl::FaultInjection::Identifier aId, nl::FaultInjection::Record * aFaultRecord,
                                                void * aContext)
{
    nl::FaultInjection::Manager & mgr = chip::System::FaultInjection::GetManager();

    return PrintFaultInjectionMaxArgCbFn(mgr, aId, aFaultRecord, aContext);
}

void SetupFaultInjectionContext(int argc, char * argv[])
{
    SetupFaultInjectionContext(argc, argv, nullptr, nullptr);
}

void SetupFaultInjectionContext(int argc, char * argv[], int32_t (*aNumEventsAvailable)(),
                                void (*aInjectAsyncEvents)(int32_t index))
{
    nl::FaultInjection::Manager & weavemgr  = chip::FaultInjection::GetManager();
    nl::FaultInjection::Manager & systemmgr = chip::System::FaultInjection::GetManager();

    sRestartCallbackCtx.mArgc = argc;
    sRestartCallbackCtx.mArgv = argv;

    nl::FaultInjection::SetGlobalContext(&sFaultInjectionGlobalContext);

    memset(&sFuzzECHeaderCb, 0, sizeof(sFuzzECHeaderCb));
    sFuzzECHeaderCb.mCallBackFn = PrintCHIPFaultInjectionMaxArgCbFn;
    weavemgr.InsertCallbackAtFault(chip::FaultInjection::kFault_FuzzExchangeHeaderTx, &sFuzzECHeaderCb);

    if (aNumEventsAvailable && aInjectAsyncEvents)
    {
        memset(&sAsyncEventCb, 0, sizeof(sAsyncEventCb));
        sAsyncEventCb.mCallBackFn = PrintSystemFaultInjectionMaxArgCbFn;
        systemmgr.InsertCallbackAtFault(chip::System::FaultInjection::kFault_AsyncEvent, &sAsyncEventCb);

        chip::System::FaultInjection::SetAsyncEventCallbacks(aNumEventsAvailable, aInjectAsyncEvents);
    }
}
