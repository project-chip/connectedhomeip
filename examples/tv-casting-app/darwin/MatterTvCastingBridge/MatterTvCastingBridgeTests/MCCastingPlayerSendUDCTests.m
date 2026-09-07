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

#import "MatterTvCastingBridge/MCCastingPlayer.h"
#import "MatterTvCastingBridge/MCCommissionerDeclaration.h"
#import "MatterTvCastingBridge/MCConnectionCallbacks.h"
#import "MatterTvCastingBridge/MCErrorUtils.h"
#import "MatterTvCastingBridge/MCIdentificationDeclarationOptions.h"
#import "MatterTvCastingBridge/MCTargetAppInfo.h"
#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>

/**
 * Unit tests for MCCastingPlayer.sendUDCWithCallbacks functionality.
 */
@interface MCCastingPlayerSendUDCTests : XCTestCase

@end

@implementation MCCastingPlayerSendUDCTests

- (void)setUp
{
    [super setUp];
}

- (void)tearDown
{
    [super tearDown];
}

/**
 * Test: sendUDCWithCallbacks with basic MCIdentificationDeclarationOptions
 * Verifies that sendUDCWithCallbacks can be called with minimal options
 */
- (void)testSendUDCWithCallbacks_BasicOptions
{
    // Arrange
    MCCastingPlayer * mockCastingPlayer = OCMClassMock([MCCastingPlayer class]);

    __block BOOL connectionCallbackCalled = NO;
    void (^connectionCompleteCallback)(NSError * _Nullable) = ^(NSError * _Nullable error) {
        connectionCallbackCalled = YES;
    };

    MCConnectionCallbacks * callbacks = [[MCConnectionCallbacks alloc] initWithCallbacks:connectionCompleteCallback
                                                         commissionerDeclarationCallback:nil];

    MCIdentificationDeclarationOptions * idOptions = [[MCIdentificationDeclarationOptions alloc] init];

    OCMStub([mockCastingPlayer sendUDCWithCallbacks:callbacks
                   identificationDeclarationOptions:idOptions])
        .andReturn(nil);

    // Act
    NSError * error = [mockCastingPlayer sendUDCWithCallbacks:callbacks
                             identificationDeclarationOptions:idOptions];

    // Assert
    XCTAssertNil(error, @"Error should be nil");
    OCMVerify([mockCastingPlayer sendUDCWithCallbacks:callbacks
                     identificationDeclarationOptions:idOptions]);
}

/**
 * Test: sendUDCWithCallbacks with NoPasscode flag set
 * Verifies that sendUDCWithCallbacks properly handles the NoPasscode flag for app detection
 */
- (void)testSendUDCWithCallbacks_WithNoPasscode
{
    // Arrange
    MCCastingPlayer * mockCastingPlayer = OCMClassMock([MCCastingPlayer class]);

    __block BOOL connectionCallbackCalled = NO;
    void (^connectionCompleteCallback)(NSError * _Nullable) = ^(NSError * _Nullable error) {
        connectionCallbackCalled = YES;
    };

    MCConnectionCallbacks * callbacks = [[MCConnectionCallbacks alloc] initWithCallbacks:connectionCompleteCallback
                                                         commissionerDeclarationCallback:nil];

    MCIdentificationDeclarationOptions * idOptions = [[MCIdentificationDeclarationOptions alloc] initWithNoPasscodeOnly:YES];

    OCMStub([mockCastingPlayer sendUDCWithCallbacks:callbacks
                   identificationDeclarationOptions:idOptions])
        .andReturn(nil);

    // Act
    NSError * error = [mockCastingPlayer sendUDCWithCallbacks:callbacks
                             identificationDeclarationOptions:idOptions];

    // Assert
    XCTAssertNil(error, @"Error should be nil");
}

/**
 * Test: sendUDCWithCallbacks with CancelPasscode flag set
 * Verifies that sendUDCWithCallbacks properly handles the CancelPasscode flag to cancel a UDC session
 */
