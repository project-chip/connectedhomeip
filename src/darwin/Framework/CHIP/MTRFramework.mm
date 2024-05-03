/**
 *    Copyright (c) 2022 Project CHIP Authors
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
