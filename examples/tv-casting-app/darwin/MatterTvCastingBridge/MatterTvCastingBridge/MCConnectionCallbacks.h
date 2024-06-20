/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#import "MCCommissionerDeclaration.h"

#import <Foundation/Foundation.h>

#ifndef MCConnectionCallbacks_h
#define MCConnectionCallbacks_h

/** @brief A container class for User Directed Commissioning (UDC) callbacks. */
@interface MCConnectionCallbacks : NSObject

/**
 * @param connectionCompleteCallback (Required) The callback called when the connection process
 * has ended, regardless of whether it was successful or not.
 * @param commissionerDeclarationCallback (Optional) The callback called when the Client/Commissionee
 * receives a CommissionerDeclaration message from the CastingPlayer/Commissioner. This callback is
 * needed to support UDC features where a reply from the Commissioner is expected. It provides information
 * indicating the Commissioner’s pre-commissioning state.
 *
 * For example: During CastingPlayer/Commissioner-Generated passcode commissioning, the Commissioner
 * replies with a CommissionerDeclaration message with PasscodeDialogDisplayed and CommissionerPasscode
 * set to true. Given these Commissioner state details, the client is expected to perform some actions
 * and responf accrdingly.
 *
 * @return A new instance of MCConnectionCallbacks.
 */
- (instancetype _Nonnull)initWithCallbacks:(void (^_Nonnull)(NSError * _Nonnull))connectionCompleteCallback
           commissionerDeclarationCallback:(void (^_Nullable)(MCCommissionerDeclaration * _Nonnull))commissionerDeclarationCallback;

/**
 * The callback called when the connection process has ended, regardless of whether it was
 * successful or not.
 */
@property void (^_Nullable connectionCompleteCallback)(NSError * _Nonnull);

/**
 * The callback called when the Client/Commissionee receives a CommissionerDeclaration
 * message from the CastingPlayer/Commissioner. This callback is needed to support UDC features
 * where a reply from the Commissioner is expected. It provides information indicating the
 * Commissioner’s pre-commissioning state.
 */
@property void (^_Nullable commissionerDeclarationCallback)(MCCommissionerDeclaration * _Nonnull);

@end

#endif /* MCConnectionCallbacks_h */
