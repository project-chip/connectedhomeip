/*
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

// module headers
#import <Matter/Matter.h>

#import "MTRTestCase+ServerAppRunner.h"
#import "MTRTestCase.h"
#import "MTRTestDeclarations.h"

static const uint64_t kDeviceId1 = 0x12344321;
static const uint16_t kTimeoutInSeconds = 3;

@interface MTRLargeMessageCommandTests : MTRTestCase

@end

@implementation MTRLargeMessageCommandTests

- (void)test001_ClientCSR
{
    MTRDeviceController * controller = [self createControllerOnTestFabric];
    XCTAssertNotNil(controller);

    dispatch_queue_t queue = dispatch_get_main_queue();

    [self startCommissionedAppWithName:@"all-clusters"
                             arguments:@[]
                            controller:controller
                                nodeID:@(kDeviceId1)];

    __auto_type * device = [MTRDevice deviceWithNodeID:@(kDeviceId1) controller:controller];

    __auto_type * cluster = [[MTRClusterTLSCertificateManagement alloc] initWithDevice:device endpointID:@(1) queue:queue];

    __auto_type * params = [[MTRTLSCertificateManagementClusterClientCSRParams alloc] init];
    params.nonce = [@"01234567890123456789012345678912" dataUsingEncoding:NSUTF8StringEncoding];

    XCTestExpectation * csrExpectation = [self expectationWithDescription:@"CSR request complete"];

    [cluster clientCSRWithParams:params expectedValues:nil expectedValueInterval:nil completion:^(MTRTLSCertificateManagementClusterClientCSRResponseParams * _Nullable data, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(data);

        [csrExpectation fulfill];
    }];

    [self waitForExpectations:@[ csrExpectation ] timeout:kTimeoutInSeconds];
}

@end
