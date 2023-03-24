/**
 *    Copyright (c) 2023 Project CHIP Authors
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

#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#import <Foundation/Foundation.h>

#include <lib/core/Optional.h>
#include <lib/support/TimeUtils.h>
#include <type_traits>

NS_ASSUME_NONNULL_BEGIN

template <typename T>
inline std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value || std::is_enum<T>::value,
    NSNumber * _Nullable>
AsNumber(chip::Optional<T> optional)
{
    return (optional.HasValue()) ? @(optional.Value()) : nil;
}

inline NSDate * ChipEpochSecondsAsDate(uint32_t chipEpochSeconds)
{
    return [NSDate dateWithTimeIntervalSince1970:(chip::kChipEpochSecondsSinceUnixEpoch + (NSTimeInterval) chipEpochSeconds)];
}

NS_ASSUME_NONNULL_END
