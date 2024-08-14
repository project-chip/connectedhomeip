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
#import "MTRLogging_Internal.h"

@interface MTRDeviceController_XPC ()
@property (nonatomic, strong) NSXPCConnection * xpcConnection;
@end

@implementation MTRDeviceController_XPC

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
    // xpc things obviously! kmo 14 aug 2024 12h35
    // transform async work to sync work

    MTR_LOG_ERROR("unimplemented method %s called", __PRETTY_FUNCTION__);
    return false;
}

typedef void (^BoolReplyBlock)(bool);
- (void)_xpc_setupCommissioningSessionWithPayload:(MTRSetupPayload *)payload
                                        newNodeID:(NSNumber *)newNodeID
                                            error:(NSError * __autoreleasing *)error
                                            reply:(BoolReplyBlock)reply
{
    MTR_LOG_ERROR("unimplemented XPC method %s called", __PRETTY_FUNCTION__);
    reply(false);
}

@end
