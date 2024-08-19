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
#import "MTRDeviceDelegateInfo.h"
#import "MTRDeviceStorageBehaviorConfiguration_Internal.h"

#import <os/lock.h>

@class MTRDeviceConnectivityMonitor;

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
- (void)removeValueForAttribute:(NSNumber *)attribute;

- (nullable instancetype)initWithDataVersion:(NSNumber * _Nullable)dataVersion attributes:(NSDictionary<NSNumber *, MTRDeviceDataValueDictionary> * _Nullable)attributes;
@end

@interface MTRDevice () {
#ifdef DEBUG
    NSUInteger _unitTestAttributesReportedSinceLastCheck;
#endif

    // _deviceCachePrimed is true if we have the data that comes from an initial
    // subscription priming report (whether it came from storage or from our
    // subscription).
    BOOL _deviceCachePrimed;

    // _persistedClusterData stores data that we have already persisted (when we have
    // cluster data persistence enabled).  Nil when we have no persistence enabled.
    NSCache<MTRClusterPath *, MTRDeviceClusterData *> * _Nullable _persistedClusterData;
    // _clusterDataToPersist stores data that needs to be persisted.  If we
    // don't have persistence enabled, this is our only data store.  Nil if we
    // currently have nothing that could need persisting.
    NSMutableDictionary<MTRClusterPath *, MTRDeviceClusterData *> * _Nullable _clusterDataToPersist;
    // _persistedClusters stores the set of "valid" keys into _persistedClusterData.
    // These are keys that could have values in _persistedClusterData even if they don't
    // right now (because they have been evicted).
    NSMutableSet<MTRClusterPath *> * _persistedClusters;

    // When we last failed to subscribe to the device (either via
    // _setupSubscriptionWithReason or via the auto-resubscribe behavior
    // of the ReadClient).  Nil if we have had no such failures.
    NSDate * _Nullable _lastSubscriptionFailureTime;
    MTRDeviceConnectivityMonitor * _connectivityMonitor;

    // This boolean keeps track of any device configuration changes received in an attribute report.
    // If this is true when the report ends, we notify the delegate.
    BOOL _deviceConfigurationChanged;

    // The completion block is set when the subscription / resubscription work is enqueued, and called / cleared when any of the following happen:
    //   1. Subscription establishes
    //   2. OnResubscriptionNeeded is called
    //   3. Subscription reset (including when getSessionForNode fails)
    MTRAsyncWorkCompletionBlock _subscriptionPoolWorkCompletionBlock;

    // Tracking of initial subscribe latency.  When _initialSubscribeStart is
    // nil, we are not tracking the latency.
    NSDate * _Nullable _initialSubscribeStart;

    // Storage behavior configuration and variables to keep track of the logic
    //  _clusterDataPersistenceFirstScheduledTime is used to track the start time of the delay between
    //      report and persistence.
    //  _mostRecentReportTimes is a list of the most recent report timestamps used for calculating
    //      the running average time between reports.
    //  _deviceReportingExcessivelyStartTime tracks when a device starts reporting excessively.
    //  _reportToPersistenceDelayCurrentMultiplier is the current multiplier that is calculated when a
    //      report comes in.
    MTRDeviceStorageBehaviorConfiguration * _storageBehaviorConfiguration;
    NSDate * _Nullable _clusterDataPersistenceFirstScheduledTime;
    NSMutableArray<NSDate *> * _mostRecentReportTimes;
    NSDate * _Nullable _deviceReportingExcessivelyStartTime;
    double _reportToPersistenceDelayCurrentMultiplier;

    // System time change observer reference
    id _systemTimeChangeObserverToken;

    // Protects mutable state used by our description getter.  This is a separate lock from "lock"
    // so that we don't need to worry about getting our description while holding "lock" (e.g due to
    // logging self).  This lock _must_ be held narrowly, with no other lock acquisitions allowed
    // while it's held, to avoid deadlock.
    os_unfair_lock _descriptionLock;

    // State used by our description getter: access to these must be protected by descriptionLock.
    NSNumber * _Nullable _vid; // nil if unknown
    NSNumber * _Nullable _pid; // nil if unknown
    // _allNetworkFeatures is a bitwise or of the feature maps of all network commissioning clusters
    // present on the device, or nil if there aren't any.
    NSNumber * _Nullable _allNetworkFeatures;
    // Copy of _internalDeviceState that is safe to use in description.
    MTRInternalDeviceState _internalDeviceStateForDescription;
    // Copy of _lastSubscriptionAttemptWait that is safe to use in description.
    uint32_t _lastSubscriptionAttemptWaitForDescription;
    // Most recent entry in _mostRecentReportTimes, if any.
    NSDate * _Nullable _mostRecentReportTimeForDescription;
    // Copy of _lastSubscriptionFailureTime that is safe to use in description.
    NSDate * _Nullable _lastSubscriptionFailureTimeForDescription;
}

- (instancetype)initForSubclasses;
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

- (BOOL)_callDelegatesWithBlock:(void (^)(id<MTRDeviceDelegate> delegate))block;

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
@property (nonatomic, retain, readwrite) NSMutableSet<MTRDeviceDelegateInfo *> * delegates;

