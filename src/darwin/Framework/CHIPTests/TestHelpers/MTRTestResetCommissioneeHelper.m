/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRTestResetCommissioneeHelper.h"

void ResetCommissionee(MTRBaseDevice * device, dispatch_queue_t queue, XCTestCase * testcase, uint16_t commandTimeout)
{
    // Put the device back in the state we found it: open commissioning window, no fabrics commissioned.
    // Get our current fabric index, for later deletion.
    XCTestExpectation * readFabricIndexExpectation = [testcase expectationWithDescription:@"Fabric index read"];

    __block NSNumber * fabricIndex;
    __auto_type * opCredsCluster = [[MTRBaseClusterOperationalCredentials alloc] initWithDevice:device endpoint:0 queue:queue];
    [opCredsCluster
        readAttributeCurrentFabricIndexWithCompletionHandler:^(NSNumber * _Nullable value, NSError * _Nullable readError) {
            XCTAssertNil(readError);
            XCTAssertNotNil(value);
            fabricIndex = value;
            [readFabricIndexExpectation fulfill];
        }];

    [testcase waitForExpectations:@[ readFabricIndexExpectation ] timeout:commandTimeout];

    // Open a commissioning window.
    XCTestExpectation * openCommissioningWindowExpectation = [testcase expectationWithDescription:@"Commissioning window opened"];

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

    [testcase waitForExpectations:@[ openCommissioningWindowExpectation ] timeout:commandTimeout];

    // Remove our fabric from the device.
    XCTestExpectation * removeFabricExpectation = [testcase expectationWithDescription:@"Fabric removed"];

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

    [testcase waitForExpectations:@[ removeFabricExpectation ] timeout:commandTimeout];
}
