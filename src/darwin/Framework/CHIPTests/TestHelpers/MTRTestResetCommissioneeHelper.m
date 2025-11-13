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

#import "MTRTestResetCommissioneeHelper.h"

void ResetCommissioneeWithNodeID(MTRBaseDevice * device, dispatch_queue_t queue, XCTestCase * testcaseUnused, uint16_t commandTimeout, NSNumber * _Nullable nodeID)
{
    // Put the device back in the state we found it: open commissioning window, no fabrics commissioned.
    // Get our current fabric index, for later deletion.
    XCTestExpectation * readFabricIndexExpectation = [[XCTestExpectation alloc] initWithDescription:@"Fabric index read"];

    __block NSNumber * fabricIndex;
    __auto_type * opCredsCluster = [[MTRBaseClusterOperationalCredentials alloc] initWithDevice:device endpoint:0 queue:queue];
    [opCredsCluster
        readAttributeCurrentFabricIndexWithCompletionHandler:^(NSNumber * _Nullable value, NSError * _Nullable readError) {
            XCTAssertNil(readError);
            XCTAssertNotNil(value);
            fabricIndex = value;
            [readFabricIndexExpectation fulfill];
        }];

    XCTAssertEqual([XCTWaiter waitForExpectations:@[ readFabricIndexExpectation ] timeout:commandTimeout], XCTWaiterResultCompleted);

    // Open a commissioning window.
    XCTestExpectation * openCommissioningWindowExpectation = [[XCTestExpectation alloc] initWithDescription:@"Commissioning window opened"];
    __auto_type * adminCommissioningCluster = [[MTRBaseClusterAdministratorCommissioning alloc] initWithDevice:device
                                                                                                      endpoint:0
                                                                                                         queue:queue];
    __auto_type * openWindowParams = [[MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams alloc] init];
    openWindowParams.commissioningTimeout = @(900);
    openWindowParams.timedInvokeTimeoutMs = @(50000);
    [adminCommissioningCluster openBasicCommissioningWindowWithParams:openWindowParams
                                                    completionHandler:^(NSError * _Nullable error) {
                                                        XCTAssertNil(error);
                                                        [openCommissioningWindowExpectation fulfill];
                                                    }];

    XCTAssertEqual([XCTWaiter waitForExpectations:@[ openCommissioningWindowExpectation ] timeout:commandTimeout], XCTWaiterResultCompleted);

    // Remove our fabric from the device.
    XCTestExpectation * removeFabricExpectation = [[XCTestExpectation alloc] initWithDescription:@"Fabric removed"];

    __auto_type * removeParams = [[MTROperationalCredentialsClusterRemoveFabricParams alloc] init];
    removeParams.fabricIndex = fabricIndex;

    [opCredsCluster removeFabricWithParams:removeParams
                         completionHandler:^(
                             MTROperationalCredentialsClusterNOCResponseParams * _Nullable data, NSError * _Nullable removeError) {
                             XCTAssertNil(removeError);
                             XCTAssertNotNil(data);
                             XCTAssertEqualObjects(data.statusCode, @(0));
                             [removeFabricExpectation fulfill];
                         }];

    XCTAssertEqual([XCTWaiter waitForExpectations:@[ removeFabricExpectation ] timeout:commandTimeout], XCTWaiterResultCompleted,
        "Resetting commissionnee 0x%llx", nodeID.unsignedLongLongValue);
}

void ResetCommissionee(MTRBaseDevice * device, dispatch_queue_t queue, XCTestCase * testcaseUnused, uint16_t commandTimeout)
{
    return ResetCommissioneeWithNodeID(device, queue, testcaseUnused, commandTimeout, nil);
}
