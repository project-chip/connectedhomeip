/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

#include "Logging.h"

#include <lib/support/Span.h>

#import <Foundation/Foundation.h>
#import <dispatch/dispatch.h>

namespace chip {
namespace Logging {
    namespace Platform {

        os_log_t LoggerForModule(chip::Logging::LogModule moduleID, char const * moduleName)
        {
            if (moduleID <= kLogModule_NotSpecified || kLogModule_Max <= moduleID) {
                moduleID = kLogModule_NotSpecified;
                moduleName = "Default";
            }

            static struct {
                dispatch_once_t onceToken;
                os_log_t logger;
            } cache[kLogModule_Max];
            auto & entry = cache[moduleID];
            dispatch_once(&entry.onceToken, ^{
                entry.logger = os_log_create("com.csa.matter", moduleName);
            });
            return entry.logger;
        }

        void LogByteSpan(
            chip::Logging::LogModule moduleId, char const * moduleName, os_log_type_t type, const chip::ByteSpan & span)
        {
            os_log_t logger = LoggerForModule(moduleId, moduleName);
            if (os_log_type_enabled(logger, type)) {
                auto size = span.size();
                auto data = span.data();
                NSMutableString * string = [[NSMutableString alloc] initWithCapacity:(size * 6)]; // 6 characters per byte
                for (size_t i = 0; i < size; i++) {
                    [string appendFormat:((i % 8 != 7) ? @"0x%02x, " : @"0x%02x,\n"), data[i]];
                }
                os_log_with_type(logger, type, "%@", string);
            }
        }

    } // namespace Platform
} // namespace Logging
} // namespace chip
