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

@end

@implementation MTRDeviceController_XPC

// KMO TODO:
// startup parameters for XPC version of controller

- (id)initWithXPCListenerEndpointForTesting:(NSXPCListenerEndpoint *)listenerEndpoint
{
    if (!(self = [super initForSubclasses])) {
        return nil;
    }
    self.xpcConnection = [[NSXPCConnection alloc] initWithListenerEndpoint:listenerEndpoint];
    self.xpcConnection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCServiceProtocol)];

    // maybe poor architecture to store this instead of access it thru XPC connection every time kmo 15 aug 2024

    // REVIEWERS: both of below inits result in proxies that complain:
    // "Return type of methods sent over this proxy must be 'void' or 'NSProgress *'"
    // which Internets seem to suggest is true.  kmo 15 aug 2024
    self.xpcRemoteObjectProxy = [self.xpcConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) {
        NSLog(@"%s: it's not my fault! XPC remote object proxy error.", __PRETTY_FUNCTION__);
    }];;
    // self.xpcRemoteObjectProxy = self.xpcConnection.remoteObjectProxy;

    [self.xpcConnection resume];

    // TODO:  something seems wrong at this point so clearly subsequent `xpcRemoteObjectProxy` calls won't
    // fare much better.  kmo 15 aug 2024 10h52
//    NSNumber * postInitMeaningOfLife = [self.xpcRemoteObjectProxy synchronouslyGetMeaningOfLife];
    NSNumber * postInitMeaningOfLife = [self.xpcRemoteObjectProxy synchronouslyGetMeaningOfLife];
    NSLog(@"%s: postInitMeaningOfLife = %@", __PRETTY_FUNCTION__, postInitMeaningOfLife);

    return self;
}

- (void)testPing {
    NSLog(@"pinging via %s", __PRETTY_FUNCTION__);
    [self.xpcRemoteObjectProxy ping];
}

// this blows up.  kmo 15 aug 2024 12h29
- (NSNumber *)meaningOfLife {
    return [self.xpcRemoteObjectProxy synchronouslyGetMeaningOfLife];
}

- (NSNumber *)internallyAsyncMeaningOfLife {
    __block int result;
    // semaphores probably not ultimately the right way to do this? kmo 15 aug 2024
    auto semaphore = dispatch_semaphore_create(0);
    [self.xpcRemoteObjectProxy getMeaningOfLifeWithReplyBlock:^(int reply) {
        result = reply;
    }];
    dispatch_semaphore_wait(semaphore, 1.0);
    // so, this will return an undefined value if timed out? kmo 15 aug 2024 12h21
    return @(result);
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
