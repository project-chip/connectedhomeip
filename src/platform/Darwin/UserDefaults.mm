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

} // namespace Platform
} // namespace chip
