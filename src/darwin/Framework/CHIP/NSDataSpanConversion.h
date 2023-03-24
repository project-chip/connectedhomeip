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
 * Utilities for converting between NSData and chip::Span.
 */

inline chip::ByteSpan AsByteSpan(NSData * data) { return chip::ByteSpan(static_cast<const uint8_t *>(data.bytes), data.length); }

inline NSData * AsData(chip::ByteSpan span) { return [NSData dataWithBytes:span.data() length:span.size()]; }

NS_ASSUME_NONNULL_END
