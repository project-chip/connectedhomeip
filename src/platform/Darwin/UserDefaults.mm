/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "UserDefaults.h"
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

#import <Foundation/Foundation.h>

static NSString * const kBDXThrottleIntervalInMsecsUserDefaultKey = @"BDXThrottleIntervalForThreadDevicesInMSecs";

static NSString * const kBDXThreadFramesPerBlockDefaultKey = @"BDXThreadFramesPerBlock";

namespace chip {
namespace Platform {

    namespace {

        enum ZeroDefault {
            kAcceptZero,
            kDontAcceptZero,
        };

        // Returns a std::optional containing the value for the user default.
        // Returns std::nullopt in the following cases:
        //   - kDontAcceptZero is passed and no user default is found
        //   - a user default is found, but it can't fit into the requested
        //     integer type (for example it's either too big, or it's negative)
        //   - kDontAcceptZero is passed and a user default with value 0 is
        //     found
        template <typename T>
        static std::optional<T> GetUserDefault(NSString * key, ZeroDefault acceptZero)
        {
            NSUserDefaults * defaults = [NSUserDefaults standardUserDefaults];
            NSInteger value = [defaults integerForKey:key];
            if (CanCastTo<T>(value) && (acceptZero == kAcceptZero || value != 0)) {
                return std::make_optional(static_cast<T>(value));
            }
            return std::nullopt;
        }

    } // namespace

    std::optional<System::Clock::Milliseconds16> GetUserDefaultBDXThrottleIntervalForThread()
    {
        std::optional intervalInMsecs = GetUserDefault<uint16_t>(kBDXThrottleIntervalInMsecsUserDefaultKey, kDontAcceptZero);
        if (intervalInMsecs.has_value()) {
            ChipLogProgress(BDX, "Got a user default value for BDX Throttle Interval for Thread devices - %d msecs", intervalInMsecs.value());
            return std::make_optional(System::Clock::Milliseconds16(intervalInMsecs.value()));
        }

        // For now return NullOptional if value returned in bdxThrottleIntervalInMsecs is 0, since that either means the key was not found or value was zero.
        // Since 0 is not a feasible value for this interval for now, we will treat that as not being set.
        return std::nullopt;
    }

    std::optional<uint8_t> GetUserDefaultBDXThreadFramesPerBlock()
    {
        std::optional numberOfFrames = GetUserDefault<uint8_t>(kBDXThreadFramesPerBlockDefaultKey, kDontAcceptZero);
        if (numberOfFrames.has_value()) {
            ChipLogProgress(BDX, "Got a user default value for thread frames per block - %d", numberOfFrames.value());
        }
        return numberOfFrames;
    }

} // namespace Platform
} // namespace chip
