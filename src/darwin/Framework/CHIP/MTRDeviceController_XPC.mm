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

@interface MTRDeviceController_XPC ()

@property (retain, readwrite) NSXPCConnection * xpcConnection;

@end

//#define MTR_HAVE_MACH_SERVICE_NAME_CONSTRUCTOR

@implementation MTRDeviceController_XPC

#ifdef MTR_HAVE_MACH_SERVICE_NAME_CONSTRUCTOR
- (id)initWithMachServiceName:(NSString *)machServiceName options:(NSXPCConnectionOptions)options
{
    MTR_LOG_DEBUG("%s")
    if (!(self = [super initForSubclasses])) {
        return nil;
    }

    self.xpcConnection = [[NSXPCConnection alloc] initWithMachServiceName:machServiceName options:options];
    self.xpcConnection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCServerProtocol)];
    
    self.xpcConnection.exportedInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCClientProtocol)];
    self.xpcConnection.exportedObject = self;

    MTR_LOG_DEBUG("%s: resuming new XPC connection");
    [self.xpcConnection resume];

    id<MTRXPCServerProtocol> proxy = [self.xpcConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) {
        MTR_LOG_ERROR("%s: XPC remote object proxy error.", __PRETTY_FUNCTION__);
    }];



    return self;
}
#endif // MTR_HAVE_MACH_SERVICE_NAME_CONSTRUCTOR

- (nullable instancetype)initWithParameters:(MTRDeviceControllerAbstractParameters *)parameters
                                      error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("%s: unimplemented method called", __PRETTY_FUNCTION__);
    return nil;
}

@end

@implementation MTRDeviceController_XPC (MTRXPCClientProtocol_MTRDeviceController)
    // as of 2024-08-16, nothing needed here.
@end
