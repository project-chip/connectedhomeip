/**
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRFramework.h"

#include <dispatch/dispatch.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

void MTRFrameworkInit()
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // Ensure Matter Platform::Memory is initialized.  This only needs
        // to happen once per process, because in practice we just use
        // malloc/free so there is nothing to initialize, so this just needs
        // to happen to avoid debug assertions.  The malloc version of
        // MemoryInit() never fails.
        chip::Platform::MemoryInit();

        // Suppress CHIP logging until we actually need it for redirection
        // (see MTRSetLogCallback()). Logging to os_log is always enabled.
        chip::Logging::SetLogFilter(chip::Logging::kLogCategory_None);
    });
}
