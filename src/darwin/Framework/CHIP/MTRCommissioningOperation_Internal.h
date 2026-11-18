/**
 *    Copyright (c) 2025 Project CHIP Authors
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
#import <Matter/MTRCommissioningOperation.h>
#import <Matter/MTRDeviceControllerDelegate.h>
#import <Matter/MTRSetupPayload.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRCommissioningOperation () <MTRDeviceControllerDelegate>

/*
 * If isInternallyCreated is YES, that also means that commissioning metrics
 * initial setup has already happened.
 */
- (instancetype)initWithParameters:(MTRCommissioningParameters *)parameters
                      setupPayload:(NSString *)payload
                   commissioningID:(NSNumber *)commissioningID
               isInternallyCreated:(BOOL)isInternallyCreated
                          delegate:(id<MTRCommissioningDelegate>)delegate
                             queue:(dispatch_queue_t)queue;

@property (nonatomic, readonly, copy) NSString * setupPayload;

@property (nonatomic, readonly, assign) BOOL isInternallyCreated;

@property (nonatomic, readonly, copy) NSNumber * commissioningID;

// True if the commissioning is waiting to resume after PASE has been
// established and the delegate chose to be notified about that.
//
// This is currently only true if isInternallyCreated, and is readwrite because
// MTRDeviceController_Concrete helps maintain this state.
//
// Accessed from several queues, so the property is atomic:
// - set to YES on the commissioning queue, in the PASE-complete handler before it
//   dispatches anything further (MTRCommissioningOperation), so a release of the
//   commissionee queued behind it on the same serial queue always sees YES;
// - cleared on the Matter queue when commissioning resumes
//   (MTRDeviceController_Concrete commissionNodeWithID:...);
// - read and cleared on the commissioning queue when the commissionee is
//   released while waiting (commissioneeReleasedWhileWaitingAfterPASE:).
// Atomicity makes each access safe; the commissioning queue clears it only
// once, so a cancel racing with a release finishes the commissioning exactly
// once.
@property (atomic, readwrite, assign) BOOL isWaitingAfterPASEEstablished;

@end

NS_ASSUME_NONNULL_END
