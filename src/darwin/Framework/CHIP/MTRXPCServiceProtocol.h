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

#import <Foundation/Foundation.h>

@class MTRSetupPayload;

NS_ASSUME_NONNULL_BEGIN

typedef void (^MTRXPCServiceIntReplyBlock)(int);
typedef void (^MTRXPCServiceBoolReplyBlock)(BOOL);

@protocol MTRXPCServiceProtocol <NSObject>
- (void)ping;
- (NSNumber *)synchronouslyGetMeaningOfLife;
- (void)getMeaningOfLifeWithReplyBlock:(MTRXPCServiceIntReplyBlock)reply;

// identify "who i am"
// need invalidation and disconnect
// temporary inclusion
- (BOOL)setupCommissioningSessionWithPayload:(MTRSetupPayload *)payload
                                   newNodeID:(NSNumber *)newNodeID
                                       error:(NSError * __autoreleasing *)error;

@end

// Add client protocol for e.g. attribute reports coming back not in response to a method call
// it will look an awful lot like the delegate!
// allow server to request UUID for reflection
// need invalidation and disconnect much like on service side
// TODO:  see how much of Woody's work covers this

//@protocol MTRXPCServiceDeviceControllerProtocol <NSObject>

//
//- (BOOL)setupCommissioningSessionWithPayload:(MTRSetupPayload *)payload
//                                   newNodeID:(NSNumber *)newNodeID
//                                       error:(NSError * __autoreleasing *)error;
//
//@end

NS_ASSUME_NONNULL_END
