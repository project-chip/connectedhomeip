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

#include <app/tests/integration/common.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLVDebug.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/Constants.h>
#include <platform/CHIPDeviceLayer.h>

chip::FabricTable gFabricTable;
chip::Messaging::ExchangeManager gExchangeManager;
chip::SessionManager gSessionManager;
chip::secure_channel::MessageCounterManager gMessageCounterManager;
chip::SessionHolder gSession;
chip::TestPersistentStorageDelegate gStorage;
chip::PersistentStorageOperationalKeystore gOperationalKeystore;
chip::Credentials::PersistentStorageOpCertStore gOpCertStore;
chip::Crypto::DefaultSessionKeystore gSessionKeystore;

void InitializeChip()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::FabricTable::InitParams fabricTableInitParams;

    printf("Init CHIP Stack\r\n");

    // Initialize System memory and resources
    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

    // Initialize the CHIP stack.
    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    // Basic Fabric Table Init
    err = gOpCertStore.Init(&gStorage);
    SuccessOrExit(err);

    err = gOperationalKeystore.Init(&gStorage);
    SuccessOrExit(err);

    fabricTableInitParams.storage             = &gStorage;
    fabricTableInitParams.operationalKeystore = &gOperationalKeystore;
    fabricTableInitParams.opCertStore         = &gOpCertStore;

    err = gFabricTable.Init(fabricTableInitParams);
    SuccessOrExit(err);

    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
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

    gFabricTable.Shutdown();
    gOperationalKeystore.Finish();
    gOpCertStore.Finish();

    chip::DeviceLayer::PlatformMgr().Shutdown();
}

void ENFORCE_FORMAT(1, 2) TLVPrettyPrinter(const char * aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    vprintf(aFormat, args);

    va_end(args);
}
