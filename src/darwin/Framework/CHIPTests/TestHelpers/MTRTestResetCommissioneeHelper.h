/**
 *    Copyright (c) 2023 Project CHIP Authors
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
#import <Matter/Matter.h>
#import <XCTest/XCTest.h>

#pragma once

NS_ASSUME_NONNULL_BEGIN

void ResetCommissionee(MTRBaseDevice * device, dispatch_queue_t queue, XCTestCase * _Nullable testcase, uint16_t commandTimeout);

// MTRBaseDevice does not expose its node ID, so for now just allow callers to
// pass that in out-of-band.
void ResetCommissioneeWithNodeID(MTRBaseDevice * device, dispatch_queue_t queue, XCTestCase * _Nullable testcase, uint16_t commandTimeout, NSNumber * _Nullable nodeID);

NS_ASSUME_NONNULL_END
