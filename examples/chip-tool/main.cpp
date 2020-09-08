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

#include "commands/Commands.h"

using namespace ::chip;

// NOTE: Remote device ID is in sync with the echo server device id
//       At some point, we may want to add an option to connect to a device without
//       knowing its id, because the ID can be learned on the first response that is received.
constexpr NodeId kLocalDeviceId  = 112233;
constexpr NodeId kRemoteDeviceId = 12344321;

// ================================================================================
// Main Code
// ================================================================================
int main(int argc, char * argv[])
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    ChipDeviceController * dc = new DeviceController::ChipDeviceController();

    err = dc->Init(kLocalDeviceId);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure: %s", ErrorStr(err)));

    err = RunCommand(dc, kRemoteDeviceId, argc, argv);
    SuccessOrExit(err);

    err = dc->ServiceEvents();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init Run Loop failure: %s", ErrorStr(err)));

exit:
    dc->Shutdown();
    delete dc;

    return (err == CHIP_NO_ERROR) ? EXIT_SUCCESS : EXIT_FAILURE;
}
