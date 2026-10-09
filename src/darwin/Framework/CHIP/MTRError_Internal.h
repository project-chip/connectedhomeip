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

#import "MTRDefines_Internal.h"
#import "MTRError_Test.h"

#include <app/MessageDef/StatusIB.h>
#include <clusters/NetworkCommissioning/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <protocols/interaction_model/StatusCode.h>

#include <string>

NS_ASSUME_NONNULL_BEGIN

MTR_DIRECT_MEMBERS
@interface MTRError ()
+ (NSError * _Nullable)errorForCHIPErrorCode:(CHIP_ERROR)errorCode;
+ (NSError * _Nullable)errorForCHIPErrorCode:(CHIP_ERROR)errorCode logContext:(id _Nullable)contextToLog;
/**
 * Same as errorForCHIPErrorCode:logContext:, but if networkCommissioningStatus
 * has a value, the resulting NSError's userInfo carries the byte under
 * MTRErrorNetworkCommissioningStatusKey.  Used by the commissioning bridge to
 * surface NetworkCommissioning cluster ConnectNetworkResponse status (e.g.
 * NetworkNotFound, AuthFailure) on otherwise-opaque commissioning failures.
 */
+ (NSError * _Nullable)errorForCHIPErrorCode:(CHIP_ERROR)errorCode
                                  logContext:(id _Nullable)contextToLog
                  networkCommissioningStatus:
                      (const chip::Optional<chip::app::Clusters::NetworkCommissioning::NetworkCommissioningStatusEnum> &)
                          networkCommissioningStatus;
/**
 * Same as the above, but also forwards the (optional) device-reported
 * driver-level connectNetworkErrorValue and (possibly-empty)
 * networkCommissioningDebugText through to userInfo under their respective
 * MTRErrorNetworkCommissioning* keys.  Empty / NullOptional values omit
 * the corresponding key so consumers can rely on key presence as the
 * "device actually reported a value" signal.
 */
+ (NSError * _Nullable)errorForCHIPErrorCode:(CHIP_ERROR)errorCode
                                  logContext:(id _Nullable)contextToLog
                  networkCommissioningStatus:
                      (const chip::Optional<chip::app::Clusters::NetworkCommissioning::NetworkCommissioningStatusEnum> &)
                          networkCommissioningStatus
                    connectNetworkErrorValue:(const chip::Optional<int32_t> &)connectNetworkErrorValue
               networkCommissioningDebugText:(const std::string &)networkCommissioningDebugText;
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
