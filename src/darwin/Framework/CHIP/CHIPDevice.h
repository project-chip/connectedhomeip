/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef CHIP_DEVICE_H
#define CHIP_DEVICE_H

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef void (^SubscriptionEstablishedHandler)(void);

@interface CHIPDevice : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Subscribe to receive attribute reports for everything (all endpoints, all
 * clusters, all attributes, all events) on the device.
 *
 * reportHandler will be called any time a data update is available (with a
 * non-nil "value" and nil "error"), or any time there is an error (with a nil
 * "value" and non-nil "error").  If it's called with an error, that will
 * terminate the subscription.
 *
 * The array passed to reportHandler will contain CHIPAttributeReport instances.
 *
 * subscriptionEstablishedHandler, if not nil, will be called once the
 * subscription is established.  This will be _after_ the first (priming) call
 * to reportHandler.
 *
 * TODO: The "all events" part does not work yet.
 */
- (void)subscribeWithQueue:(dispatch_queue_t)queue
                minInterval:(uint16_t)minInterval
                maxInterval:(uint16_t)maxInterval
              reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
    subscriptionEstablished:(SubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler;
@end

@interface CHIPAttributePath : NSObject
@property (nonatomic, readonly, strong, nonnull) NSNumber * endpoint;
@property (nonatomic, readonly, strong, nonnull) NSNumber * cluster;
@property (nonatomic, readonly, strong, nonnull) NSNumber * attribute;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

@interface CHIPAttributeReport : NSObject
@property (nonatomic, readonly, strong, nonnull) CHIPAttributePath * path;
// value is nullable because nullable attributes can have nil as value.
@property (nonatomic, readonly, strong, nullable) id value;
@end

NS_ASSUME_NONNULL_END

#endif /* CHIP_DEVICE_H */
