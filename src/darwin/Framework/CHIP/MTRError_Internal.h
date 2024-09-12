/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#import <Foundation/Foundation.h>
#import <Matter/MTRError.h>

#import "MTRDefines_Internal.h"

#include <app/MessageDef/StatusIB.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

NS_ASSUME_NONNULL_BEGIN

MTR_DIRECT_MEMBERS
@interface MTRError : NSObject
+ (NSError *)errorWithCode:(MTRErrorCode)code;
+ (NSError * _Nullable)errorForCHIPErrorCode:(CHIP_ERROR)errorCode;
+ (NSError * _Nullable)errorForCHIPErrorCode:(CHIP_ERROR)errorCode logContext:(id _Nullable)contextToLog;
+ (NSError * _Nullable)errorForIMStatus:(const chip::app::StatusIB &)status;
+ (NSError * _Nullable)errorForIMStatusCode:(chip::Protocols::InteractionModel::Status)status;
+ (CHIP_ERROR)errorToCHIPErrorCode:(NSError * _Nullable)error;
@end

// Similar to VerifyOrDie, but throws an NSInvalidArgumentException
#define MTRVerifyArgumentOrDie(cond, reason) \
    do {                                     \
        if (mtr_unlikely(!(cond))) {         \
            MTRThrowInvalidArgument(reason); \
        }                                    \
    } while (0)

MTR_EXTERN _Noreturn void MTRThrowInvalidArgument(NSString * reason);

NS_ASSUME_NONNULL_END
