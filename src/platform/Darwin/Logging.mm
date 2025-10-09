/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

#include "Logging.h"

#include <lib/support/Span.h>

#import <Foundation/Foundation.h>

namespace chip {
namespace Logging {
    namespace Platform {

        // GetModuleName() returns 3 character acronyms that are not very readable, define a
        // separate array containing the full module names (replacing "NotSpecified" with "Default").
        constexpr char const * gLoggerNames[kLogModule_Max] = {
#define _CHIP_LOGMODULE_FULLNAME(MOD, ...) (kLogModule_##MOD == kLogModule_NotSpecified ? "Default" : #MOD),
            CHIP_LOGMODULES_ENUMERATE(_CHIP_LOGMODULE_FULLNAME)
        };

        struct CachedLogger {
            dispatch_once_t once = 0;
            void * /* os_log_t */ handle = nullptr; // void * to avoid a destructor that requires a static initializer
        };
        CachedLogger gLoggers[kLogModule_Max] {};

        void CreateLogger(void * context)
        {
            auto entry = static_cast<CachedLogger *>(context);
            auto moduleId = entry - gLoggers;
            entry->handle = (__bridge_retained void *) os_log_create("com.csa.matter", gLoggerNames[moduleId]);
        }

        void * LoggerForModule(LogModule moduleID)
        {
            if (moduleID < kLogModule_NotSpecified || kLogModule_Max <= moduleID) {
                moduleID = kLogModule_NotSpecified;
            }
            auto entry = &gLoggers[moduleID];
            dispatch_once_f(&entry->once, entry, CreateLogger);
            return entry->handle;
        }

        void LogByteSpan(LogModule moduleId, os_log_type_t type, const chip::ByteSpan & span)
        {
            auto logger = LoggerForModule(moduleId);
            if (ChipPlatformLogEnabled(logger, type)) {
                auto size = span.size();
                auto data = span.data();
                NSMutableString * string = [[NSMutableString alloc] initWithCapacity:(size * 6)]; // 6 characters per byte
                for (size_t i = 0; i < size; i++) {
                    [string appendFormat:((i % 8 != 7) ? @"0x%02x, " : @"0x%02x,\n"), data[i]];
                }
                ChipPlatformLogImpl(logger, type, "%@", string);
            }
        }

    } // namespace Platform
} // namespace Logging
} // namespace chip