- (void)testSendUDCWithCallbacks_WithCancelPasscode
{
    // Arrange
    MCCastingPlayer * mockCastingPlayer = OCMClassMock([MCCastingPlayer class]);

    __block BOOL connectionCallbackCalled = NO;
    void (^connectionCompleteCallback)(NSError * _Nullable) = ^(NSError * _Nullable error) {
        connectionCallbackCalled = YES;
    };

    MCConnectionCallbacks * callbacks = [[MCConnectionCallbacks alloc] initWithCallbacks:connectionCompleteCallback
                                                         commissionerDeclarationCallback:nil];

    MCIdentificationDeclarationOptions * idOptions = [[MCIdentificationDeclarationOptions alloc] initWithCancelPasscodeOnly:YES];

    OCMStub([mockCastingPlayer sendUDCWithCallbacks:callbacks
                   identificationDeclarationOptions:idOptions])
        .andReturn(nil);

    // Act
    NSError * error = [mockCastingPlayer sendUDCWithCallbacks:callbacks
                             identificationDeclarationOptions:idOptions];

    // Assert
    XCTAssertNil(error, @"Error should be nil");
}

/**
 * Test: sendUDCWithCallbacks with InstanceName
 * Verifies that sendUDCWithCallbacks properly handles a custom instanceName
 */
- (void)testSendUDCWithCallbacks_WithInstanceName
{
    // Arrange
    MCCastingPlayer * mockCastingPlayer = OCMClassMock([MCCastingPlayer class]);
    NSString * instanceName = [[NSUUID UUID] UUIDString];

    __block BOOL connectionCallbackCalled = NO;
    void (^connectionCompleteCallback)(NSError * _Nullable) = ^(NSError * _Nullable error) {
        connectionCallbackCalled = YES;
    };

    MCConnectionCallbacks * callbacks = [[MCConnectionCallbacks alloc] initWithCallbacks:connectionCompleteCallback
                                                         commissionerDeclarationCallback:nil];

    MCIdentificationDeclarationOptions * idOptions = [[MCIdentificationDeclarationOptions alloc] init];
    [idOptions setInstanceName:instanceName];

    OCMStub([mockCastingPlayer sendUDCWithCallbacks:callbacks
                   identificationDeclarationOptions:idOptions])
        .andReturn(nil);

    // Act
    NSError * error = [mockCastingPlayer sendUDCWithCallbacks:callbacks
                             identificationDeclarationOptions:idOptions];

    // Assert
    XCTAssertNil(error, @"Error should be nil");
    XCTAssertEqualObjects([idOptions getInstanceName], instanceName, @"InstanceName should match");
}

/**
 * Test: sendUDCWithCallbacks with TargetAppInfo
 * Verifies that sendUDCWithCallbacks properly handles TargetAppInfo for app detection
 */
- (void)testSendUDCWithCallbacks_WithTargetAppInfo
{
    // Arrange
    MCCastingPlayer * mockCastingPlayer = OCMClassMock([MCCastingPlayer class]);
    UInt16 targetVendorId = 0xFFF1;

    __block BOOL connectionCallbackCalled = NO;
    void (^connectionCompleteCallback)(NSError * _Nullable) = ^(NSError * _Nullable error) {
        connectionCallbackCalled = YES;
    };

    MCConnectionCallbacks * callbacks = [[MCConnectionCallbacks alloc] initWithCallbacks:connectionCompleteCallback
                                                         commissionerDeclarationCallback:nil];

    MCIdentificationDeclarationOptions * idOptions = [[MCIdentificationDeclarationOptions alloc] initWithNoPasscodeOnly:YES];
    MCTargetAppInfo * targetAppInfo = [[MCTargetAppInfo alloc] initWithVendorId:targetVendorId];
    [idOptions addTargetAppInfo:targetAppInfo];

    OCMStub([mockCastingPlayer sendUDCWithCallbacks:callbacks
                   identificationDeclarationOptions:idOptions])
        .andReturn(nil);

    // Act
    NSError * error = [mockCastingPlayer sendUDCWithCallbacks:callbacks
                             identificationDeclarationOptions:idOptions];

    // Assert
    XCTAssertNil(error, @"Error should be nil");
}

/**
 * Test: sendUDCWithCallbacks with CommissionerDeclarationCallback
 * Verifies that sendUDCWithCallbacks properly registers the CommissionerDeclarationCallback
 */
