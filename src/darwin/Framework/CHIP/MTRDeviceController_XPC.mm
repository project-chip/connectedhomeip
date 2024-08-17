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
#import "MTRDeviceController_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRXPCClientProtocol.h"
#import "MTRXPCServerProtocol.h"
#import "MTRXPCServiceProtocol.h"

@interface MTRDeviceController_XPC ()

@property (retain, readwrite) NSXPCConnection * xpcConnection;

@end

//#define MTR_HAVE_MACH_SERVICE_NAME_CONSTRUCTOR
@implementation MTRDeviceController_XPC

#ifdef MTR_HAVE_MACH_SERVICE_NAME_CONSTRUCTOR
- (id)initWithMachServiceName:(NSString *)machServiceName options:(NSXPCConnectionOptions)options
{
    if (!(self = [super initForSubclasses])) {
        return nil;
    }

    self.xpcConnection = [[NSXPCConnection alloc] initWithMachServiceName:machServiceName options:options];
    self.xpcConnection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCServerProtocol)];
    self.xpcConnection.exportedInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCClientProtocol)];

    // TODO:  implement client protocol somewhere, probably on this object
    // kmo 16 aug 2024 12h26
    // self.xpcConnection.exportedObject = self;

    [self.xpcConnection resume];

    id<MTRXPCServerProtocol> proxy = [self.xpcConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) {
        MTR_LOG_ERROR("%s: XPC remote object proxy error.", __PRETTY_FUNCTION__);
    }];

    NSDictionary * helloContext = @{ @"human_readable_context" : @"hello" };
    [proxy deviceController:self.uniqueIdentifier checkInWithContext:helloContext];

    return self;
}
#endif // MTR_HAVE_MACH_SERVICE_NAME_CONSTRUCTOR

- (id)initWithXPCListenerEndpointForTesting:(NSXPCListenerEndpoint *)listenerEndpoint
{
    if (!(self = [super initForSubclasses])) {
        return nil;
    }
    self.xpcConnection = [[NSXPCConnection alloc] initWithListenerEndpoint:listenerEndpoint];
    self.xpcConnection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCServiceProtocol)];
    [self.xpcConnection resume];

    // TODO:  something seems wrong at this point so clearly subsequent `xpcRemoteObjectProxy` calls won't fare much better

    return self;
}

- (void)testPing
{
    MTR_LOG_ERROR("ping not supported in new XPC Server protocol");
    //    MTR_LOG_DEBUG("pinging via %s", __PRETTY_FUNCTION__);
    //    id<MTRXPCServerProtocol> proxy = [self.xpcConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) {
    //        MTR_LOG_ERROR("%s: XPC remote object proxy error.", __PRETTY_FUNCTION__);
    //    }];
    //    [proxy ping];
}

- (nullable instancetype)initWithParameters:(MTRDeviceControllerAbstractParameters *)parameters
                                      error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("unimplemented method %s called", __PRETTY_FUNCTION__);
    return nil;
}

@end
