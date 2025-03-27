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

static NSString * const kSRPTimeoutInMsecsUserDefaultKey = @"SRPTimeoutInMSecsOverride";

static NSString * const kBDXThrottleIntervalInMsecsUserDefaultKey = @"BDXThrottleIntervalForThreadDevicesInMSecs";

namespace chip {
namespace Platform {

    std::optional<uint16_t> GetUserDefaultDnssdSRPTimeoutInMSecs()
    {
        NSUserDefaults * defaults = [NSUserDefaults standardUserDefaults];
        NSInteger srpTimeoutValue = [defaults integerForKey:kSRPTimeoutInMsecsUserDefaultKey];
        if (CanCastTo<uint16_t>(srpTimeoutValue)) {
            uint16_t timeoutinMsecs = static_cast<uint16_t>(srpTimeoutValue);
            ChipLogProgress(Discovery, "Got a user default value for Dnssd SRP timeout - %d msecs", timeoutinMsecs);
            return std::make_optional(timeoutinMsecs);
        }
        return std::nullopt;
    }

    std::optional<System::Clock::Milliseconds16> GetUserDefaultBDXThrottleIntervalForThread()
    {
        NSUserDefaults * defaults = [NSUserDefaults standardUserDefaults];
        NSInteger bdxThrottleIntervalInMsecs = [defaults integerForKey:kBDXThrottleIntervalInMsecsUserDefaultKey];

        if (bdxThrottleIntervalInMsecs > 0 && CanCastTo<uint16_t>(bdxThrottleIntervalInMsecs)) {
            uint16_t intervalInMsecs = static_cast<uint16_t>(bdxThrottleIntervalInMsecs);
            ChipLogProgress(BDX, "Got a user default value for BDX Throttle Interval for Thread devices - %d msecs", intervalInMsecs);
            return std::make_optional(System::Clock::Milliseconds16(intervalInMsecs));
        }

        // For now return NullOptional if value returned in bdxThrottleIntervalInMsecs is 0, since that either means the key was not found or value was zero.
        // Since 0 is not a feasible value for this interval for now, we will treat that as not being set.
        return std::nullopt;
    }

} // namespace Platform
} // namespace chip
