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
- (void)getMeaningOfLifeWithReplyBlock:(MTRXPCServiceIntReplyBlock)reply;

// these real methods will be optional for now, but not when we are done building here.
@optional
- (void)setupCommissioningSessionWithPayload:(MTRSetupPayload *)payload
newNodeID:(NSNumber *)newNodeID
error:(NSError * __autoreleasing *)error
reply:(MTRXPCServiceBoolReplyBlock)reply;

@end

NS_ASSUME_NONNULL_END
