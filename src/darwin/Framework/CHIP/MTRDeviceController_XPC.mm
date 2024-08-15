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
#import "MTRXPCServiceProtocol.h"
#import "MTRLogging_Internal.h"
#import "MTRDeviceController_Internal.h"

@interface MTRDeviceController_XPC ()
@property (retain, readwrite) NSXPCConnection * xpcConnection;
@property (retain, readwrite) id<MTRXPCServiceProtocol> xpcRemoteObjectProxy;

// testing only
@property (retain, readwrite) NSXPCListener * testListener;

@end

@implementation MTRDeviceController_XPC (TestXPCListener)

@end

@implementation MTRDeviceController_XPC

- (id)initWithTestXPCListener {
    // TODO:  update with well-known service name
//    self.xpcConnection = [[NSXPCConnection alloc] initWithServiceName:@"wellknown"];

    // testing mode
    self.testListener = [NSXPCListener anonymousListener];
    [self.testListener setDelegate:self];
    [self.testListener resume];
    NSXPCConnection * xpcConnection = [[NSXPCConnection alloc] initWithListenerEndpoint:self.testListener.endpoint];

    return [self initWithXPCConnection:xpcConnection];
}

- (id)initWithXPCConnection:(NSXPCConnection *)newConnection
{
//    if (!(self = [super initForSubclasses])) {
//        return nil;
//    }

    self.xpcConnection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCServiceProtocol)];
    self.xpcRemoteObjectProxy = self.xpcConnection.remoteObjectProxy;
    [self.xpcConnection resume];

    // ping and meaning of life just as a test
    [self.xpcRemoteObjectProxy ping];
    NSNumber * lifeMeaning = [self.xpcRemoteObjectProxy synchronouslyGetMeaningOfLife];
    NSLog(@"meaning of life appears to be %@", lifeMeaning);

    return self;
}

- (nullable instancetype)initWithParameters:(MTRDeviceControllerAbstractParameters *)parameters
                                      error:(NSError * __autoreleasing *)error 
{
    MTR_LOG_ERROR("unimplemented method %s called", __PRETTY_FUNCTION__);
    // initiate XPC connection? kmo 14 aug 2024 12h35
    return nil;
}

- (BOOL)setupCommissioningSessionWithPayload:(MTRSetupPayload *)payload
                                   newNodeID:(NSNumber *)newNodeID
                                       error:(NSError * __autoreleasing *)error
{
    MTR_LOG_DEBUG("called XPC stub %s", __PRETTY_FUNCTION__);
    return [self.xpcRemoteObjectProxy setupCommissioningSessionWithPayload:payload newNodeID:newNodeID error:error];
}

@end
