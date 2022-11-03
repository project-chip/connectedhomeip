/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#include <app/MessageDef/StatusIB.h>
#include <lib/core/CHIPError.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRError : NSObject
+ (nullable NSError *)errorForCHIPErrorCode:(CHIP_ERROR)errorCode;
+ (nullable NSError *)errorForIMStatus:(const chip::app::StatusIB &)status;
+ (CHIP_ERROR)errorToCHIPErrorCode:(NSError * _Nullable)error;
@end

NS_ASSUME_NONNULL_END
