/**
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

#ifndef MCCommand_h
#define MCCommand_h

@interface MCCommand<RequestType, ResponseType> : NSObject

/**
 * @brief Invokes this MCCommand on the associated MCEndpoint and corresponding MCCluster
 *
 * @param request request data corresponding to this command invocation
 * @param completion Called when command execution completes with nil NSError if successful and responseData. On failure, the NSError describes the error
 * @param timedInvokeTimeoutMs command timeout in ms
 */
- (void)invoke:(RequestType _Nonnull)request
                 context:(void * _Nullable)context
              completion:(void (^_Nonnull)(void * _Nullable, NSError * _Nullable, ResponseType _Nullable))completion
    timedInvokeTimeoutMs:(NSNumber * _Nullable)timedInvokeTimeoutMs;

@end

#endif /* MCCommand_h */
