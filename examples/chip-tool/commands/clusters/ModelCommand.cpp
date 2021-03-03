/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#include "ModelCommand.h"

#include <inttypes.h>

using namespace ::chip;

namespace {
constexpr uint16_t kWaitDurationInSeconds = 10;
} // namespace

CHIP_ERROR ModelCommand::Run(PersistentStorage & storage, NodeId localId, NodeId remoteId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mCommissioner.SetUdpListenPort(storage.GetListenPort());
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Commissioner: %s", ErrorStr(err)));

    err = mCommissioner.Init(localId, &storage);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Commissioner: %s", ErrorStr(err)));

    err = mCommissioner.ServiceEvents();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Run Loop: %s", ErrorStr(err)));

    err = mCommissioner.GetDevice(remoteId, &mDevice);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Init failure! No pairing for device: %" PRIu64, localId));

    err = SendCommand(mDevice, mEndPointId);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Failed to send message: %s", ErrorStr(err)));

    UpdateWaitForResponse(true);
    WaitForResponse(kWaitDurationInSeconds);

    VerifyOrExit(GetCommandExitStatus(), err = CHIP_ERROR_INTERNAL);

exit:
    mCommissioner.ServiceEventSignal();
    mCommissioner.Shutdown();
    return err;
}
