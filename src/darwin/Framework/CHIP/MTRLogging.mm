/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#import "MTRLogging_Internal.h"

#import "MTRFramework.h"
#import "MTRUnfairLock.h"

#import <algorithm>
#import <atomic>
#import <os/lock.h>

using namespace chip::Logging;

static_assert(MTRLogTypeError == (NSInteger) kLogCategory_Error, "MTRLogType* != kLogCategory_*");
static_assert(MTRLogTypeProgress == (NSInteger) kLogCategory_Progress, "MTRLogType* != kLogCategory_*");
static_assert(MTRLogTypeDetail == (NSInteger) kLogCategory_Detail, "MTRLogType* != kLogCategory_*");

static os_unfair_lock logCallbackLock = OS_UNFAIR_LOCK_INIT;
static MTRLogCallback logCallback = nil;

static void MTRLogCallbackTrampoline(const char * moduleName, uint8_t category, const char * format, va_list args)
{
    os_unfair_lock_lock(&logCallbackLock);
    MTRLogCallback callback = logCallback;
    os_unfair_lock_unlock(&logCallbackLock);
    if (!callback) {
        return;
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
    // Note: Format using NSString so that '%@' placeholders are supported
    NSString * message = [[NSString alloc] initWithFormat:@(format) arguments:args];
#pragma clang diagnostic pop

    auto type = std::min(static_cast<MTRLogType>(category), MTRLogTypeDetail); // hide kLogCategory_Automation
    callback(type, @(moduleName), message);
}

void MTRSetLogCallback(MTRLogType logTypeThreshold, MTRLogCallback _Nullable callback)
{
    MTRFrameworkInit();

    std::lock_guard lock(logCallbackLock);
    if (callback) {
        SetLogRedirectCallback(&MTRLogCallbackTrampoline);
        SetLogFilter(static_cast<LogCategory>(std::min(std::max(logTypeThreshold, MTRLogTypeError), MTRLogTypeDetail)));
        logCallback = callback;
    } else {
        logCallback = nil;
        SetLogFilter(kLogCategory_None);
        SetLogRedirectCallback(nullptr);
    }
}
