/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRErrorTestUtils : NSObject
+ (uint8_t)errorToZCLErrorCode:(NSError * _Nullable)error;
@end

NS_ASSUME_NONNULL_END
