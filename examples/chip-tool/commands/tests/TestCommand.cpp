/*
 *   Copyright (c) 2021 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "TestCommand.h"

constexpr uint16_t kWaitDurationInSeconds = 30;

static void test_os_sleep_ms(uint64_t millisecs)
{
    struct timespec sleep_time;
    uint64_t s = millisecs / 1000;

    millisecs -= s * 1000;
    sleep_time.tv_sec  = static_cast<time_t>(s);
    sleep_time.tv_nsec = static_cast<long>(millisecs * 1000000);

    nanosleep(&sleep_time, nullptr);
}

CHIP_ERROR TestCommand::Run(PersistentStorage & storage, NodeId localId, NodeId remoteId)
{
    ReturnErrorOnFailure(mOpCredsIssuer.Initialize(storage));

    chip::Controller::CommissionerInitParams params;

    params.storageDelegate                = &storage;
    params.operationalCredentialsDelegate = &mOpCredsIssuer;

    ReturnErrorOnFailure(mCommissioner.SetUdpListenPort(storage.GetListenPort()));
    ReturnErrorOnFailure(mCommissioner.Init(localId, params));
    ReturnErrorOnFailure(mCommissioner.ServiceEvents());
    ReturnErrorOnFailure(mCommissioner.GetDevice(remoteId, &mDevice));

    ReturnErrorOnFailure(NextTest());

    UpdateWaitForResponse(true);
    WaitForResponse(kWaitDurationInSeconds);

    mCommissioner.ServiceEventSignal();

    // Give some time for all the pending messages to flush before shutting down
    // Note: This is working around racy code in message queues during shutdown
    // TODO: Remove this workaround once we understand the message queue and shutdown race
    test_os_sleep_ms(1000);
    mCommissioner.Shutdown();

    VerifyOrReturnError(GetCommandExitStatus(), CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}
