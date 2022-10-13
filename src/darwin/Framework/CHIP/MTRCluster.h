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

typedef void (^ResponseHandler)(id _Nullable value, NSError * _Nullable error);
typedef void (^StatusCompletion)(NSError * _Nullable error);
typedef void (^SubscriptionEstablishedHandler)(void);

@class MTRBaseDevice;

NS_ASSUME_NONNULL_BEGIN

/**
 * MTRCluster
 *    This is the base class for clusters.
 */
@interface MTRCluster : NSObject
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

/**
 * MTRWriteParams
 *    This is used to control the behavior of cluster writes.
 *    If not provided (i.e. nil passed for the CHIPWriteParams argument), will be
 *    treated as if a default-initialized object was passed in.
 */
@interface MTRWriteParams : NSObject <NSCopying>

/**
 * Controls whether the write is a timed write.
 *
 * If nil (the default value), a regular write is done for attributes that do
 * not require a timed write and a timed write with some default timed request
 * timeout is done for attributes that require a timed write.
 *
 * If not nil, a timed write is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual write
 * request) within the timeout window.
 *
 * This value is specified in milliseconds
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedWriteTimeout;

/**
 * Sets the data version for the Write Request for the interaction.
 *
 * If not nil, the write will only succeed if the current data version of
 * the cluster matches the provided data version.
 */
@property (nonatomic, copy, nullable) NSNumber * dataVersion;

- (instancetype)init;
- (id)copyWithZone:(nullable NSZone *)zone;

@end

/**
 * MTRReadParams
 *    This is used to control the behavior of attribute reads and subscribes.
 *    If not provided (i.e. nil passed for the MTRReadParams argument), will be
 *    treated as if a default-initialized object was passed in.
 */
@interface MTRReadParams : NSObject <NSCopying>

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
@property (nonatomic, copy, nullable) NSNumber * fabricFiltered;

- (instancetype)init;
- (id)copyWithZone:(nullable NSZone *)zone;

@end

/**
 * MTRSubscribeParams
 *    This is used to control the behavior of attribute subscribes.  If not
 *    provided (i.e. nil passed for the MTRSubscribeParams argument), will be
 *    treated as if a default-initialized object was passed in.
 */
@interface MTRSubscribeParams : MTRReadParams

/**
 * Whether the subscribe should allow previous subscriptions to stay in
 * place. nil (the default value) is treated as NO.
 *
 * If NO, the subscribe will cancel any existing subscriptions to the target
 * node when it sets up the new one.
 *
 * If YES, the subscribe will allow any previous subscriptions to remain.
 */
@property (nonatomic, copy, nullable) NSNumber * keepPreviousSubscriptions;

/**
 * Whether the subscription should automatically try to re-establish if it
 * drops.  nil (the default value) is treated as YES.
 *
 * If NO, loss of subscription will simply lead to an error report.  Some
 * subscription APIs do not support this value.
 *
 * If YES, loss of subscription will lead to an automatic resubscription
 * attempt.  If this succeeds, the subscriptionEstablished callback will be
 * called again.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * autoResubscribe;

- (instancetype)init;
- (id)copyWithZone:(nullable NSZone *)zone;

@end

NS_ASSUME_NONNULL_END
