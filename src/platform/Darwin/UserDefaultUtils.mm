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

/**
 *    @file
 *          Implementation for an utility to set configurable parametes via user defaults.
 */
#import "UserDefaultUtils.h"
#import <Foundation/Foundation.h>

static NSString * const kUserDefaultDomain = @"org.csa-iot.matter.darwindefaults";
static NSString * const kSRPTimeoutUserDefaultKey = @"SRPTimeoutOverride";

namespace chip {
namespace DeviceLayer {
    namespace Utils {

        uint16_t getUserDefaultDnssdSRPTimeout()
        {
            NSUserDefaults * defaults = [[NSUserDefaults alloc] initWithSuiteName:kUserDefaultDomain];
            return static_cast<uint16_t>([defaults integerForKey:kSRPTimeoutUserDefaultKey]);
        }

    } // namespace Utils
} // namespace DeviceLayer
} // namespace chip
