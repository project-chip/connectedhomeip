//
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

#import <XCTest/XCTest.h>
#import <Matter/Matter.h>
#import "MTRXPCService.h"

@interface MTRXPCServiceTests<NSXPCListenerDelegate> : XCTestCase
@property (nonatomic, readwrite, strong) NSXPCListener * xpcListener;
@property (nonatomic, readwrite, strong) NSXPCInterface * serviceInterface;
@property (nonatomic, readwrite, strong) NSXPCInterface * clientInterface;
@property (readwrite, strong) NSXPCConnection * xpcConnection;
@end

@implementation MTRXPCServiceTests

// NSXPCListenerDelegate
- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    XCTAssertNil(_xpcConnection);
    XCTAssertNotNil(newConnection);
    NSLog(@"XPC listener accepting connection");
    newConnection.exportedInterface = _serviceInterface;
//    newConnection.remoteObjectInterface = _clientInterface;
//    newConnection.exportedObject = self;
    newConnection.invalidationHandler = ^{
        dispatch_async(dispatch_get_main_queue(), ^{
            NSLog(@"XPC connection disconnected");
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

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
    _xpcListener = [NSXPCListener anonymousListener];
    [_xpcListener setDelegate:(id<NSXPCListenerDelegate>)self];
    _serviceInterface = [MTRXPCService xpcInterfaceForServiceServerProtocol];
    [_xpcListener resume];
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
}

@end
