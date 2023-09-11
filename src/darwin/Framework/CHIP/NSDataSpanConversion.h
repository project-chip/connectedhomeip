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
 * Utilities for converting between NSData and chip::Span.
 */

inline chip::ByteSpan AsByteSpan(NSData * data) { return chip::ByteSpan(static_cast<const uint8_t *>(data.bytes), data.length); }

inline NSData * AsData(chip::ByteSpan span) { return [NSData dataWithBytes:span.data() length:span.size()]; }

NS_ASSUME_NONNULL_END
