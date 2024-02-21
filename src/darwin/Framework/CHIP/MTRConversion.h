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

#include <lib/core/CASEAuthTag.h>
#include <lib/core/CHIPError.h>
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

inline NSDate * MatterEpochSecondsAsDate(uint32_t matterEpochSeconds)
{
    return [NSDate dateWithTimeIntervalSince1970:(chip::kChipEpochSecondsSinceUnixEpoch + (NSTimeInterval) matterEpochSeconds)];
}

/**
 * Returns whether the conversion could be performed.  Will return false if the
 * passed-in date is our of the range representable as a Matter epoch-s value.
 */
bool DateToMatterEpochSeconds(NSDate * date, uint32_t & epoch);

/**
 * Returns whether the conversion could be performed.  Will return false if the
 * passed-in date is our of the range representable as a Matter epoch-s value.
 */
bool DateToMatterEpochMilliseconds(NSDate * date, uint64_t & matterEpochMilliseconds);

/**
 * Returns whether the conversion could be performed.  Will return false if the
 * passed-in date is our of the range representable as a Matter epoch-s value.
 */
bool DateToMatterEpochMicroseconds(NSDate * date, uint64_t & matterEpochMicroseconds);

/**
 * Utilities for converting between NSSet<NSNumber *> and chip::CATValues.
 */
CHIP_ERROR SetToCATValues(NSSet<NSNumber *> * catSet, chip::CATValues & values);
NSSet<NSNumber *> * CATValuesToSet(const chip::CATValues & values);

NS_ASSUME_NONNULL_END
