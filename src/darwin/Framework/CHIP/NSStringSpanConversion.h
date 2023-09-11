/**
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#import <Foundation/Foundation.h>

#include <lib/support/Span.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Utilities for converting between NSString and chip::CharSpan.
 */

inline chip::CharSpan AsCharSpan(NSString * str)
{
    return chip::CharSpan([str UTF8String], [str lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
}

inline NSString * AsString(chip::CharSpan span)
{
    return [[NSString alloc] initWithBytes:span.data() length:span.size() encoding:NSUTF8StringEncoding];
}

NS_ASSUME_NONNULL_END
