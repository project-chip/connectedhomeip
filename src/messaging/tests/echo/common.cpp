/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file implements constants, globals and interfaces common
 *      to and used by CHIP example applications.
 *
 */

#include <errno.h>

#include "common.h"
#include <lib/core/CHIPCore.h>
#include <lib/support/ErrorStr.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>

chip::FabricTable gFabricTable;
chip::SessionManager gSessionManager;
chip::Messaging::ExchangeManager gExchangeManager;
chip::secure_channel::MessageCounterManager gMessageCounterManager;
chip::TestPersistentStorageDelegate gStorage;
chip::Crypto::DefaultSessionKeystore gSessionKeystore;

void InitializeChip()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    printf("Init CHIP Stack\r\n");

    // Initialize System memory and resources
    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

    // Initialize the CHIP stack.
    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    // Initialize TCP.
    err = chip::DeviceLayer::TCPEndPointManager()->Init(chip::DeviceLayer::SystemLayer());
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("Failed to init CHIP Stack with err: %s\r\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }
}

void ShutdownChip()
{
    gMessageCounterManager.Shutdown();
    gExchangeManager.Shutdown();
    gSessionManager.Shutdown();
    (void) chip::DeviceLayer::TCPEndPointManager()->Shutdown();
    chip::DeviceLayer::PlatformMgr().Shutdown();
}
