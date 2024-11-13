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

#ifndef MCAttribute_h
#define MCAttribute_h

@interface MCAttribute<__covariant ObjectType> : NSObject

/**
 * @brief Reads the value of the MCAttribute that belongs to the associated MCEndpoint and corresponding MCCluster
 * @param context current context passed back in completion block
 * @param completion Called when attribute read completes with nil NSError if successful and before, after values. On failure, the NSError describes the error
 */
- (void)read:(void * _Nullable)context
    completion:(void (^_Nonnull)(void * _Nullable context, ObjectType _Nullable before, ObjectType _Nullable after, NSError * _Nullable error))completion;

/**
 * @brief Subscribe to the value of the MCAttribute that belongs to the associated MCEndpoint and corresponding MCCluster
 * @param context current context passed back in completion block
 * @param completion Called when attribute read completes with nil NSError if successful and before, after values. On failure, the NSError describes the error
 * @param minInterval the requested minimum interval boundary floor in seconds for attribute udpates
 * @param maxInterval the requested maximum interval boundary ceiling in seconds for attribute udpates
 */
- (void)subscribe:(void * _Nullable)context
       completion:(void (^_Nonnull)(void * _Nullable context, ObjectType _Nullable before, ObjectType _Nullable after, NSError * _Nullable error))completion
      minInterval:(NSNumber * _Nonnull)minInterval
      maxInterval:(NSNumber * _Nonnull)maxInterval;
@end

#endif /* MCAttribute_h */
