/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#import <Matter/MTRBaseDevice.h>
#import <Matter/MTRDevice.h>

#import "MTRAsyncWorkQueue.h"
#import "MTRDefines_Internal.h"
#import "MTRDeviceStorageBehaviorConfiguration_Internal.h"

NS_ASSUME_NONNULL_BEGIN

@class MTRAsyncWorkQueue;

typedef NSDictionary<NSString *, id> * MTRDeviceDataValueDictionary;

typedef void (^MTRDevicePerformAsyncBlock)(MTRBaseDevice * baseDevice);

typedef NS_ENUM(NSUInteger, MTRInternalDeviceState) {
    // Unsubscribed means we do not have a subscription and are not trying to set one up.
    MTRInternalDeviceStateUnsubscribed = 0,
    // Subscribing means we are actively trying to establish our initial subscription (e.g. doing
    // DNS-SD discovery, trying to establish CASE to the peer, getting priming reports, etc).
    MTRInternalDeviceStateSubscribing = 1,
    // InitialSubscriptionEstablished means we have at some point finished setting up a
    // subscription.  That subscription may have dropped since then, but if so it's the ReadClient's
    // responsibility to re-establish it.
    MTRInternalDeviceStateInitialSubscriptionEstablished = 2,
    // Resubscribing means we had established a subscription, but then
    // detected a subscription drop due to not receiving a report on time. This
    // covers all the actions that happen when re-subscribing (discovery, CASE,
    // getting priming reports, etc).
    MTRInternalDeviceStateResubscribing = 3,
    // LaterSubscriptionEstablished meant that we had a subscription drop and
    // then re-created a subscription.
    MTRInternalDeviceStateLaterSubscriptionEstablished = 4,
};

/**
 * Information about a cluster: data version and known attribute values.
 */
MTR_TESTABLE
@interface MTRDeviceClusterData : NSObject <NSSecureCoding, NSCopying>
@property (nonatomic, nullable) NSNumber * dataVersion;
@property (nonatomic, readonly) NSDictionary<NSNumber *, MTRDeviceDataValueDictionary> * attributes; // attributeID => data-value dictionary

- (void)storeValue:(MTRDeviceDataValueDictionary _Nullable)value forAttribute:(NSNumber *)attribute;

- (nullable instancetype)initWithDataVersion:(NSNumber * _Nullable)dataVersion attributes:(NSDictionary<NSNumber *, MTRDeviceDataValueDictionary> * _Nullable)attributes;
@end

@interface MTRDevice ()
- (instancetype)initWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller;

// Called from MTRClusters for writes and commands
- (void)setExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)values
    expectedValueInterval:(NSNumber *)expectedValueIntervalMs;

// called by controller to clean up and shutdown
- (void)invalidate;

// Called by controller when a new operational advertisement for what we think
// is this device's identity has been observed.  This could have
// false-positives, for example due to compressed fabric id collisions.
- (void)nodeMayBeAdvertisingOperational;

/**
 * Like the public invokeCommandWithEndpointID but:
 *
 * 1) Allows passing through a serverSideProcessingTimeout.
 * 2) Expects one of the command payload structs as commandPayload
 * 3) On success, returns an instance of responseClass via the completion (or
 *    nil if there is no responseClass, which indicates a status-only command).
 */
- (void)_invokeKnownCommandWithEndpointID:(NSNumber *)endpointID
                                clusterID:(NSNumber *)clusterID
                                commandID:(NSNumber *)commandID
                           commandPayload:(id)commandPayload
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval
                       timedInvokeTimeout:(NSNumber * _Nullable)timeout
              serverSideProcessingTimeout:(NSNumber * _Nullable)serverSideProcessingTimeout
                            responseClass:(Class _Nullable)responseClass
                                    queue:(dispatch_queue_t)queue
                               completion:(void (^)(id _Nullable response, NSError * _Nullable error))completion;

// Queue used for various internal bookkeeping work.
@property (nonatomic) dispatch_queue_t queue;
@property (nonatomic, readonly) MTRAsyncWorkQueue<MTRDevice *> * asyncWorkQueue;

// Method to insert persisted cluster data
//   Contains data version information and attribute values.
- (void)setPersistedClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)clusterData;

// Method to insert persisted data that pertains to the whole device.
- (void)setPersistedDeviceData:(NSDictionary<NSString *, id> *)data;

#ifdef DEBUG
- (NSUInteger)unitTestAttributeCount;
#endif

- (void)setStorageBehaviorConfiguration:(MTRDeviceStorageBehaviorConfiguration *)storageBehaviorConfiguration;

@end

#pragma mark - Utility for clamping numbers
// Returns a NSNumber object that is aNumber if it falls within the range [min, max].
// Returns min or max, if it is below or above, respectively.
NSNumber * MTRClampedNumber(NSNumber * aNumber, NSNumber * min, NSNumber * max);

#pragma mark - Constants

static NSString * const kDefaultSubscriptionPoolSizeOverrideKey = @"subscriptionPoolSizeOverride";
static NSString * const kTestStorageUserDefaultEnabledKey = @"enableTestStorage";

// Declared inside platform, but noting here for reference
// static NSString * const kSRPTimeoutInMsecsUserDefaultKey = @"SRPTimeoutInMSecsOverride";

NS_ASSUME_NONNULL_END
