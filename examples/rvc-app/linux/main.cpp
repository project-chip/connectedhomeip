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

#define RVC_ENDPOINT 1

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {
constexpr char kChipEventFifoPathPrefix[] = "/tmp/chip_rvc_fifo_";
NamedPipeCommands sChipNamedPipeCommands;
RvcAppCommandDelegate sRvcAppCommandDelegate;
} // namespace

RvcDevice * gRvcDevice = nullptr;

void ApplicationInit()
{
    std::string path = kChipEventFifoPathPrefix + std::to_string(getpid());

    if (sChipNamedPipeCommands.Start(path, &sRvcAppCommandDelegate) != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        sChipNamedPipeCommands.Stop();
    }

    gRvcDevice = new RvcDevice(RVC_ENDPOINT);
    gRvcDevice->Init();

    sRvcAppCommandDelegate.SetRvcDevice(gRvcDevice);
}

void ApplicationShutdown()
{
    delete gRvcDevice;
    gRvcDevice = nullptr;

    sChipNamedPipeCommands.Stop();
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();
    return 0;
}
