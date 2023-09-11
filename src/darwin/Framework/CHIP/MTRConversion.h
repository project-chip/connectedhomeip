/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

inline NSDate * ChipEpochSecondsAsDate(uint32_t chipEpochSeconds)
{
    return [NSDate dateWithTimeIntervalSince1970:(chip::kChipEpochSecondsSinceUnixEpoch + (NSTimeInterval) chipEpochSeconds)];
}

/**
 * Utilities for converting between NSSet<NSNumber *> and chip::CATValues.
 */
CHIP_ERROR SetToCATValues(NSSet<NSNumber *> * catSet, chip::CATValues & values);
NSSet<NSNumber *> * CATValuesToSet(const chip::CATValues & values);

NS_ASSUME_NONNULL_END
