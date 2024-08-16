/**
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

#import "MTRDeviceController_XPC.h"
#import "MTRDeviceController_XPC_Internal.h"
#import "MTRXPCService.h"
#import "MTRXPCServiceProtocol.h"
#import "MTRXPCServiceTestsDummyService.h"
#import <Matter/Matter.h>
#import <XCTest/XCTest.h>

@interface MTRXPCServiceTests<NSXPCListenerDelegate> : XCTestCase

@property (nonatomic, readwrite, strong) NSXPCListener * xpcListener;
@property (nonatomic, readwrite, strong) NSXPCInterface * serviceInterface;
@property (nonatomic, readwrite, strong) NSXPCInterface * clientInterface;
@property (readwrite, strong) NSXPCConnection * xpcConnection;

@property (nonatomic, readwrite, strong) MTRXPCServiceTestsDummyService * dummyService;

@end

@implementation MTRXPCServiceTests

// NSXPCListenerDelegate
- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    NSLog(@"%s: XPC listener accepting connection", __PRETTY_FUNCTION__);

    XCTAssertNil(_xpcConnection);
    XCTAssertNotNil(newConnection);
    XCTAssertNotNil(_dummyService);

    newConnection.exportedInterface = _serviceInterface;
    newConnection.exportedObject = _dummyService;
    //    newConnection.remoteObjectInterface = _serviceInterface;
    newConnection.invalidationHandler = ^{
        dispatch_async(dispatch_get_main_queue(), ^{
            NSLog(@"%s: XPC connection disconnected", __PRETTY_FUNCTION__);
            self.xpcConnection = nil;
            //            if (self.xpcDisconnectExpectation) {
            //                [self.xpcDisconnectExpectation fulfill];
            //                self.xpcDisconnectExpectation = nil;
            //            }
        });
    };
    dispatch_async(dispatch_get_main_queue(), ^{
        self.xpcConnection = newConnection;
        [newConnection resume];
    });
    return YES;
}

// end NSXPCListenerDelegate

- (void)setUp
{
    _dummyService = [[MTRXPCServiceTestsDummyService alloc] init];
    _xpcListener = [NSXPCListener anonymousListener];
    [_xpcListener setDelegate:(id<NSXPCListenerDelegate>) self];
    _serviceInterface = [MTRXPCService xpcInterfaceForServiceServerProtocol];
    [_xpcListener resume];
    XCTAssertNotNil(_xpcListener);
    NSLog(@"%s: done", __PRETTY_FUNCTION__);
}

- (void)tearDown
{
    _xpcListener.delegate = nil;
    _xpcListener = nil;
    _serviceInterface = nil;
    _dummyService = nil;
}

- (void)testExample
{
    NSLog(@"%s", __PRETTY_FUNCTION__);

    XCTAssertNotNil(_xpcListener);
    XCTAssertNotNil(_xpcListener.endpoint);
    NSXPCConnection * serviceConnection = [[NSXPCConnection alloc] initWithListenerEndpoint:_xpcListener.endpoint];
    serviceConnection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCServiceProtocol)];
    id<MTRXPCServiceProtocol> obj = serviceConnection.remoteObjectProxy;

    [serviceConnection resume];

    XCTAssertNotNil(obj);

    [obj ping];

    //    NSNumber * lifeMeaning = [obj synchronouslyGetMeaningOfLife];
    //
    //    NSLog(@"test got lifeMeaning = %@", lifeMeaning);

    XCTestExpectation * expectation = [[XCTestExpectation alloc] initWithDescription:@"should get meaning of life"];
    __block NSNumber * asyncLifeMeaning;
    [obj getMeaningOfLifeWithReplyBlock:^(int reply) {
        asyncLifeMeaning = @(reply);
        NSLog(@"got async life meaning %@", asyncLifeMeaning);
        [expectation fulfill];
    }];

    [self waitForExpectations:@[ expectation ] timeout:1.0];
    NSLog(@"done with %s", __PRETTY_FUNCTION__);
}

@end
