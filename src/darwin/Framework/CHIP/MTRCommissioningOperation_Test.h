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
#import <Matter/MTRCommissioningOperation.h>

NS_ASSUME_NONNULL_BEGIN

// Production interval after which the post-PASE watchdog fires.  Exposed so
// tests can assert against the same constant the production code uses.
extern const NSTimeInterval kMTRPostPASEWatchdogInterval;

@interface MTRCommissioningOperation (PostPASEWatchdogTesting)

// Overrides the post-PASE watchdog interval for all subsequently armed
// watchdogs in the current process.  Pass 0 to restore the production
// interval (kMTRPostPASEWatchdogInterval).  Intended only for tests that
// need to exercise the watchdog without waiting five minutes.
+ (void)setPostPASEWatchdogIntervalForTesting:(NSTimeInterval)interval;

// One-shot fault injection: if set to YES, the next call to the internal
// _armPostPASEWatchdog helper on this instance will simulate a
// dispatch_source_create failure and return NO without arming the timer.
// The flag is cleared as soon as it is observed, so each call to this
// setter injects exactly one failure.
- (void)setForceNextArmFailureForTesting:(BOOL)force;

// Test-only accessor for the _postPASEWatchdog dispatch_source ivar.
// Returns the source under _stateLock so tests can verify synchronous
// cancel semantics after -stop: returns (the cancel in -stop is now
// synchronous; tests can read this immediately after -stop returns and
// expect nil).  Do NOT use this to drive production logic; this exists
// purely so test_WatchdogCancel_IsSynchronousOnStop can pin the
// synchronous-cancel contract.
- (nullable dispatch_source_t)postPASEWatchdogForTesting;

@end

NS_ASSUME_NONNULL_END
