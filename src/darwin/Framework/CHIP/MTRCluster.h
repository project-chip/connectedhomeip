/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#import <Matter/MTRDefines.h>

MTR_DEPRECATED("ResponseHandler is not used", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
typedef void (^ResponseHandler)(id _Nullable value, NSError * _Nullable error);
MTR_DEPRECATED("Please use MTRStatusCompletion instead", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
typedef void (^StatusCompletion)(NSError * _Nullable error);
MTR_DEPRECATED(
    "Please use MTRSubscriptionEstablishedHandler instead", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
typedef void (^SubscriptionEstablishedHandler)(void);

typedef void (^MTRStatusCompletion)(NSError * _Nullable error);
typedef void (^MTRSubscriptionEstablishedHandler)(void);

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
 * This value is specified in milliseconds.
 */
@property (nonatomic, copy, nullable) NSNumber * timedWriteTimeout;

/**
 * Sets the data version for the Write Request for the interaction.
 *
 * If not nil, the write will only succeed if the current data version of
 * the cluster matches the provided data version.
 */
@property (nonatomic, copy, nullable) NSNumber * dataVersion;

@end

/**
 * MTRReadParams
 *    This is used to control the behavior of attribute/event reads and subscribes.
 *    If not provided (i.e. nil passed for the MTRReadParams argument), will be
 *    treated as if a default-initialized object was passed in.
 */
@interface MTRReadParams : NSObject <NSCopying>

/**
 * Whether the read/subscribe is fabric-filtered. The default is YES.
 *
 * If YES, the read/subscribe is fabric-filtered and will only see things
 * associated with the fabric of the reader/subscriber.
 *
 * If NO, the read/subscribe is not fabric-filtered and will see all
 * non-fabric-sensitive data for the given attribute path.
 */
@property (nonatomic, assign, getter=shouldFilterByFabric)
    BOOL filterByFabric API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Sets a filter for which events will be reported in the read/subscribe interaction.
 *
 * If nil (the default value), all of the queued events will be reported from lowest to highest event number.
 *
 * If not nil, queued events with an event number smaller than minEventNumber will not be reported.
 */
@property (nonatomic, copy, nullable) NSNumber * minEventNumber API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * MTRSubscribeParams
 *    This is used to control the behavior of attribute/event subscribes.  If not
 *    provided (i.e. nil passed for the MTRSubscribeParams argument), will be
 *    treated as if a default-initialized object was passed in.
 */
@interface MTRSubscribeParams : MTRReadParams

/**
 * Whether the subscribe should replace already-existing
 * subscriptions.  The default value is YES.
 *
 * If YES, the subscribe will cancel any existing subscriptions to the target
 * node when it sets up the new one.
 *
 * If NO, the subscribe will allow any previous subscriptions to remain.
 */
@property (nonatomic, assign, getter=shouldReplaceExistingSubscriptions)
    BOOL replaceExistingSubscriptions API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Whether the subscription should automatically try to re-establish if it
 * drops.  The default value is YES.
 *
 * If NO, loss of subscription will simply lead to an error report.  Some
 * subscription APIs do not support this value.
 *
 * If YES, loss of subscription will lead to an automatic resubscription
 * attempt.  If this succeeds, the subscriptionEstablished callback will be
 * called again.
 *
 */
@property (nonatomic, assign, getter=shouldResubscribeAutomatically)
    BOOL resubscribeAutomatically API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * The minimum time, in seconds, between consecutive reports a server will send
 * for this subscription.  This can be used to rate-limit the subscription
 * traffic.  Any non-negative value is allowed, including 0.
 */
@property (nonatomic, copy) NSNumber * minInterval API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * The suggested maximum time, in seconds, during which the server is allowed to
 * send no reports at all for this subscription.  Must be at least as large as
 * minInterval.  The server is allowed to use a larger time than this as the
 * maxInterval it selects if it needs to (e.g. to meet its power budget).
 */
@property (nonatomic, copy) NSNumber * maxInterval API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Controls whether events will be reported urgently. The default value is YES.
 *
 * If YES, the events will be reported as soon as the minInterval does not prevent it.
 *
 * If NO, the events will be reported at the maximum interval.
 */
@property (nonatomic, assign, getter=shouldReportEventsUrgently)
    BOOL reportEventsUrgently API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Initialize an MTRSubscribeParams.  Must provide a minInterval and
 * maxInterval; there are no default values for those.
 */
- (instancetype)initWithMinInterval:(NSNumber *)minInterval maxInterval:(NSNumber *)maxInterval;

@end

@interface MTRReadParams (Deprecated)

@property (nonatomic, copy, nullable) NSNumber * fabricFiltered MTR_DEPRECATED(
    "Please use filterByFabric", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRSubscribeParams (Deprecated)

@property (nonatomic, copy, nullable) NSNumber * keepPreviousSubscriptions MTR_DEPRECATED(
    "Please use replaceExistingSubscriptions", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy, nullable) NSNumber * autoResubscribe MTR_DEPRECATED(
    "Please use resubscribeAutomatically", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * init and new exist for now, for backwards compatibility, and initialize with
 * minInterval set to 1 and maxInterval set to 0, which will not work on its
 * own.  Uses of MTRSubscribeParams that rely on init must all be using
 * (deprecated) APIs that pass in a separate minInterval and maxInterval.
 */
- (instancetype)init MTR_DEPRECATED(
    "Please use initWithMinInterval", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (instancetype)new MTR_DEPRECATED(
    "Please use initWithMinInterval", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

NS_ASSUME_NONNULL_END
