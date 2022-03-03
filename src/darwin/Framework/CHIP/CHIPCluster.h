/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

@class CHIPDevice;

NS_ASSUME_NONNULL_BEGIN

/**
 * CHIPCluster
 *    This is the base class for clusters.
 */
@interface CHIPCluster : NSObject
- (nullable instancetype)initWithDevice:(CHIPDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER;
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

/**
 * CHIPReadParams
 *    This is used to control the behavior of attribute reads and subscribes.
 *    If not provided (i.e. nil passed for the CHIPReadParams argument), will be
 *    treated as if a default-initialized object was passed in.
 */
@interface CHIPReadParams : NSObject

/**
 * Whether the read/subscribe is fabric-filtered. nil (the default value) is
 * treated as YES.
 *
 * If YES, the read/subscribe is fabric-filtered and will only see things
 * associated with the fabric of the reader/subscriber.
 *
 * If NO, the read/subscribe is not fabric-filtered and will see all
 * non-fabric-sensitive data for the given attribute path.
 */
@property (strong, nonatomic, nullable) NSNumber * fabricFiltered;

- (instancetype)init;
@end

/**
 * CHIPSubscribeParams
 *    This is used to control the behavior of attribute subscribes.  If not
 *    provided (i.e. nil passed for the CHIPSubscribeParams argument), will be
 *    treated as if a default-initialized object was passed in.
 */
@interface CHIPSubscribeParams : CHIPReadParams

/**
 * Whether the subscribe should allow previous subscriptions to stay in
 * place. nil (the default value) is treated as NO.
 *
 * If NO, the subscribe will cancel any existing subscriptions to the target
 * node when it sets up the new one.
 *
 * If YES, the subscribe will allow any previous subscriptions to remain.
 */
@property (strong, nonatomic, nullable) NSNumber * keepPreviousSubscriptions;

- (instancetype)init;
@end

NS_ASSUME_NONNULL_END