@property (nonatomic, readonly) os_unfair_lock lock; // protects the caches and device state
// protects against concurrent time updates by guarding timeUpdateScheduled flag which manages time updates scheduling,
// and protects device calls to setUTCTime and setDSTOffset.  This can't just be replaced with "lock", because the time
// update code calls public APIs like readAttributeWithEndpointID:.. (which attempt to take "lock") while holding
// whatever lock protects the time sync bits.
@property (nonatomic, readonly) os_unfair_lock timeSyncLock;

@property (nonatomic) BOOL receivingReport;
@property (nonatomic) BOOL receivingPrimingReport;

// TODO: instead of all the BOOL properties that are some facet of the state, move to internal state machine that has (at least):
//   Actively receiving report
//   Actively receiving priming report

@property (nonatomic) MTRInternalDeviceState internalDeviceState;

#define MTRDEVICE_SUBSCRIPTION_ATTEMPT_MIN_WAIT_SECONDS (1)
#define MTRDEVICE_SUBSCRIPTION_ATTEMPT_MAX_WAIT_SECONDS (3600)
@property (nonatomic) uint32_t lastSubscriptionAttemptWait;

/**
 * If reattemptingSubscription is true, that means that we have failed to get a
 * CASE session for the publisher and are now waiting to try again.  In this
 * state we never have subscriptionActive true or a non-null currentReadClient.
 */
@property (nonatomic) BOOL reattemptingSubscription;

// Expected value cache is attributePath => NSArray of [NSDate of expiration time, NSDictionary of value, expected value ID]
//   - See MTRDeviceExpectedValueFieldIndex for the definitions of indices into this array.
// See MTRDeviceResponseHandler definition for value dictionary details.
@property (nonatomic) NSMutableDictionary<MTRAttributePath *, NSArray *> * expectedValueCache;

// This is a monotonically increasing value used when adding entries to expectedValueCache
// Currently used/updated only in _getAttributesToReportWithNewExpectedValues:expirationTime:expectedValueID:
@property (nonatomic) uint64_t expectedValueNextID;

@property (nonatomic) BOOL expirationCheckScheduled;

@property (nonatomic, assign) BOOL timeUpdateScheduled;

@property (nonatomic, retain) NSDate * estimatedStartTimeFromGeneralDiagnosticsUpTime;

@property (nonatomic, retain) NSMutableDictionary * temporaryMetaDataCache;

// Method to insert persisted cluster data
//   Contains data version information and attribute values.
- (void)setPersistedClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)clusterData;

// Method to insert persisted data that pertains to the whole device.
- (void)setPersistedDeviceData:(NSDictionary<NSString *, id> *)data;

#ifdef DEBUG
- (NSUInteger)unitTestAttributeCount;
- (void)_callFirstDelegateSynchronouslyWithBlock:(void (^)(id<MTRDeviceDelegate> delegate))block;
#endif

- (void)setStorageBehaviorConfiguration:(MTRDeviceStorageBehaviorConfiguration *)storageBehaviorConfiguration;

// Returns whether this MTRDevice uses Thread for communication
- (BOOL)deviceUsesThread;

- (BOOL)_subscriptionsAllowed;
- (BOOL)_deviceUsesThread;

- (void)_scheduleSubscriptionPoolWork:(dispatch_block_t)workBlock inNanoseconds:(int64_t)inNanoseconds description:(NSString *)description;
- (void)_setupSubscriptionWithReason:(NSString *)reason;

@end

#pragma mark - Constants

static NSString * const kDefaultSubscriptionPoolSizeOverrideKey = @"subscriptionPoolSizeOverride";
static NSString * const kTestStorageUserDefaultEnabledKey = @"enableTestStorage";

// ex-MTRDeviceClusterData constants
static NSString * const sDataVersionKey = @"dataVersion";
static NSString * const sAttributesKey = @"attributes";
static NSString * const sLastInitialSubscribeLatencyKey = @"lastInitialSubscribeLatency";

// Declared inside platform, but noting here for reference
// static NSString * const kSRPTimeoutInMsecsUserDefaultKey = @"SRPTimeoutInMSecsOverride";

// Declaring selector so compiler won't complain about testing and calling it in _handleReportEnd
#ifdef DEBUG
@protocol MTRDeviceUnitTestDelegate <MTRDeviceDelegate>
- (void)unitTestReportEndForDevice:(MTRDevice *)device;
- (BOOL)unitTestShouldSetUpSubscriptionForDevice:(MTRDevice *)device;
- (BOOL)unitTestShouldSkipExpectedValuesForWrite:(MTRDevice *)device;
- (NSNumber *)unitTestMaxIntervalOverrideForSubscription:(MTRDevice *)device;
- (BOOL)unitTestForceAttributeReportsIfMatchingCache:(MTRDevice *)device;
- (BOOL)unitTestPretendThreadEnabled:(MTRDevice *)device;
- (void)unitTestSubscriptionPoolDequeue:(MTRDevice *)device;
- (void)unitTestSubscriptionPoolWorkComplete:(MTRDevice *)device;
- (void)unitTestClusterDataPersisted:(MTRDevice *)device;
- (BOOL)unitTestSuppressTimeBasedReachabilityChanges:(MTRDevice *)device;
@end
#endif

NS_ASSUME_NONNULL_END
