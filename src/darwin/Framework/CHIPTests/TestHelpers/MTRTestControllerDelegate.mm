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

#import "MTRTestControllerDelegate.h"

@implementation MTRTestControllerDelegate
- (id)initWithExpectation:(XCTestExpectation *)expectation newNodeID:(NSNumber *)newNodeID
{
    self = [super init];
    if (self) {
        _expectation = expectation;
        _deviceID = newNodeID;
        _countryCode = nil;
    }
    return self;
}

- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError *)error
{
    XCTAssertEqual(error.code, 0);

    NSError * getDeviceError = nil;
    __auto_type * device = [controller deviceBeingCommissionedWithNodeID:self.deviceID error:&getDeviceError];
    XCTAssertNil(getDeviceError);
    XCTAssertNotNil(device);

    // Now check that getting with some other random id fails.
    device = [controller deviceBeingCommissionedWithNodeID:@(self.deviceID.unsignedLongLongValue + 1) error:&getDeviceError];
    XCTAssertNil(device);
    XCTAssertNotNil(getDeviceError);

    __auto_type * params = [[MTRCommissioningParameters alloc] init];
    params.countryCode = self.countryCode;

    NSError * commissionError = nil;
    [controller commissionNodeWithID:self.deviceID commissioningParams:params error:&commissionError];
    XCTAssertNil(commissionError);

    // Keep waiting for controller:commissioningComplete:
}

- (void)controller:(MTRDeviceController *)controller commissioningComplete:(NSError *)error
{
    XCTAssertEqual(error.code, 0);
    [_expectation fulfill];
    _expectation = nil;
}

@end
