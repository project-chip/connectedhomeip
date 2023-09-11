/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Matter/MTRDefines.h>
#import <Matter/MTRError.h>

#include <app/MessageDef/StatusIB.h>
#include <lib/core/CHIPError.h>

NS_ASSUME_NONNULL_BEGIN

MTR_HIDDEN
@interface MTRError : NSObject
+ (NSError * _Nullable)errorForCHIPErrorCode:(CHIP_ERROR)errorCode;
+ (NSError * _Nullable)errorForIMStatus:(const chip::app::StatusIB &)status;
+ (CHIP_ERROR)errorToCHIPErrorCode:(NSError * _Nullable)error;
@end

NS_ASSUME_NONNULL_END
