/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "RvcAppCommandDelegate.h"
#include "rvc-device.h"
#include <AppMain.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

// Todo s
//  - Add a named pipe for out-of-band messages
//  - Add and handle the following out-of-band messages
//    - charged
//    - docked?
//    - charger found
//    - low charge
//    - activity complete
//    - error event

namespace {
constexpr const char kChipEventFifoPathPrefix[] = "/tmp/chip_rvc_fifo_";
NamedPipeCommands sChipNamedPipeCommands;
RvcAppCommandDelegate sRvcAppCommandDelegate;
} // namespace

RvcDevice * rvcDevice = nullptr;

void ApplicationInit()
{
    rvcDevice = new RvcDevice;
    rvcDevice->Init();
}

void ApplicationShutdown()
{
    delete rvcDevice;
    rvcDevice = nullptr;
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    std::string path = kChipEventFifoPathPrefix + std::to_string(getpid());

    if (sChipNamedPipeCommands.Start(path, &sRvcAppCommandDelegate) != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        sChipNamedPipeCommands.Stop();
    }

    ChipLinuxAppMainLoop();
    return 0;
}