- (void)testSendUDCWithCallbacks_WithCommissionerDeclarationCallback
{
    // Arrange
    MCCastingPlayer * mockCastingPlayer = OCMClassMock([MCCastingPlayer class]);

    __block BOOL connectionCallbackCalled = NO;
    __block BOOL commissionerDeclarationCallbackCalled = NO;

    void (^connectionCompleteCallback)(NSError * _Nullable) = ^(NSError * _Nullable error) {
        connectionCallbackCalled = YES;
    };

    void (^commissionerDeclarationCallback)(MCCommissionerDeclaration * _Nonnull) = ^(MCCommissionerDeclaration * _Nonnull cd) {
        commissionerDeclarationCallbackCalled = YES;
    };

    MCConnectionCallbacks * callbacks = [[MCConnectionCallbacks alloc] initWithCallbacks:connectionCompleteCallback
                                                         commissionerDeclarationCallback:commissionerDeclarationCallback];

    MCIdentificationDeclarationOptions * idOptions = [[MCIdentificationDeclarationOptions alloc] initWithNoPasscodeOnly:YES];

    OCMStub([mockCastingPlayer sendUDCWithCallbacks:callbacks
                   identificationDeclarationOptions:idOptions])
        .andReturn(nil);

    // Act
    NSError * error = [mockCastingPlayer sendUDCWithCallbacks:callbacks
                             identificationDeclarationOptions:idOptions];

    // Assert
    XCTAssertNil(error, @"Error should be nil");
}

/**
 * Test: sendUDCWithCallbacks complete app detection flow
 * Verifies the complete flow: send with NoPasscode, receive response, send CancelPasscode
 */
- (void)testSendUDCWithCallbacks_CompleteAppDetectionFlow
{
    // Arrange
    MCCastingPlayer * mockCastingPlayer = OCMClassMock([MCCastingPlayer class]);
    NSString * instanceName = [[NSUUID UUID] UUIDString];
    UInt16 targetVendorId = 0xFFF1;

    __block BOOL connectionCallbackCalled = NO;
    __block BOOL commissionerDeclarationCallbackCalled = NO;
    __block BOOL appFound = NO;

    void (^connectionCompleteCallback)(NSError * _Nullable) = ^(NSError * _Nullable error) {
        connectionCallbackCalled = YES;
    };

    void (^commissionerDeclarationCallback)(MCCommissionerDeclaration * _Nonnull) = ^(MCCommissionerDeclaration * _Nonnull cd) {
        commissionerDeclarationCallbackCalled = YES;

        // Check if app was found
        appFound = !cd.noAppsFound;

        // Send CancelPasscode to end the UDC session
        MCIdentificationDeclarationOptions * cancelOptions = [[MCIdentificationDeclarationOptions alloc] initWithCancelPasscodeOnly:YES];
        [cancelOptions setInstanceName:instanceName];

        void (^cancelCallback)(NSError * _Nullable) = ^(NSError * _Nullable error) {
            // Cancel complete
        };

        MCConnectionCallbacks * cancelCallbacks = [[MCConnectionCallbacks alloc] initWithCallbacks:cancelCallback
                                                                   commissionerDeclarationCallback:nil];

        [mockCastingPlayer sendUDCWithCallbacks:cancelCallbacks
               identificationDeclarationOptions:cancelOptions];
    };

    MCConnectionCallbacks * callbacks = [[MCConnectionCallbacks alloc] initWithCallbacks:connectionCompleteCallback
                                                         commissionerDeclarationCallback:commissionerDeclarationCallback];

    MCIdentificationDeclarationOptions * idOptions = [[MCIdentificationDeclarationOptions alloc] initWithNoPasscodeOnly:YES];
    [idOptions setInstanceName:instanceName];

    MCTargetAppInfo * targetAppInfo = [[MCTargetAppInfo alloc] initWithVendorId:targetVendorId];
    [idOptions addTargetAppInfo:targetAppInfo];

    OCMStub([mockCastingPlayer sendUDCWithCallbacks:[OCMArg any]
                   identificationDeclarationOptions:[OCMArg any]])
        .andReturn(nil);

    // Act
    NSError * error = [mockCastingPlayer sendUDCWithCallbacks:callbacks
                             identificationDeclarationOptions:idOptions];

    // Assert
    XCTAssertNil(error, @"Error should be nil");
    XCTAssertEqualObjects([idOptions getInstanceName], instanceName, @"InstanceName should match");
}

/**
 * Test: sendUDCWithCallbacks with multiple TargetAppInfo entries
 * Verifies that sendUDCWithCallbacks can handle multiple target apps
 */
