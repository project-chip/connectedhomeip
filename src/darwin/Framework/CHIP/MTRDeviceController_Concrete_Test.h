/**
 *    Copyright (c) 2026 Project CHIP Authors
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

@class MTRCommissioningOperation;

NS_ASSUME_NONNULL_BEGIN

#ifdef DEBUG
// Commissioning test hooks. MTRDeviceController_Concrete adopts this protocol, so the compiler checks that the
// implementations match these declarations. Call them off the Matter queue, while the controller is running.
@protocol MTRDeviceControllerCommissioningTestHooks
- (void)unitTestExpirePASESessions;
// Delivers a release of the current commissioning's commissionee to the controller delegate bridge, as the commissioner
// would (OnCommissioneeReleased). Returns NO if there is no current commissioning.
- (BOOL)unitTestNotifyCommissioneePASESessionLost;
// Blocks the commissioning queue (up to 60 seconds) until the semaphore is signaled, so a test can control which work
// queued on it runs first.
- (void)unitTestHoldCommissioningQueueUntil:(dispatch_semaphore_t)semaphore;
// The current commissioning, and the call that ends it, as MTRCommissioningOperation uses them.
@property (atomic, readonly, nullable, weak) MTRCommissioningOperation * currentCommissioning;
- (void)commissioningDone:(MTRCommissioningOperation *)commissioning;
// Makes the commissioning current, as startWithController: does.
- (void)unitTestSetCurrentCommissioning:(MTRCommissioningOperation *)commissioning;
// Installs the commissioning as the delegate bridge's delegate on the Matter queue, as starting its PASE session does.
- (void)unitTestInstallCommissioningAsBridgeDelegate:(MTRCommissioningOperation *)commissioning;
// The delegate bridge's current delegate, read on the Matter queue.
- (nullable id)unitTestBridgeDelegate;
// Called by commissioningDone: after it clears the current commissioning and before it queues the delegate bridge
// reset. Nil by default.
@property (atomic, copy, nullable) dispatch_block_t unitTestCommissioningDoneBeforeDelegateReset;
@end
#endif

NS_ASSUME_NONNULL_END
