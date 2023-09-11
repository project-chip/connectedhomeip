/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppMain.h"
#include <app-common/zap-generated/ids/Clusters.h>

#include "LockAppCommandDelegate.h"
#include "LockManager.h"

using namespace chip;
using namespace chip::app;

namespace {
// Variables for handling named pipe commands
constexpr const char kChipEventFifoPathPrefix[] = "/tmp/chip_lock_app_fifo-";
NamedPipeCommands sChipNamedPipeCommands;
LockAppCommandDelegate sLockAppCommandDelegate;

} // anonymous namespace

void ApplicationInit()
{
    auto path = kChipEventFifoPathPrefix + std::to_string(getpid());
    if (sChipNamedPipeCommands.Start(path, &sLockAppCommandDelegate) != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        sChipNamedPipeCommands.Stop();
    }
}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);
    ChipLinuxAppMainLoop();
    return 0;
}

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    // TODO: Watch for LockState, DoorState, Mode, etc changes and trigger appropriate action
    if (attributePath.mClusterId == Clusters::DoorLock::Id)
    {
        ChipLogProgress(Zcl, "Door Lock attribute changed");
    }
}
