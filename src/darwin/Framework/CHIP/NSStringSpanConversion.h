/**
 *    Copyright (c) 2022 Project CHIP Authors
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
