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

- (id)initWithXPCConnection:(NSXPCConnection *)newConnection
{
    if (!(self = [super initForSubclasses])) {
        return nil;
    }

    self.xpcConnection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCServiceProtocol)];
    self.xpcRemoteObjectProxy = self.xpcConnection.remoteObjectProxy;
    [self.xpcConnection resume];
    
    // TODO:  something seems wrong at this point so clearly subsequent `xpcRemoteObjectProxy` calls won't
    // fare much better.  kmo 15 aug 2024 10h52
    NSNumber * postInitMeaningOfLife = [self.xpcRemoteObjectProxy synchronouslyGetMeaningOfLife];
    NSLog(@"%s: postInitMeaningOfLife = %@", __PRETTY_FUNCTION__, postInitMeaningOfLife);

    return self;
}

- (void)testPing {
    NSLog(@"pinging via %s", __PRETTY_FUNCTION__);
    [self.xpcRemoteObjectProxy ping];
}

- (NSNumber *)meaningOfLife {
    return [self.xpcRemoteObjectProxy synchronouslyGetMeaningOfLife];
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
