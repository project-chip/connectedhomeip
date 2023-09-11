/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRLogging_Internal.h"

#import "MTRFramework.h"

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

    os_unfair_lock_lock(&logCallbackLock);
    if (callback) {
        SetLogRedirectCallback(&MTRLogCallbackTrampoline);
        SetLogFilter(static_cast<LogCategory>(std::min(std::max(logTypeThreshold, MTRLogTypeError), MTRLogTypeDetail)));
        logCallback = callback;
    } else {
        logCallback = nil;
        SetLogFilter(kLogCategory_None);
        SetLogRedirectCallback(nullptr);
    }
    os_unfair_lock_unlock(&logCallbackLock);
}