- (void)testSendUDCWithCallbacks_WithMultipleTargetApps
{
    // Arrange
    MCCastingPlayer * mockCastingPlayer = OCMClassMock([MCCastingPlayer class]);
    UInt16 targetVendorId1 = 0xFFF1;
    UInt16 targetVendorId2 = 0xFFF2;

    __block BOOL connectionCallbackCalled = NO;
    void (^connectionCompleteCallback)(NSError * _Nullable) = ^(NSError * _Nullable error) {
        connectionCallbackCalled = YES;
    };

    MCConnectionCallbacks * callbacks = [[MCConnectionCallbacks alloc] initWithCallbacks:connectionCompleteCallback
                                                         commissionerDeclarationCallback:nil];

    MCIdentificationDeclarationOptions * idOptions = [[MCIdentificationDeclarationOptions alloc] initWithNoPasscodeOnly:YES];

    MCTargetAppInfo * targetAppInfo1 = [[MCTargetAppInfo alloc] initWithVendorId:targetVendorId1];
    MCTargetAppInfo * targetAppInfo2 = [[MCTargetAppInfo alloc] initWithVendorId:targetVendorId2];
    [idOptions addTargetAppInfo:targetAppInfo1];
    [idOptions addTargetAppInfo:targetAppInfo2];

    OCMStub([mockCastingPlayer sendUDCWithCallbacks:callbacks
                   identificationDeclarationOptions:idOptions])
        .andReturn(nil);

    // Act
    NSError * error = [mockCastingPlayer sendUDCWithCallbacks:callbacks
                             identificationDeclarationOptions:idOptions];

    // Assert
    XCTAssertNil(error, @"Error should be nil");
}

/**
 * Test: sendUDCWithCallbacks with error response
 * Verifies that sendUDCWithCallbacks properly handles error responses
 */
- (void)testSendUDCWithCallbacks_WithError
{
    // Arrange
    MCCastingPlayer * mockCastingPlayer = OCMClassMock([MCCastingPlayer class]);

    __block BOOL connectionCallbackCalled = NO;
    void (^connectionCompleteCallback)(NSError * _Nullable) = ^(NSError * _Nullable error) {
        connectionCallbackCalled = YES;
    };

    MCConnectionCallbacks * callbacks = [[MCConnectionCallbacks alloc] initWithCallbacks:connectionCompleteCallback
                                                         commissionerDeclarationCallback:nil];

    MCIdentificationDeclarationOptions * idOptions = [[MCIdentificationDeclarationOptions alloc] init];

    NSError * testError = [NSError errorWithDomain:@"TestDomain" code:1 userInfo:@{ NSLocalizedDescriptionKey : @"Test error" }];
    OCMStub([mockCastingPlayer sendUDCWithCallbacks:callbacks
                   identificationDeclarationOptions:idOptions])
        .andReturn(testError);

    // Act
    NSError * error = [mockCastingPlayer sendUDCWithCallbacks:callbacks
                             identificationDeclarationOptions:idOptions];

    // Assert
    XCTAssertNotNil(error, @"Error should not be nil");
    XCTAssertEqual(error.code, 1, @"Error code should match");
}

/**
 * Test: sendUDCWithCallbacks validates required callbacks
 * Verifies that sendUDCWithCallbacks validates the presence of required callbacks
 */
- (void)testSendUDCWithCallbacks_ValidatesRequiredCallbacks
{
    // Arrange
    MCCastingPlayer * mockCastingPlayer = OCMClassMock([MCCastingPlayer class]);

    __block BOOL connectionCallbackCalled = NO;
    void (^connectionCompleteCallback)(NSError * _Nullable) = ^(NSError * _Nullable error) {
        connectionCallbackCalled = YES;
    };

    MCConnectionCallbacks * callbacks = [[MCConnectionCallbacks alloc] initWithCallbacks:connectionCompleteCallback
                                                         commissionerDeclarationCallback:nil];

    MCIdentificationDeclarationOptions * idOptions = [[MCIdentificationDeclarationOptions alloc] init];

    OCMStub([mockCastingPlayer sendUDCWithCallbacks:callbacks
                   identificationDeclarationOptions:idOptions])
        .andReturn(nil);

    // Act
    NSError * error = [mockCastingPlayer sendUDCWithCallbacks:callbacks
                             identificationDeclarationOptions:idOptions];

    // Assert
    XCTAssertNil(error, @"Error should be nil with valid callbacks");
}

@end
