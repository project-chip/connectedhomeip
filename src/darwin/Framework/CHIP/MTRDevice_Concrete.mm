/**
 *
 *    Copyright (c) 2022-2025 Project CHIP Authors
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

#import <Matter/Matter.h>
#import <os/lock.h>

#import "MTRAsyncWorkQueue.h"
#import "MTRAttributeSpecifiedCheck.h"
#import "MTRBaseClusters.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRBaseSubscriptionCallback.h"
#import "MTRCluster.h"
#import "MTRClusterConstants.h"
#import "MTRCommandTimedCheck.h"
#import "MTRConversion.h"
#import "MTRDefines_Internal.h"
#import "MTRDeviceConnectivityMonitor.h"
#import "MTRDeviceControllerOverXPC.h"
#import "MTRDeviceController_Internal.h"
#import "MTRDeviceDataValidation.h"
#import "MTRDevice_Concrete.h"
#import "MTRDevice_Internal.h"
#import "MTRError_Internal.h"
#import "MTREventTLVValueDecoder_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRMetricKeys.h"
#import "MTRMetricsCollector.h"
#import "MTRTimeUtils.h"
#import "MTRUnfairLock.h"
#import "MTRUtilities.h"
#import "zap-generated/MTRCommandPayloads_Internal.h"

#import "lib/core/CHIPError.h"
#import "lib/core/DataModelTypes.h"
#import <app/ConcreteAttributePath.h>
#import <lib/support/FibonacciUtils.h>

#import <app/AttributePathParams.h>
#import <app/BufferedReadCallback.h>
#import <app/ClusterStateCache.h>
#import <app/InteractionModelEngine.h>
#import <platform/LockTracker.h>
#import <platform/PlatformManager.h>

static NSString * const sLastInitialSubscribeLatencyKey = @"lastInitialSubscribeLatency";

static NSString * const sDeviceMayBeReachableReason = @"SPI client indicated the device may now be reachable";

// Not static, because these are public API.
NSString * const MTRPreviousDataKey = @"previousData";
NSString * const MTRDataVersionKey = @"dataVersion";

// allow readwrite access to superclass properties
@interface MTRDevice_Concrete ()

@property (nonatomic, readwrite) MTRAsyncWorkQueue<MTRDevice *> * asyncWorkQueue;
@property (nonatomic, readwrite) MTRDeviceState state;
@property (nonatomic, readwrite, nullable) NSDate * estimatedStartTime;
@property (nonatomic, readwrite, nullable, copy) NSNumber * estimatedSubscriptionLatency;
@property (nonatomic, readwrite, assign) BOOL suspended;

// nullable because technically _deviceController is nullable.
@property (nonatomic, readonly, nullable) MTRDeviceController_Concrete * _concreteController;

@end

typedef void (^MTRDeviceAttributeReportHandler)(NSArray * _Nonnull);

#define kSecondsToWaitBeforeMarkingUnreachableAfterSettingUpSubscription 10

// Disabling pending crashes
#define ENABLE_CONNECTIVITY_MONITORING 0

/* BEGIN DRAGONS: Note methods here cannot be renamed, and are used by private callers, do not rename, remove or modify behavior here */

@interface NSObject (MatterPrivateForInternalDragonsDoNotFeed)
- (void)_deviceInternalStateChanged:(MTRDevice *)device;
@end

/* END DRAGONS */

#pragma mark - SubscriptionCallback class declaration
using namespace chip;
using namespace chip::app;
using namespace chip::Protocols::InteractionModel;
using namespace chip::Tracing::DarwinFramework;

typedef void (^FirstReportHandler)(void);

namespace {

class SubscriptionCallback final : public MTRBaseSubscriptionCallback {
public:
    SubscriptionCallback(DataReportCallback attributeReportCallback, DataReportCallback eventReportCallback,
        ErrorCallback errorCallback, MTRDeviceResubscriptionScheduledHandler resubscriptionCallback,
        SubscriptionEstablishedHandler subscriptionEstablishedHandler, OnDoneHandler onDoneHandler,
        UnsolicitedMessageFromPublisherHandler unsolicitedMessageFromPublisherHandler, ReportBeginHandler reportBeginHandler,
        ReportEndHandler reportEndHandler)
        : MTRBaseSubscriptionCallback(attributeReportCallback, eventReportCallback, errorCallback, resubscriptionCallback,
            subscriptionEstablishedHandler, onDoneHandler, unsolicitedMessageFromPublisherHandler, reportBeginHandler,
            reportEndHandler)
    {
    }

    // Used to reset Resubscription backoff on events that indicate likely availability of device to come back online
    void ResetResubscriptionBackoff() { mResubscriptionNumRetries = 0; }

private:
    void OnSubscriptionEstablished(chip::SubscriptionId aSubscriptionId) override;

    void OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus) override;

    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override;

    CHIP_ERROR OnResubscriptionNeeded(chip::app::ReadClient * apReadClient, CHIP_ERROR aTerminationCause) override;

    // Copied from ReadClient and customized for MTRDevice resubscription time reset
    uint32_t ComputeTimeTillNextSubscription();
    uint32_t mResubscriptionNumRetries = 0;
};

} // anonymous namespace

#pragma mark - MTRDeviceMatterCPPObjectsHolder

// Class to hold C++ objects that can only be manipulated on the Matter queue
@interface MTRDeviceMatterCPPObjectsHolder : NSObject
@property (nonatomic, readonly) ReadClient * readClient;
@property (nonatomic, readonly) SubscriptionCallback * subscriptionCallback; // valid when and only when readClient is valid
- (void)setReadClient:(ReadClient * _Nullable)readClient subscriptionCallback:(SubscriptionCallback * _Nullable)subscriptionCallback;
- (void)clearReadClientAndDeleteSubscriptionCallback;
@end

@implementation MTRDeviceMatterCPPObjectsHolder
@synthesize readClient = _readClient;
@synthesize subscriptionCallback = _subscriptionCallback;
- (ReadClient *)readClient
{
    assertChipStackLockedByCurrentThread();
    @synchronized(self) {
        return _readClient;
    }
}
- (SubscriptionCallback *)subscriptionCallback
{
    assertChipStackLockedByCurrentThread();
    @synchronized(self) {
        return _subscriptionCallback;
    }
}
- (void)setReadClient:(ReadClient * _Nullable)readClient subscriptionCallback:(SubscriptionCallback * _Nullable)subscriptionCallback
{
    assertChipStackLockedByCurrentThread();
    @synchronized(self) {
        // Sanity check and log if readClient and subscriptionCallback aren't both valid or both null
        if (((readClient == nullptr) && (subscriptionCallback != nullptr)) || ((readClient != nullptr) && (subscriptionCallback == nullptr))) {
            MTR_LOG_ERROR("%@: setReadClient:subscriptionCallback: readClient and subscriptionCallback must both be valid or both be null %p %p", self, readClient, subscriptionCallback);
        }

        // Sanity check and log if overriding existing values
        if ((readClient != nullptr) && (_readClient != nullptr)) {
            MTR_LOG_ERROR("%@: setReadClient:subscriptionCallback: readClient set when current value not null %p %p", self, readClient, _readClient);
        }
        if (((subscriptionCallback != nullptr)) && (_subscriptionCallback != nullptr)) {
            MTR_LOG_ERROR("%@: setReadClient:subscriptionCallback: subscriptionCallback set when current value not null %p %p", self, subscriptionCallback, _subscriptionCallback);
        }

        _readClient = readClient;
        _subscriptionCallback = subscriptionCallback;
    }
}
- (void)clearReadClientAndDeleteSubscriptionCallback
{
    assertChipStackLockedByCurrentThread();
    @synchronized(self) {
        _readClient = nullptr;
        if (_subscriptionCallback) {
            delete _subscriptionCallback;
            _subscriptionCallback = nullptr;
        }
    }
}
@end

#pragma mark - MTRDevice

// Utility methods for working with MTRInternalDeviceState, located near the
// enum so it's easier to notice that they need to stay in sync.
namespace {
bool HadSubscriptionEstablishedOnce(MTRInternalDeviceState state)
{
    return state >= MTRInternalDeviceStateInitialSubscriptionEstablished;
}

bool NeedToStartSubscriptionSetup(MTRInternalDeviceState state)
{
    return state <= MTRInternalDeviceStateUnsubscribed;
}

bool HaveSubscriptionEstablishedRightNow(MTRInternalDeviceState state)
{
    return state == MTRInternalDeviceStateInitialSubscriptionEstablished || state == MTRInternalDeviceStateLaterSubscriptionEstablished;
}

NSString * InternalDeviceStateString(MTRInternalDeviceState state)
{
    switch (state) {
    case MTRInternalDeviceStateUnsubscribed:
        return @"Unsubscribed";
    case MTRInternalDeviceStateSubscribing:
        return @"Subscribing";
    case MTRInternalDeviceStateInitialSubscriptionEstablished:
        return @"InitialSubscriptionEstablished";
    case MTRInternalDeviceStateResubscribing:
        return @"Resubscribing";
    case MTRInternalDeviceStateLaterSubscriptionEstablished:
        return @"LaterSubscriptionEstablished";
    default:
        return @"Unknown";
    }
}
} // anonymous namespace

typedef NS_ENUM(NSUInteger, MTRDeviceExpectedValueFieldIndex) {
    MTRDeviceExpectedValueFieldExpirationTimeIndex = 0,
    MTRDeviceExpectedValueFieldValueIndex = 1,
    MTRDeviceExpectedValueFieldIDIndex = 2
};

typedef NS_ENUM(NSUInteger, MTRDeviceReadRequestFieldIndex) {
    MTRDeviceReadRequestFieldPathIndex = 0,
    MTRDeviceReadRequestFieldParamsIndex = 1
};

typedef NS_ENUM(NSUInteger, MTRDeviceWriteRequestFieldIndex) {
    MTRDeviceWriteRequestFieldPathIndex = 0,
    MTRDeviceWriteRequestFieldValueIndex = 1,
    MTRDeviceWriteRequestFieldTimeoutIndex = 2,
    MTRDeviceWriteRequestFieldExpectedValueIDIndex = 3,
};

typedef NS_ENUM(NSUInteger, MTRDeviceWorkItemBatchingID) {
    MTRDeviceWorkItemBatchingReadID = 1,
    MTRDeviceWorkItemBatchingWriteID = 2,
};

typedef NS_ENUM(NSUInteger, MTRDeviceWorkItemDuplicateTypeID) {
    MTRDeviceWorkItemDuplicateReadTypeID = 1,
};

// Minimal time to wait since our last resubscribe failure before we will allow
// a read attempt to prod our subscription.
//
// TODO: Figure out a better value for this, but for now don't allow this to
// happen more often than once every 10 minutes.
#define MTRDEVICE_MIN_RESUBSCRIBE_DUE_TO_READ_INTERVAL_SECONDS (10 * 60)

// Weight of new data in determining subscription latencies.  To avoid random
// outliers causing too much noise in the value, treat an existing value (if
// any) as having 2/3 weight and the new value as having 1/3 weight.  These
// weights are subject to change, if it's determined that different ones give
// better behavior.
#define MTRDEVICE_SUBSCRIPTION_LATENCY_NEW_VALUE_WEIGHT (1.0 / 3.0)

@interface MTRDevice_Concrete ()
// protects against concurrent time updates by guarding timeUpdateScheduled flag which manages time updates scheduling,
// and protects device calls to setUTCTime and setDSTOffset.  This can't just be replaced with "lock", because the time
// update code calls public APIs like readAttributeWithEndpointID:.. (which attempt to take "lock") while holding
// whatever lock protects the time sync bits.
@property (nonatomic, readonly) os_unfair_lock timeSyncLock;

@property (nonatomic) NSMutableArray<NSDictionary<NSString *, id> *> * unreportedEvents;

// The highest event number we have observed, if there was one at all.
@property (nonatomic, readwrite, nullable) NSNumber * highestObservedEventNumber;

// receivingReport is true if we are receving a subscription report.  In
// particular, this will be false if we're just getting an attribute value from
// a read-through.
@property (nonatomic) BOOL receivingReport;

// receivingPrimingReport is true if this subscription report is part of us
// establishing a new subscription to the device.  When this is true, it is
// _not_ guaranteed that any particular set of attributes will be reported
// (e.g. everything could be filtered out by our DataVersion filters).
// Conversely, when this is false that tells us nothing about attributes _not_
// being reported: a device could randomly decide to rev all data versions and
// report all attributes at any point in time, for example due to performing
// subscription resumption.
@property (nonatomic) BOOL receivingPrimingReport;

// TODO: instead of all the BOOL properties that are some facet of the state, move to internal state machine that has (at least):
//   Actively receiving report
//   Actively receiving priming report

@property (nonatomic) MTRInternalDeviceState internalDeviceState;
@property (nonatomic) BOOL doingCASEAttemptForDeviceMayBeReachable;

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

@property (nonatomic) BOOL timeUpdateScheduled;

@property (nonatomic) NSDate * estimatedStartTimeFromGeneralDiagnosticsUpTime;

@property (nonatomic) NSDate * lastDeviceBecameActiveCallbackTime;
@property (nonatomic) BOOL throttlingDeviceBecameActiveCallbacks;

// Keep track of the last time we received subscription related communication from the device
@property (nonatomic, nullable) NSDate * lastSubscriptionActiveTime;

/**
 * If currentReadClient is non-null, that means that we successfully
 * called SendAutoResubscribeRequest on the ReadClient and have not yet gotten
 * an OnDone for that ReadClient.
 */
@property (nonatomic, readonly) MTRDeviceMatterCPPObjectsHolder * matterCPPObjectsHolder;

@end

// Declaring selector so compiler won't complain about testing and calling it in _handleReportEnd
#ifdef DEBUG
@protocol MTRDeviceUnitTestDelegate <MTRDeviceDelegate>
- (void)unitTestReportBeginForDevice:(MTRDevice *)device;
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
- (void)unitTestSubscriptionCallbackDeleteForDevice:(MTRDevice *)device;
- (void)unitTestSubscriptionResetForDevice:(MTRDevice *)device;
@end
#endif

@implementation MTRDevice_Concrete {
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

// synthesize superclass property readwrite accessors
@synthesize queue = _queue;
@synthesize asyncWorkQueue = _asyncWorkQueue;
@synthesize state = _state;
@synthesize estimatedStartTime = _estimatedStartTime;
@synthesize estimatedSubscriptionLatency = _estimatedSubscriptionLatency;
//@synthesize lock = _lock;
//@synthesize persistedClusterData = _persistedClusterData;

- (instancetype)initWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController_Concrete *)controller
{
    // `super` was NSObject, is now MTRDevice.  MTRDevice hides its `init`
    if (self = [super initForSubclassesWithNodeID:nodeID controller:controller]) {
        _timeSyncLock = OS_UNFAIR_LOCK_INIT;
        _descriptionLock = OS_UNFAIR_LOCK_INIT;
        _queue
            = dispatch_queue_create("org.csa-iot.matter.framework.device.workqueue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
        _expectedValueCache = [NSMutableDictionary dictionary];
        _asyncWorkQueue = [[MTRAsyncWorkQueue alloc] initWithContext:self];
        _state = MTRDeviceStateUnknown;
        _internalDeviceState = MTRInternalDeviceStateUnsubscribed;
        _internalDeviceStateForDescription = MTRInternalDeviceStateUnsubscribed;
        _doingCASEAttemptForDeviceMayBeReachable = NO;
        if (controller.controllerDataStore) {
            _persistedClusterData = [[NSCache alloc] init];
        } else {
            _persistedClusterData = nil;
        }
        _clusterDataToPersist = nil;
        _persistedClusters = [NSMutableSet set];
        _highestObservedEventNumber = nil;
        _matterCPPObjectsHolder = [[MTRDeviceMatterCPPObjectsHolder alloc] init];
        _throttlingDeviceBecameActiveCallbacks = NO;

        // If there is a data store, make sure we have an observer to monitor system clock changes, so
        // NSDate-based write coalescing could be reset and not get into a bad state.
        if (_persistedClusterData) {
            mtr_weakify(self);
            _systemTimeChangeObserverToken = [[NSNotificationCenter defaultCenter] addObserverForName:NSSystemClockDidChangeNotification object:nil queue:nil usingBlock:^(NSNotification * _Nonnull notification) {
                mtr_strongify(self);
                VerifyOrReturn(self, MTR_LOG_DEBUG("NSNotificationCenter addObserverForName called back with nil MTRDevice"));

                std::lock_guard lock(self->_lock);
                [self _resetStorageBehaviorState];
            }];
        }

        self.suspended = controller.suspended;

        MTR_LOG_DEBUG("%@ init with hex nodeID 0x%016llX", self, _nodeID.unsignedLongLongValue);
    }
    return self;
}

- (void)dealloc
{
    MTR_LOG("MTRDevice dealloc: %p", self);

    [[NSNotificationCenter defaultCenter] removeObserver:_systemTimeChangeObserverToken];

#ifdef DEBUG
    // Save the first delegate for testing
    __block id testDelegate = nil;
    for (MTRDeviceDelegateInfo * delegateInfo in _delegates) {
        testDelegate = delegateInfo.delegate;
        break;
    }
#endif
    [_delegates removeAllObjects];

    // Delete subscription callback object to tear down ReadClient
    MTRDeviceMatterCPPObjectsHolder * matterCPPObjectsHolder = self.matterCPPObjectsHolder;
    [self._concreteController asyncDispatchToMatterQueue:^{
        [matterCPPObjectsHolder clearReadClientAndDeleteSubscriptionCallback];
#ifdef DEBUG
        // tell test delegate about having completed the deletion
        if ([testDelegate respondsToSelector:@selector(unitTestSubscriptionCallbackDeleteForDevice:)]) {
            [testDelegate unitTestSubscriptionCallbackDeleteForDevice:nil];
        }
#endif
    } errorHandler:nil];

    // Clear this device from subscription pool and persist cached data to storage as needed.
    std::lock_guard lock(_lock);
    [self _clearSubscriptionPoolWork];
    [self _doPersistClusterData];
}

- (NSString *)description
{
    id _Nullable vid;
    id _Nullable pid;
    NSNumber * _Nullable networkFeatures;
    MTRInternalDeviceState internalDeviceState;
    uint32_t lastSubscriptionAttemptWait;
    NSDate * _Nullable mostRecentReportTime;
    NSDate * _Nullable lastSubscriptionFailureTime;
    {
        std::lock_guard lock(_descriptionLock);
        vid = _vid;
        pid = _pid;
        networkFeatures = _allNetworkFeatures;
        internalDeviceState = _internalDeviceStateForDescription;
        lastSubscriptionAttemptWait = _lastSubscriptionAttemptWaitForDescription;
        mostRecentReportTime = _mostRecentReportTimeForDescription;
        lastSubscriptionFailureTime = _lastSubscriptionFailureTimeForDescription;
    }

    if (vid == nil) {
        vid = @"Unknown";
    }

    if (pid == nil) {
        pid = @"Unknown";
    }

    NSString * wifi;
    NSString * thread;
    if (networkFeatures == nil) {
        wifi = @"NO";
        thread = @"NO";
    } else {
        wifi = MTR_YES_NO(networkFeatures.unsignedLongLongValue & MTRNetworkCommissioningFeatureWiFiNetworkInterface);
        thread = MTR_YES_NO(networkFeatures.unsignedLongLongValue & MTRNetworkCommissioningFeatureThreadNetworkInterface);
    }

    NSString * reportAge;
    if (mostRecentReportTime) {
        reportAge = [NSString stringWithFormat:@" (%.0lfs ago)", -[mostRecentReportTime timeIntervalSinceNow]];
    } else {
        reportAge = @"";
    }

    NSString * subscriptionFailureAge;
    if (lastSubscriptionFailureTime) {
        subscriptionFailureAge = [NSString stringWithFormat:@" (%.0lfs ago)", -[lastSubscriptionFailureTime timeIntervalSinceNow]];
    } else {
        subscriptionFailureAge = @"";
    }

    return [NSString
        stringWithFormat:@"<%@: %p, node: %016llX-%016llX (%llu), VID: %@, PID: %@, WiFi: %@, Thread: %@, state: %@, last subscription attempt wait: %lus, queued work: %lu, last report: %@%@, last subscription failure: %@%@, controller: %@>", NSStringFromClass(self.class), self, _deviceController.compressedFabricID.unsignedLongLongValue, _nodeID.unsignedLongLongValue, _nodeID.unsignedLongLongValue, vid, pid, wifi, thread, InternalDeviceStateString(internalDeviceState), static_cast<unsigned long>(lastSubscriptionAttemptWait), static_cast<unsigned long>(_asyncWorkQueue.itemCount), mostRecentReportTime, reportAge, lastSubscriptionFailureTime, subscriptionFailureAge, _deviceController.uniqueIdentifier];
}

- (NSDictionary *)_internalProperties
{
    NSMutableDictionary * properties = [NSMutableDictionary dictionary];
    {
        std::lock_guard lock(_descriptionLock);

        MTR_OPTIONAL_ATTRIBUTE(kMTRDeviceInternalPropertyKeyVendorID, _vid, properties);
        MTR_OPTIONAL_ATTRIBUTE(kMTRDeviceInternalPropertyKeyProductID, _pid, properties);
        MTR_OPTIONAL_ATTRIBUTE(kMTRDeviceInternalPropertyNetworkFeatures, _allNetworkFeatures, properties);
        MTR_OPTIONAL_ATTRIBUTE(kMTRDeviceInternalPropertyMostRecentReportTime, _mostRecentReportTimeForDescription, properties);
    }

    {
        std::lock_guard lock(_lock);
        MTR_OPTIONAL_ATTRIBUTE(kMTRDeviceInternalPropertyDeviceInternalState, [NSNumber numberWithUnsignedInteger:_internalDeviceState], properties);
        MTR_OPTIONAL_ATTRIBUTE(kMTRDeviceInternalPropertyLastSubscriptionAttemptWait, [NSNumber numberWithUnsignedInt:_lastSubscriptionAttemptWait], properties);
        MTR_OPTIONAL_ATTRIBUTE(kMTRDeviceInternalPropertyLastSubscriptionFailureTime, _lastSubscriptionFailureTime, properties);

        MTR_OPTIONAL_ATTRIBUTE(kMTRDeviceInternalPropertyDeviceState, @(_state), properties);
        MTR_OPTIONAL_ATTRIBUTE(kMTRDeviceInternalPropertyDeviceCachePrimed, @(_deviceCachePrimed), properties);
        MTR_OPTIONAL_ATTRIBUTE(kMTRDeviceInternalPropertyEstimatedStartTime, _estimatedStartTime, properties);
        MTR_OPTIONAL_ATTRIBUTE(kMTRDeviceInternalPropertyEstimatedSubscriptionLatency, _estimatedSubscriptionLatency, properties);
    }

    return properties;
}

- (nullable NSNumber *)vendorID
{
    std::lock_guard lock(_descriptionLock);
    return [_vid copy];
}

- (nullable NSNumber *)productID
{
    std::lock_guard lock(_descriptionLock);
    return [_pid copy];
}

- (MTRNetworkCommissioningFeature)networkCommissioningFeatures
{
    std::lock_guard lock(_descriptionLock);
    return [_allNetworkFeatures unsignedIntValue];
}

- (void)_notifyDelegateOfPrivateInternalPropertiesChanges
{
    os_unfair_lock_assert_owner(&self->_lock);
    [self _callDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
        if ([delegate respondsToSelector:@selector(device:internalStateUpdated:)]) {
            [delegate performSelector:@selector(device:internalStateUpdated:) withObject:self withObject:[self _internalProperties]];
        }
    }];
}

#pragma mark - Time Synchronization

- (void)_setTimeOnDevice
{
    NSDate * now = [NSDate date];
    // If no date available, error
    if (!now) {
        MTR_LOG_ERROR("%@ Could not retrieve current date. Unable to setUTCTime on endpoints.", self);
        return;
    }

    uint64_t matterEpochTimeMicroseconds = 0;
    if (!DateToMatterEpochMicroseconds(now, matterEpochTimeMicroseconds)) {
        MTR_LOG_ERROR("%@ Could not convert NSDate (%@) to Matter Epoch Time. Unable to setUTCTime on endpoints.", self, now);
        return;
    }

    // Set Time on each Endpoint with a Time Synchronization Cluster Server
    NSArray<NSNumber *> * endpointsToSync = [self _endpointsWithTimeSyncClusterServer];
    for (NSNumber * endpoint in endpointsToSync) {
        MTR_LOG_DEBUG("%@ Setting Time on Endpoint %@", self, endpoint);
        [self _setUTCTime:matterEpochTimeMicroseconds withGranularity:MTRTimeSynchronizationGranularityMicrosecondsGranularity forEndpoint:endpoint];

        // Check how many DST offsets this endpoint supports.
        auto dstOffsetsMaxSizePath = [MTRAttributePath attributePathWithEndpointID:endpoint clusterID:@(MTRClusterIDTypeTimeSynchronizationID) attributeID:@(MTRAttributeIDTypeClusterTimeSynchronizationAttributeDSTOffsetListMaxSizeID)];
        auto dstOffsetsMaxSize = [self readAttributeWithEndpointID:dstOffsetsMaxSizePath.endpoint clusterID:dstOffsetsMaxSizePath.cluster attributeID:dstOffsetsMaxSizePath.attribute params:nil];
        if (dstOffsetsMaxSize == nil) {
            // This endpoint does not support TZ, so won't support SetDSTOffset.
            MTR_LOG("%@ Unable to SetDSTOffset on endpoint %@, since it does not support the TZ feature", self, endpoint);
            continue;
        }
        auto attrReport = [[MTRAttributeReport alloc] initWithResponseValue:@{
            MTRAttributePathKey : dstOffsetsMaxSizePath,
            MTRDataKey : dstOffsetsMaxSize,
        }
                                                                      error:nil];
        uint8_t maxOffsetCount;
        if (attrReport == nil) {
            MTR_LOG_ERROR("%@ DSTOffsetListMaxSize value on endpoint %@ is invalid. Defaulting to 1.", self, endpoint);
            maxOffsetCount = 1;
        } else {
            NSNumber * maxOffsetCountAsNumber = attrReport.value;
            maxOffsetCount = maxOffsetCountAsNumber.unsignedCharValue;
            if (maxOffsetCount == 0) {
                MTR_LOG_ERROR("%@ DSTOffsetListMaxSize value on endpoint %@ is 0, which is not allowed. Defaulting to 1.", self, endpoint);
                maxOffsetCount = 1;
            }
        }
        auto * dstOffsets = MTRComputeDSTOffsets(maxOffsetCount);
        if (dstOffsets == nil) {
            MTR_LOG_ERROR("%@ Could not retrieve DST offset information. Unable to setDSTOffset on endpoint %@.", self, endpoint);
            continue;
        }

        [self _setDSTOffsets:dstOffsets forEndpoint:endpoint];
    }
}

- (void)_scheduleNextUpdate:(UInt64)nextUpdateInSeconds
{
    mtr_weakify(self);
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (nextUpdateInSeconds * NSEC_PER_SEC)), self.queue, ^{
        mtr_strongify(self);
        MTR_LOG_DEBUG("%@ Timer expired, start Device Time Update", self);
        if (self) {
            [self _performScheduledTimeUpdate];
        } else {
            MTR_LOG_DEBUG("%@ MTRDevice no longer valid. No Timer Scheduled will be scheduled for a Device Time Update.", self);
            return;
        }
    });
    self.timeUpdateScheduled = YES;
    MTR_LOG_DEBUG("%@ Timer Scheduled for next Device Time Update, in %llu seconds", self, nextUpdateInSeconds);
}

// Time Updates are a day apart (this can be changed in the future)
#define MTR_DEVICE_TIME_UPDATE_DEFAULT_WAIT_TIME_SEC (24 * 60 * 60)
// assume lock is held
- (void)_updateDeviceTimeAndScheduleNextUpdate
{
    os_unfair_lock_assert_owner(&self->_timeSyncLock);
    if (self.timeUpdateScheduled) {
        MTR_LOG_DEBUG("%@ Device Time Update already scheduled", self);
        return;
    }

    [self _setTimeOnDevice];
    [self _scheduleNextUpdate:MTR_DEVICE_TIME_UPDATE_DEFAULT_WAIT_TIME_SEC];
}

- (void)_performScheduledTimeUpdate
{
    std::lock_guard lock(_timeSyncLock);
    // Device needs to still be reachable
    if (self.state != MTRDeviceStateReachable) {
        MTR_LOG_DEBUG("%@ Device is not reachable, canceling Device Time Updates.", self);
        return;
    }
    // Device must not be invalidated
    if (!self.timeUpdateScheduled) {
        MTR_LOG_DEBUG("%@ Device Time Update is no longer scheduled, MTRDevice may have been invalidated.", self);
        return;
    }
    self.timeUpdateScheduled = NO;
    [self _updateDeviceTimeAndScheduleNextUpdate];
}

- (NSArray<NSNumber *> *)_endpointsWithTimeSyncClusterServer
{
    NSArray<NSNumber *> * endpointsOnDevice;
    {
        std::lock_guard lock(_lock);
        endpointsOnDevice = [self _endpointList];
    }

    NSMutableArray<NSNumber *> * endpointsWithTimeSyncCluster = [[NSMutableArray<NSNumber *> alloc] init];
    for (NSNumber * endpoint in endpointsOnDevice) {
        // Get list of server clusters on endpoint
        auto clusterList = [self readAttributeWithEndpointID:endpoint clusterID:@(MTRClusterIDTypeDescriptorID) attributeID:@(MTRAttributeIDTypeClusterDescriptorAttributeServerListID) params:nil];
        NSArray<NSNumber *> * clusterArray = [self arrayOfNumbersFromAttributeValue:clusterList];

        if (clusterArray && [clusterArray containsObject:@(MTRClusterIDTypeTimeSynchronizationID)]) {
            [endpointsWithTimeSyncCluster addObject:endpoint];
        }
    }
    MTR_LOG_DEBUG("%@ Device has following endpoints with Time Sync Cluster Server: %@", self, endpointsWithTimeSyncCluster);
    return endpointsWithTimeSyncCluster;
}

- (void)_setUTCTime:(UInt64)matterEpochTime withGranularity:(uint8_t)granularity forEndpoint:(NSNumber *)endpoint
{
    MTR_LOG_DEBUG(" %@ _setUTCTime with matterEpochTime: %llu, endpoint %@", self, matterEpochTime, endpoint);
    MTRTimeSynchronizationClusterSetUTCTimeParams * params = [[MTRTimeSynchronizationClusterSetUTCTimeParams
        alloc] init];
    params.utcTime = @(matterEpochTime);
    params.granularity = @(granularity);
    mtr_weakify(self);
    auto setUTCTimeResponseHandler = ^(id _Nullable response, NSError * _Nullable error) {
        mtr_strongify(self);
        if (error) {
            MTR_LOG_ERROR("%@ _setUTCTime failed on endpoint %@, with parameters %@, error: %@", self, endpoint, params, error);
        }
    };

    [self _invokeKnownCommandWithEndpointID:endpoint
                                  clusterID:@(MTRClusterIDTypeTimeSynchronizationID)
                                  commandID:@(MTRCommandIDTypeClusterTimeSynchronizationCommandSetUTCTimeID)
                             commandPayload:params
                             expectedValues:nil
                      expectedValueInterval:nil
                         timedInvokeTimeout:nil
                serverSideProcessingTimeout:params.serverSideProcessingTimeout
                              responseClass:nil
                                      queue:self.queue
                                 completion:setUTCTimeResponseHandler];
}

- (void)_setDSTOffsets:(NSArray<MTRTimeSynchronizationClusterDSTOffsetStruct *> *)dstOffsets forEndpoint:(NSNumber *)endpoint
{
    MTR_LOG_DEBUG("%@ _setDSTOffsets with offsets: %@, endpoint %@",
        self, dstOffsets, endpoint);

    MTRTimeSynchronizationClusterSetDSTOffsetParams * params = [[MTRTimeSynchronizationClusterSetDSTOffsetParams
        alloc] init];
    params.dstOffset = dstOffsets;

    mtr_weakify(self);
    auto setDSTOffsetResponseHandler = ^(id _Nullable response, NSError * _Nullable error) {
        mtr_strongify(self);
        if (error) {
            MTR_LOG_ERROR("%@ _setDSTOffsets failed on endpoint %@, with parameters %@, error: %@", self, endpoint, params, error);
        }
    };

    [self _invokeKnownCommandWithEndpointID:endpoint
                                  clusterID:@(MTRClusterIDTypeTimeSynchronizationID)
                                  commandID:@(MTRCommandIDTypeClusterTimeSynchronizationCommandSetDSTOffsetID)
                             commandPayload:params
                             expectedValues:nil
                      expectedValueInterval:nil
                         timedInvokeTimeout:nil
                serverSideProcessingTimeout:params.serverSideProcessingTimeout
                              responseClass:nil
                                      queue:self.queue
                                 completion:setDSTOffsetResponseHandler];
}

- (NSMutableArray<NSNumber *> *)arrayOfNumbersFromAttributeValue:(MTRDeviceDataValueDictionary)dataDictionary
{
    if (![MTRArrayValueType isEqual:dataDictionary[MTRTypeKey]]) {
        return nil;
    }

    id value = dataDictionary[MTRValueKey];
    if (![value isKindOfClass:NSArray.class]) {
        return nil;
    }

    NSArray * valueArray = value;
    __auto_type outputArray = [NSMutableArray<NSNumber *> arrayWithCapacity:valueArray.count];

    for (id item in valueArray) {
        if (![item isKindOfClass:NSDictionary.class]) {
            return nil;
        }

        NSDictionary * itemDictionary = item;
        id data = itemDictionary[MTRDataKey];
        if (![data isKindOfClass:NSDictionary.class]) {
            return nil;
        }

        NSDictionary * dataDictionary = data;
        id dataType = dataDictionary[MTRTypeKey];
        id dataValue = dataDictionary[MTRValueKey];
        if (![dataType isKindOfClass:NSString.class] || ![dataValue isKindOfClass:NSNumber.class]) {
            return nil;
        }
        [outputArray addObject:dataValue];
    }
    return outputArray;
}

#pragma mark Subscription and delegate handling

// subscription intervals are in seconds
#define MTR_DEVICE_SUBSCRIPTION_MAX_INTERVAL_MIN (10 * 60) // 10 minutes (for now)
#define MTR_DEVICE_SUBSCRIPTION_MAX_INTERVAL_MAX (60 * 60) // 60 minutes
#define MTR_DEVICE_MIN_SECONDS_BETWEEN_DEVICE_BECAME_ACTIVE_CALLBACKS (1 * 60) // 1 minute (for now)

- (BOOL)_subscriptionsAllowed
{
    os_unfair_lock_assert_owner(&self->_lock);

    // We should not allow a subscription when we are suspended or for device controllers over XPC.
    return self.suspended == NO && ![_deviceController isKindOfClass:MTRDeviceControllerOverXPC.class];
}

- (void)_delegateAdded:(id<MTRDeviceDelegate>)delegate
{
    os_unfair_lock_assert_owner(&self->_lock);

    [super _delegateAdded:delegate];

    [self _ensureSubscriptionForExistingDelegates:@"delegate is set"];
}

- (void)_ensureSubscriptionForExistingDelegates:(NSString *)reason
{
    os_unfair_lock_assert_owner(&self->_lock);

    __block BOOL shouldSetUpSubscription = [self _subscriptionsAllowed];

    // For unit testing only. If this ever changes to not being for unit testing purposes,
    // we would need to move the code outside of where we acquire the lock above.
#ifdef DEBUG
    [self _callFirstDelegateSynchronouslyWithBlock:^(id testDelegate) {
        if ([testDelegate respondsToSelector:@selector(unitTestShouldSetUpSubscriptionForDevice:)]) {
            shouldSetUpSubscription = [testDelegate unitTestShouldSetUpSubscriptionForDevice:self];
        }
    }];
#endif

    if (shouldSetUpSubscription) {
        MTR_LOG("%@ - starting subscription setup", self);
        // Record the time of first addDelegate call that triggers initial subscribe, and do not reset this value on subsequent addDelegate calls
        if (!_initialSubscribeStart) {
            _initialSubscribeStart = [NSDate now];
        }
        mtr_weakify(self);
        if ([self _deviceUsesThread]) {
            MTR_LOG(" => %@ - device is a thread device, scheduling in pool", self);
            NSString * description = [NSString stringWithFormat:@"MTRDevice setDelegate first subscription / controller resume (%p)", self];
            [self _scheduleSubscriptionPoolWork:^{
                mtr_strongify(self);
                VerifyOrReturn(self, MTR_LOG_DEBUG("_ensureSubscriptionForExistingDelegates _scheduleSubscriptionPoolWork called back with nil MTRDevice"));

                [self->_deviceController asyncDispatchToMatterQueue:^{
                    mtr_strongify(self);
                    VerifyOrReturn(self, MTR_LOG_DEBUG("_ensureSubscriptionForExistingDelegates asyncDispatchToMatterQueue called back with nil MTRDevice"));

                    std::lock_guard lock(self->_lock);
                    [self _setupSubscriptionWithReason:[NSString stringWithFormat:@"%@ and scheduled subscription is happening", reason]];
                } errorHandler:^(NSError * _Nonnull error) {
                    mtr_strongify(self);
                    VerifyOrReturn(self, MTR_LOG_DEBUG("_ensureSubscriptionForExistingDelegates asyncDispatchToMatterQueue errored with nil MTRDevice"));

                    // If controller is not running, clear work item from the subscription queue
                    MTR_LOG_ERROR("%@ could not dispatch to matter queue for resubscription - error %@", self, error);
                    std::lock_guard lock(self->_lock);
                    [self _clearSubscriptionPoolWork];
                }];
            } inNanoseconds:0 description:description];
        } else {
            [_deviceController asyncDispatchToMatterQueue:^{
                mtr_strongify(self);
                VerifyOrReturn(self, MTR_LOG_DEBUG("_ensureSubscriptionForExistingDelegates asyncDispatchToMatterQueue called back with nil MTRDevice"));

                std::lock_guard lock(self->_lock);
                [self _setupSubscriptionWithReason:[NSString stringWithFormat:@"%@ and subscription is needed", reason]];
            } errorHandler:nil];
        }
    }
}

- (void)invalidate
{
    MTR_LOG("%@ invalidate", self);

    [_asyncWorkQueue invalidate];

    os_unfair_lock_lock(&self->_timeSyncLock);
    _timeUpdateScheduled = NO;
    os_unfair_lock_unlock(&self->_timeSyncLock);

    os_unfair_lock_lock(&self->_lock);

    // Flush unstored attributes if any
    [self _persistClusterData];

    _state = MTRDeviceStateUnknown;

    // Make sure we don't try to resubscribe if we have a pending resubscribe
    // attempt, since we now have no delegate.
    _reattemptingSubscription = NO;

    // Clear subscription pool work item if it's in progress, to avoid forever
    // taking up a slot in the controller's work queue.
    [self _clearSubscriptionPoolWork];

    mtr_weakify(self);
    [_deviceController asyncDispatchToMatterQueue:^{
        mtr_strongify(self);
        VerifyOrReturn(self, MTR_LOG_DEBUG("invalidate asyncDispatchToMatterQueue called back with nil MTRDevice"));

        MTR_LOG("%@ invalidate disconnecting ReadClient and SubscriptionCallback", self);

        // Destroy the read client and callback (has to happen on the Matter
        // queue, to avoid deleting objects that are being referenced), to
        // tear down the subscription.  We will get no more callbacks from
        // the subscription after this point.
        std::lock_guard lock(self->_lock);
        [self _resetSubscription];
    }
                                     errorHandler:nil];

    [self _stopConnectivityMonitoring];

    os_unfair_lock_unlock(&self->_lock);

    [super invalidate];
}

- (void)nodeMayBeAdvertisingOperational
{
    assertChipStackLockedByCurrentThread();

    MTR_LOG("%@ saw new operational advertisement", self);

    [self _triggerResubscribeWithReason:@"operational advertisement seen"
                    nodeLikelyReachable:YES];
}

// Trigger a resubscribe as needed.  nodeLikelyReachable should be YES if we
// have reason to suspect the node is now reachable, NO if we have no idea
// whether it might be.
- (void)_triggerResubscribeWithReason:(NSString *)reason nodeLikelyReachable:(BOOL)nodeLikelyReachable
{
    MTR_LOG("%@ _triggerResubscribeWithReason called with reason %@", self, reason);
    assertChipStackLockedByCurrentThread();

    // We might want to trigger a resubscribe on our existing ReadClient.  Do
    // that outside the scope of our lock, so we're not calling arbitrary code
    // we don't control with the lock held.  This is safe, because we are
    // running on he Matter queue and the ReadClient can't get destroyed while
    // we are on that queue.
    ReadClient * readClientToResubscribe = nullptr;
    SubscriptionCallback * subscriptionCallback = nullptr;

    os_unfair_lock_lock(&self->_lock);

    // Don't change state to MTRDeviceStateReachable, since the device might not
    // in fact be reachable yet; we won't know until we have managed to
    // establish a CASE session.  And at that point, our subscription will
    // trigger the state change as needed.
    BOOL shouldReattemptSubscription = NO;
    if (self.reattemptingSubscription) {
        shouldReattemptSubscription = YES;
    } else {
        readClientToResubscribe = self.matterCPPObjectsHolder.readClient;
        subscriptionCallback = self.matterCPPObjectsHolder.subscriptionCallback;
    }
    os_unfair_lock_unlock(&self->_lock);

    if (readClientToResubscribe) {
        if (nodeLikelyReachable) {
            // If we have reason to suspect the node is now reachable, reset the
            // backoff timer, so that if this attempt fails we'll try again
            // quickly; it's possible we'll just catch the node at a bad time
            // here (e.g. still booting up), but should try again reasonably quickly.
            subscriptionCallback->ResetResubscriptionBackoff();
        }
        readClientToResubscribe->TriggerResubscribeIfScheduled(reason.UTF8String);
    } else if (((_internalDeviceState == MTRInternalDeviceStateSubscribing && !self.doingCASEAttemptForDeviceMayBeReachable) || shouldReattemptSubscription) && nodeLikelyReachable) {
        // If we have reason to suspect that the node is now reachable and we haven't established a
        // CASE session yet, let's consider it to be stalled and invalidate the pairing session.

        // Reset back off for framework resubscription
        os_unfair_lock_lock(&self->_lock);
        [self _setLastSubscriptionAttemptWait:0];
        os_unfair_lock_unlock(&self->_lock);

        mtr_weakify(self);
        [self._concreteController asyncGetCommissionerOnMatterQueue:^(Controller::DeviceCommissioner * commissioner) {
            mtr_strongify(self);
            VerifyOrReturn(self, MTR_LOG_DEBUG("_triggerResubscribeWithReason asyncGetCommissionerOnMatterQueue called back with nil MTRDevice"));

            auto caseSessionMgr = commissioner->CASESessionMgr();
            VerifyOrDie(caseSessionMgr != nullptr);
            caseSessionMgr->ReleaseSession(commissioner->GetPeerScopedId(self->_nodeID.unsignedLongLongValue));
        } errorHandler:nil /* not much we can do */];
    }

    // The subscription reattempt here eventually asyncs onto the matter queue for session,
    // and should be called after the above ReleaseSession call, to avoid churn.
    if (shouldReattemptSubscription) {
        std::lock_guard lock(_lock);
        [self _reattemptSubscriptionNowIfNeededWithReason:reason];
    }
}

// Return YES if we are in a state where, apart from communication issues with
// the device, we will be able to get reports via our subscription.
- (BOOL)_subscriptionAbleToReport
{
    std::lock_guard lock(_lock);
    if (![self _delegateExists]) {
        // No delegate definitely means no subscription.
        return NO;
    }

    // For unit testing only, matching logic in setDelegate
#ifdef DEBUG
    __block BOOL useTestDelegateOverride = NO;
    __block BOOL testDelegateShouldSetUpSubscriptionForDevice = NO;
    [self _callFirstDelegateSynchronouslyWithBlock:^(id testDelegate) {
        if ([testDelegate respondsToSelector:@selector(unitTestShouldSetUpSubscriptionForDevice:)]) {
            useTestDelegateOverride = YES;
            testDelegateShouldSetUpSubscriptionForDevice = [testDelegate unitTestShouldSetUpSubscriptionForDevice:self];
        }
    }];
    if (useTestDelegateOverride && !testDelegateShouldSetUpSubscriptionForDevice) {
        return NO;
    }

#endif

    // Subscriptions are not able to report if they are not allowed.
    return [self _subscriptionsAllowed];
}

// Notification that read-through was skipped for an attribute read.
- (void)_readThroughSkipped
{
    std::lock_guard lock(_lock);
    if (_state == MTRDeviceStateReachable) {
        // We're getting reports from the device, so there's nothing else to be
        // done here.  We could skip this check, because our "try to
        // resubscribe" code would be a no-op in this case, but then we'd have
        // an extra dispatch in the common case of read-while-subscribed, which
        // is not great for peformance.
        return;
    }

    if (_lastSubscriptionFailureTime == nil) {
        // No need to try to do anything here, because we have never failed a
        // subscription attempt (so we might be in the middle of one now, and no
        // need to prod things along).
        return;
    }

    if ([[NSDate now] timeIntervalSinceDate:_lastSubscriptionFailureTime] < MTRDEVICE_MIN_RESUBSCRIBE_DUE_TO_READ_INTERVAL_SECONDS) {
        // Not enough time has passed since we last tried.  Don't create extra
        // network traffic.
        //
        // TODO: Do we need to worry about this being too spammy in the log if
        // we keep getting reads while not subscribed?  We could add another
        // backoff timer or counter for the log line...
        MTR_LOG_DEBUG("%@ skipping resubscribe from skipped read-through: not enough time has passed since %@", self, _lastSubscriptionFailureTime);
        return;
    }

    // Do the remaining work on the Matter queue, because we may want to touch
    // ReadClient in there.  If the dispatch fails, that's fine; it means our
    // controller has shut down, so nothing to be done.
    mtr_weakify(self);
    [_deviceController asyncDispatchToMatterQueue:^{
        mtr_strongify(self);
        VerifyOrReturn(self, MTR_LOG_DEBUG("_readThroughSkipped asyncDispatchToMatterQueue called back with nil MTRDevice"));

        [self _triggerResubscribeWithReason:@"read-through skipped while not subscribed" nodeLikelyReachable:NO];
    }
                                     errorHandler:nil];
}

- (void)_callDelegateDeviceCachePrimed
{
    os_unfair_lock_assert_owner(&self->_lock);
    [self _callDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
        if ([delegate respondsToSelector:@selector(deviceCachePrimed:)]) {
            [delegate deviceCachePrimed:self];
        }
    }];
    [self _notifyDelegateOfPrivateInternalPropertiesChanges];
}

// assume lock is held
- (void)_changeState:(MTRDeviceState)state
{
    // We want to avoid situations where something changes our state and then an
    // async block that was queued earlier in response to something changes it
    // again, to a value that no longer makes sense. To avoid that:
    //
    // 1) All state changes happen on the Matter queue.
    // 2) All state changes happen synchronously with the event that actually
    //    triggers the state change.
    assertChipStackLockedByCurrentThread();

    os_unfair_lock_assert_owner(&self->_lock);
    MTRDeviceState lastState = _state;
    _state = state;
    if (lastState != state) {
        if (state != MTRDeviceStateReachable) {
            MTR_LOG("%@ reachability state change %lu => %lu, set estimated start time to nil", self, static_cast<unsigned long>(lastState),
                static_cast<unsigned long>(state));
            _estimatedStartTime = nil;
            _estimatedStartTimeFromGeneralDiagnosticsUpTime = nil;
        } else {
            MTR_LOG(
                "%@ reachability state change %lu => %lu", self, static_cast<unsigned long>(lastState), static_cast<unsigned long>(state));
        }
        [self _callDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
            [delegate device:self stateChanged:state];
        }];
        [self _notifyDelegateOfPrivateInternalPropertiesChanges];
    } else {
        MTR_LOG(
            "%@ Not reporting reachability state change, since no change in state %lu => %lu", self, static_cast<unsigned long>(lastState), static_cast<unsigned long>(state));
    }
}

- (void)_changeInternalState:(MTRInternalDeviceState)state
{
    // We want to avoid situations where something changes our state and then an
    // async block that was queued earlier in response to something changes it
    // again, to a value that no longer makes sense. To avoid that:
    //
    // 1) All state changes happen on the Matter queue.
    // 2) All state changes happen synchronously with the event that actually
    //    triggers the state change.
    assertChipStackLockedByCurrentThread();

    os_unfair_lock_assert_owner(&self->_lock);
    MTRInternalDeviceState lastState = _internalDeviceState;
    _internalDeviceState = state;
    {
        std::lock_guard lock(_descriptionLock);
        _internalDeviceStateForDescription = _internalDeviceState;
    }
    if (lastState != state) {
        MTR_LOG("%@ internal state change %lu => %lu", self, static_cast<unsigned long>(lastState), static_cast<unsigned long>(state));

        /* BEGIN DRAGONS: This is a huge hack for a specific use case, do not rename, remove or modify behavior here */
        // TODO: This should only be called for thread devices
        [self _callDelegatesWithBlock:^(id delegate) {
            if ([delegate respondsToSelector:@selector(_deviceInternalStateChanged:)]) {
                [delegate _deviceInternalStateChanged:self];
            }
        }];
        /* END DRAGONS */

        [self _notifyDelegateOfPrivateInternalPropertiesChanges];
    }
}

#ifdef DEBUG
- (MTRInternalDeviceState)_getInternalState
{
    std::lock_guard lock(self->_lock);
    return _internalDeviceState;
}
#endif

// First Time Sync happens 2 minutes after reachability (this can be changed in the future)
#define MTR_DEVICE_TIME_UPDATE_INITIAL_WAIT_TIME_SEC (60 * 2)
- (void)_handleSubscriptionEstablished
{
    os_unfair_lock_lock(&self->_lock);

    // If subscription had reset since this handler was scheduled, do not execute "established" logic below
    if (!HaveSubscriptionEstablishedRightNow(_internalDeviceState)) {
        MTR_LOG("%@ _handleSubscriptionEstablished run with internal state %lu - skipping subscription establishment logic", self, static_cast<unsigned long>(_internalDeviceState));
        os_unfair_lock_unlock(&self->_lock);
        return;
    }

    // We have completed the subscription work - remove from the subscription pool.
    [self _clearSubscriptionPoolWork];

    // No need to monitor connectivity after subscription establishment
    [self _stopConnectivityMonitoring];

    // reset subscription attempt wait time when subscription succeeds
    [self _setLastSubscriptionAttemptWait:0];

    auto initialSubscribeStart = _initialSubscribeStart;
    // We no longer need to track subscribe latency for this device.
    _initialSubscribeStart = nil;

    if (initialSubscribeStart != nil) {
        // We want time interval from initialSubscribeStart to now, not the other
        // way around.
        NSTimeInterval subscriptionLatency = -[initialSubscribeStart timeIntervalSinceNow];
        if (_estimatedSubscriptionLatency == nil) {
            _estimatedSubscriptionLatency = @(subscriptionLatency);
        } else {
            NSTimeInterval newSubscriptionLatencyEstimate = MTRDEVICE_SUBSCRIPTION_LATENCY_NEW_VALUE_WEIGHT * subscriptionLatency + (1 - MTRDEVICE_SUBSCRIPTION_LATENCY_NEW_VALUE_WEIGHT) * _estimatedSubscriptionLatency.doubleValue;
            _estimatedSubscriptionLatency = @(newSubscriptionLatencyEstimate);
        }
        [self _storePersistedDeviceData];
    }

    os_unfair_lock_unlock(&self->_lock);

    os_unfair_lock_lock(&self->_timeSyncLock);

    if (!self.timeUpdateScheduled) {
        [self _scheduleNextUpdate:MTR_DEVICE_TIME_UPDATE_INITIAL_WAIT_TIME_SEC];
    }

    os_unfair_lock_unlock(&self->_timeSyncLock);
}

- (void)_handleSubscriptionError:(NSError *)error
{
    assertChipStackLockedByCurrentThread();

    std::lock_guard lock(_lock);
    [self _doHandleSubscriptionError:error];
}

- (void)_doHandleSubscriptionError:(nullable NSError *)error
{
    assertChipStackLockedByCurrentThread();

    os_unfair_lock_assert_owner(&_lock);

    [self _changeInternalState:MTRInternalDeviceStateUnsubscribed];
    _unreportedEvents = nil;

    [self _changeState:MTRDeviceStateUnreachable];
}

- (BOOL)deviceUsesThread
{
    std::lock_guard lock(_lock);
    return [self _deviceUsesThread];
}

// This method is used for signaling whether to use the subscription pool. This functions as
// a heuristic for whether to throttle subscriptions to the device via a pool of subscriptions.
// If products appear that have both Thread and Wifi enabled but are primarily on wifi, this
// method will need to be updated to reflect that.
- (BOOL)_deviceUsesThread
{
    os_unfair_lock_assert_owner(&self->_lock);

#ifdef DEBUG
    // Note: This is a hack to allow our unit tests to test the subscription pooling behavior we have implemented for thread, so we mock devices to be a thread device
    __block BOOL pretendThreadEnabled = NO;
    [self _callFirstDelegateSynchronouslyWithBlock:^(id testDelegate) {
        if ([testDelegate respondsToSelector:@selector(unitTestPretendThreadEnabled:)]) {
            pretendThreadEnabled = [testDelegate unitTestPretendThreadEnabled:self];
        }
    }];
    if (pretendThreadEnabled) {
        return YES;
    }
#endif

    MTRClusterPath * networkCommissioningClusterPath = [MTRClusterPath clusterPathWithEndpointID:@(kRootEndpointId) clusterID:@(MTRClusterIDTypeNetworkCommissioningID)];
    MTRDeviceClusterData * networkCommissioningClusterData = [self _clusterDataForPath:networkCommissioningClusterPath];
    NSNumber * networkCommissioningClusterFeatureMapValueNumber = networkCommissioningClusterData.attributes[@(MTRClusterGlobalAttributeFeatureMapID)][MTRValueKey];

    if (networkCommissioningClusterFeatureMapValueNumber == nil)
        return NO;
    if (![networkCommissioningClusterFeatureMapValueNumber isKindOfClass:[NSNumber class]]) {
        MTR_LOG_ERROR("%@ Unexpected NetworkCommissioning FeatureMap value %@", self, networkCommissioningClusterFeatureMapValueNumber);
        return NO;
    }

    uint32_t networkCommissioningClusterFeatureMapValue = static_cast<uint32_t>(networkCommissioningClusterFeatureMapValueNumber.unsignedLongValue);

    return (networkCommissioningClusterFeatureMapValue & MTRNetworkCommissioningFeatureThreadNetworkInterface) != 0 ? YES : NO;
}

- (void)_clearSubscriptionPoolWork
{
    os_unfair_lock_assert_owner(&self->_lock);
    MTRAsyncWorkCompletionBlock completion = self->_subscriptionPoolWorkCompletionBlock;
    if (completion) {
#ifdef DEBUG
        [self _callDelegatesWithBlock:^(id testDelegate) {
            if ([testDelegate respondsToSelector:@selector(unitTestSubscriptionPoolWorkComplete:)]) {
                [testDelegate unitTestSubscriptionPoolWorkComplete:self];
            }
        }];
#endif
        self->_subscriptionPoolWorkCompletionBlock = nil;
        completion(MTRAsyncWorkComplete);
    }
}

- (void)_scheduleSubscriptionPoolWork:(dispatch_block_t)workBlock inNanoseconds:(int64_t)inNanoseconds description:(NSString *)description
{
    os_unfair_lock_assert_owner(&self->_lock);

    // Sanity check we are not scheduling for this device multiple times in the pool
    if (_subscriptionPoolWorkCompletionBlock) {
        MTR_LOG("%@ already scheduled in subscription pool for this device - ignoring: %@", self, description);
        return;
    }

    mtr_weakify(self);
    dispatch_block_t workBlockToQueue = ^{
        mtr_strongify(self);
        // This block may be delayed by a specified number of nanoseconds, potentially running after the device is deallocated.
        // If so, MTRAsyncWorkItem::initWithQueue will assert on a nil queue, which will cause a crash.
        VerifyOrReturn(self, MTR_LOG_DEBUG("_scheduleSubscriptionPoolWork workBlockToQueue called with nil MTRDevice"));

        // In the case where a resubscription triggering event happened and already established, running the work block should result in a no-op
        MTRAsyncWorkItem * workItem = [[MTRAsyncWorkItem alloc] initWithQueue:self.queue];
        [workItem setReadyHandler:^(id _Nonnull context, NSInteger retryCount, MTRAsyncWorkCompletionBlock _Nonnull completion) {
            mtr_strongify(self);
            VerifyOrReturn(self, MTR_LOG_DEBUG("_scheduleSubscriptionPoolWork readyHandler called with nil MTRDevice"));

            MTR_LOG("%@ - work item is ready to attempt pooled subscription", self);
            os_unfair_lock_lock(&self->_lock);
#ifdef DEBUG
            [self _callDelegatesWithBlock:^(id testDelegate) {
                mtr_strongify(self);
                if ([testDelegate respondsToSelector:@selector(unitTestSubscriptionPoolDequeue:)]) {
                    [testDelegate unitTestSubscriptionPoolDequeue:self];
                }
            }];
#endif
            if (self->_subscriptionPoolWorkCompletionBlock) {
                // This means a resubscription triggering event happened and is now in-progress
                MTR_LOG("%@ timer fired but already running in subscription pool - ignoring: %@", self, description);
                os_unfair_lock_unlock(&self->_lock);

                // call completion as complete to remove from queue
                completion(MTRAsyncWorkComplete);
                return;
            }

            // Otherwise, save the completion block
            self->_subscriptionPoolWorkCompletionBlock = completion;
            os_unfair_lock_unlock(&self->_lock);

            workBlock();
        }];
        [self._concreteController.concurrentSubscriptionPool enqueueWorkItem:workItem description:description];
        MTR_LOG("%@ - enqueued in the subscription pool", self);
    };

    if (inNanoseconds > 0) {
        // Wait the required amount of time, then put it in the subscription pool to wait additionally for a spot
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, inNanoseconds), self.queue, workBlockToQueue);
    } else {
        // Put in subscription pool directly if there is no wait time
        workBlockToQueue();
    }
}

- (void)_handleResubscriptionNeededWithDelay:(NSNumber *)resubscriptionDelayMs
{
    assertChipStackLockedByCurrentThread();

    std::lock_guard lock(_lock);

    // Change our state before going async.
    [self _changeState:MTRDeviceStateUnknown];

    // If we have never had a subscription established, stay in the Subscribing
    // state; don't transition to Resubscribing just because our attempt at
    // subscribing failed.
    if (HadSubscriptionEstablishedOnce(self->_internalDeviceState)) {
        [self _changeInternalState:MTRInternalDeviceStateResubscribing];
    }

    mtr_weakify(self);
    dispatch_async(self.queue, ^{
        mtr_strongify(self);
        VerifyOrReturn(self, MTR_LOG_DEBUG("_handleResubscriptionNeededWithDelay async to self.queue with nil MTRDevice"));

        [self _handleResubscriptionNeededWithDelayOnDeviceQueue:resubscriptionDelayMs];
    });
}

- (void)_handleResubscriptionNeededWithDelayOnDeviceQueue:(NSNumber *)resubscriptionDelayMs
{
    os_unfair_lock_lock(&self->_lock);

    // If we are here, then the ReadClient either just detected a subscription
    // drop or just tried again and failed.  Either way, count it as "tried and
    // failed to subscribe": in the latter case it's actually true, and in the
    // former case we recently had a subscription and do not want to be forcing
    // retries immediately.
    _lastSubscriptionFailureTime = [NSDate now];
    {
        std::lock_guard lock(_descriptionLock);
        _lastSubscriptionFailureTimeForDescription = _lastSubscriptionFailureTime;
    }
    BOOL deviceUsesThread = [self _deviceUsesThread];

    // If a previous resubscription failed, remove the item from the subscription pool.
    [self _clearSubscriptionPoolWork];

    os_unfair_lock_unlock(&self->_lock);

    // Use the existing _triggerResubscribeWithReason mechanism, which does the right checks when
    // this block is run -- if other triggering events had happened, this would become a no-op.
    mtr_weakify(self);
    auto resubscriptionBlock = ^{
        mtr_strongify(self);
        VerifyOrReturn(self, MTR_LOG_DEBUG("_handleResubscriptionNeededWithDelayOnDeviceQueue resubscriptionBlock called with nil MTRDevice"));

        [self->_deviceController asyncDispatchToMatterQueue:^{
            mtr_strongify(self);
            VerifyOrReturn(self, MTR_LOG_DEBUG("_handleResubscriptionNeededWithDelayOnDeviceQueue resubscriptionBlock asyncDispatchToMatterQueue called back with nil MTRDevice"));

            [self _triggerResubscribeWithReason:@"ResubscriptionNeeded timer fired" nodeLikelyReachable:NO];
        } errorHandler:^(NSError * _Nonnull error) {
            mtr_strongify(self);
            VerifyOrReturn(self, MTR_LOG_DEBUG("_handleResubscriptionNeededWithDelayOnDeviceQueue resubscriptionBlock asyncDispatchToMatterQueue errored with nil MTRDevice"));

            // If controller is not running, clear work item from the subscription queue
            MTR_LOG_ERROR("%@ could not dispatch to matter queue for resubscription - error %@", self, error);
            std::lock_guard lock(self->_lock);
            [self _clearSubscriptionPoolWork];
        }];
    };

    int64_t resubscriptionDelayNs = static_cast<int64_t>(resubscriptionDelayMs.unsignedIntValue * NSEC_PER_MSEC);
    if (deviceUsesThread) {
        std::lock_guard lock(_lock);
        // For Thread-enabled devices, schedule the _triggerResubscribeWithReason call to run in the subscription pool
        NSString * description = [NSString stringWithFormat:@"ReadClient resubscription (%p)", self];
        [self _scheduleSubscriptionPoolWork:resubscriptionBlock inNanoseconds:resubscriptionDelayNs description:description];
    } else {
        // For non-Thread-enabled devices, just call the resubscription block after the specified time
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, resubscriptionDelayNs), self.queue, resubscriptionBlock);
    }

    // Set up connectivity monitoring in case network routability changes for the positive, to accelerate resubscription
    [self _setupConnectivityMonitoring];
}

- (void)_handleSubscriptionReset:(NSNumber * _Nullable)retryDelay
{
    assertChipStackLockedByCurrentThread();

    std::lock_guard lock(_lock);
    [self _doHandleSubscriptionReset:retryDelay];
}

- (void)_setLastSubscriptionAttemptWait:(uint32_t)lastSubscriptionAttemptWait
{
    os_unfair_lock_assert_owner(&_lock);
    _lastSubscriptionAttemptWait = lastSubscriptionAttemptWait;

    {
        std::lock_guard lock(_descriptionLock);
        _lastSubscriptionAttemptWaitForDescription = lastSubscriptionAttemptWait;
    }

    [self _notifyDelegateOfPrivateInternalPropertiesChanges];
}

- (void)_doHandleSubscriptionReset:(NSNumber * _Nullable)retryDelay
{
    assertChipStackLockedByCurrentThread();

    os_unfair_lock_assert_owner(&_lock);

    if (self.suspended) {
        MTR_LOG("%@ ignoring expected subscription reset on controller suspend", self);
        [self _clearSubscriptionPoolWork];
        return;
    }

    // If we are here, then either we failed to establish initial CASE, or we
    // failed to send the initial SubscribeRequest message, or our ReadClient
    // has given up completely.  Those all count as "we have tried and failed to
    // subscribe".
    _lastSubscriptionFailureTime = [NSDate now];
    {
        std::lock_guard lock(_descriptionLock);
        _lastSubscriptionFailureTimeForDescription = _lastSubscriptionFailureTime;
    }

    // if there is no delegate then also do not retry
    if (![self _delegateExists]) {
        // NOTE: Do not log anything here: we have been invalidated, and the
        // Matter stack might already be torn down.
        [self _clearSubscriptionPoolWork];
        return;
    }

    // don't schedule multiple retries
    if (self.reattemptingSubscription) {
        return;
    }

    self.reattemptingSubscription = YES;

    NSTimeInterval secondsToWait;
    if (_lastSubscriptionAttemptWait < MTRDEVICE_SUBSCRIPTION_ATTEMPT_MIN_WAIT_SECONDS) {
        _lastSubscriptionAttemptWait = MTRDEVICE_SUBSCRIPTION_ATTEMPT_MIN_WAIT_SECONDS;
        secondsToWait = _lastSubscriptionAttemptWait;
    } else if (retryDelay != nil) {
        // The device responded but is currently busy. Reset our backoff
        // counter, so that we don't end up waiting for a long time if the next
        // attempt fails for some reason, and retry after whatever time period
        // the device told us to use.
        [self _setLastSubscriptionAttemptWait:0];
        secondsToWait = retryDelay.doubleValue;
        MTR_LOG("%@ resetting resubscribe attempt counter, and delaying by the server-provided delay: %f",
            self, secondsToWait);
    } else {
        auto lastSubscriptionAttemptWait = _lastSubscriptionAttemptWait * 2;
        if (lastSubscriptionAttemptWait > MTRDEVICE_SUBSCRIPTION_ATTEMPT_MAX_WAIT_SECONDS) {
            lastSubscriptionAttemptWait = MTRDEVICE_SUBSCRIPTION_ATTEMPT_MAX_WAIT_SECONDS;
        }
        [self _setLastSubscriptionAttemptWait:lastSubscriptionAttemptWait];
        secondsToWait = lastSubscriptionAttemptWait;
    }

    MTR_LOG("%@ scheduling to reattempt subscription in %f seconds", self, secondsToWait);

    // If we started subscription or session establishment but failed, remove item from the subscription pool so we can re-queue.
    [self _clearSubscriptionPoolWork];

    // Call _reattemptSubscriptionNowIfNeededWithReason when timer fires - if subscription is
    // in a better state at that time this will be a no-op.
    mtr_weakify(self);
    auto resubscriptionBlock = ^{
        mtr_strongify(self);
        VerifyOrReturn(self, MTR_LOG_DEBUG("_doHandleSubscriptionReset resubscriptionBlock called with nil MTRDevice"));

        [self->_deviceController asyncDispatchToMatterQueue:^{
            mtr_strongify(self);
            VerifyOrReturn(self, MTR_LOG_DEBUG("_doHandleSubscriptionReset resubscriptionBlock asyncDispatchToMatterQueue called back with nil MTRDevice"));

            std::lock_guard lock(self->_lock);
            [self _reattemptSubscriptionNowIfNeededWithReason:@"got subscription reset"];
        }
            errorHandler:^(NSError * _Nonnull error) {
                mtr_strongify(self);
                VerifyOrReturn(self, MTR_LOG_DEBUG("_doHandleSubscriptionReset resubscriptionBlock asyncDispatchToMatterQueue errored with nil MTRDevice"));

                // If controller is not running, clear work item from the subscription queue
                MTR_LOG_ERROR("%@ could not dispatch to matter queue for resubscription - error %@", self, error);
                std::lock_guard lock(self->_lock);
                [self _clearSubscriptionPoolWork];
            }];
    };

    int64_t resubscriptionDelayNs = static_cast<int64_t>(secondsToWait * NSEC_PER_SEC);
    if ([self _deviceUsesThread]) {
        // For Thread-enabled devices, schedule the _reattemptSubscriptionNowIfNeededWithReason call to run in the subscription pool
        NSString * description = [NSString stringWithFormat:@"MTRDevice resubscription (%p)", self];
        [self _scheduleSubscriptionPoolWork:resubscriptionBlock inNanoseconds:resubscriptionDelayNs description:description];
    } else {
        // For non-Thread-enabled devices, just call the resubscription block after the specified time
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, resubscriptionDelayNs), self.queue, resubscriptionBlock);
    }
    [self _notifyDelegateOfPrivateInternalPropertiesChanges];
}

- (void)_reattemptSubscriptionNowIfNeededWithReason:(NSString *)reason
{
    assertChipStackLockedByCurrentThread();

    os_unfair_lock_assert_owner(&self->_lock);
    if (!self.reattemptingSubscription) {
        [self _clearSubscriptionPoolWork];
        return;
    }

    MTR_LOG("%@ reattempting subscription with reason %@", self, reason);
    [self _setupSubscriptionWithReason:reason];
}

- (void)_handleUnsolicitedMessageFromPublisher
{
    assertChipStackLockedByCurrentThread();

    std::lock_guard lock(_lock);

    [self _changeState:MTRDeviceStateReachable];

    // Given the framework requests a minimum subscription keep alive time of devices, this callback is not expected to happen more often than that
    BOOL shouldCallDelegate = NO;
    if (self.lastDeviceBecameActiveCallbackTime) {
        NSTimeInterval intervalSinceLastCallback = -[self.lastDeviceBecameActiveCallbackTime timeIntervalSinceNow];
        if (intervalSinceLastCallback > MTR_DEVICE_MIN_SECONDS_BETWEEN_DEVICE_BECAME_ACTIVE_CALLBACKS) {
            shouldCallDelegate = YES;
        }
    } else {
        shouldCallDelegate = YES;
    }

    if (shouldCallDelegate) {
        [self _callDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
            if ([delegate respondsToSelector:@selector(deviceBecameActive:)]) {
                [delegate deviceBecameActive:self];
            }
        }];
        self.lastDeviceBecameActiveCallbackTime = [NSDate now];
        self.throttlingDeviceBecameActiveCallbacks = NO;
    } else if (!self.throttlingDeviceBecameActiveCallbacks) {
        MTR_LOG("%@ throttling deviceBecameActive callbacks because report came in too soon after %@", self, self.lastDeviceBecameActiveCallbackTime);
        self.throttlingDeviceBecameActiveCallbacks = YES;
    }

    // in case this is called during exponential back off of subscription
    // reestablishment, this starts the attempt right away
    // TODO: This doesn't really make sense.  If we _don't_ have a live
    // ReadClient how did we get this notification and if we _do_ have an active
    // ReadClient, this call or _setupSubscriptionWithReason would be no-ops.
    [self _reattemptSubscriptionNowIfNeededWithReason:@"got unsolicited message from publisher"];
}

- (void)_markDeviceAsUnreachableIfNeverSubscribed
{
    mtr_weakify(self);
    [_deviceController asyncDispatchToMatterQueue:^{
        mtr_strongify(self);
        VerifyOrReturn(self, MTR_LOG_DEBUG("_markDeviceAsUnreachableIfNeverSubscribed asyncDispatchToMatterQueue called back with nil MTRDevice"));

        std::lock_guard lock(self->_lock);

        if (HadSubscriptionEstablishedOnce(self->_internalDeviceState)) {
            return;
        }

        MTR_LOG("%@ still not subscribed, marking the device as unreachable", self);
        [self _changeState:MTRDeviceStateUnreachable];
    }
                                     errorHandler:nil];
}

- (void)_handleReportBegin
{
    assertChipStackLockedByCurrentThread();

    std::lock_guard lock(_lock);

    _receivingReport = YES;
    if (_state != MTRDeviceStateReachable) {
        [self _changeState:MTRDeviceStateReachable];
    }

    // If we currently don't have an established subscription, this must be a
    // priming report.
    _receivingPrimingReport = !HaveSubscriptionEstablishedRightNow(_internalDeviceState);

    // For unit testing only
#ifdef DEBUG
    [self _callDelegatesWithBlock:^(id testDelegate) {
        if ([testDelegate respondsToSelector:@selector(unitTestReportBeginForDevice:)]) {
            [testDelegate unitTestReportBeginForDevice:self];
        }
    }];
#endif
}

- (NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)_clusterDataToPersistSnapshot
{
    os_unfair_lock_assert_owner(&self->_lock);
    NSMutableDictionary * clusterDataToReturn = [NSMutableDictionary dictionary];
    for (MTRClusterPath * clusterPath in _clusterDataToPersist) {
        clusterDataToReturn[clusterPath] = [_clusterDataToPersist[clusterPath] copy];
    }

    return clusterDataToReturn;
}

- (NSTimeInterval)_reportToPersistenceDelayTimeAfterMutiplier
{
    return _storageBehaviorConfiguration.reportToPersistenceDelayTime * _reportToPersistenceDelayCurrentMultiplier;
}

- (NSTimeInterval)_reportToPersistenceDelayTimeMaxAfterMutiplier
{
    return _storageBehaviorConfiguration.reportToPersistenceDelayTimeMax * _reportToPersistenceDelayCurrentMultiplier;
}

- (BOOL)_dataStoreExists
{
    os_unfair_lock_assert_owner(&self->_lock);
    return _persistedClusterData != nil;
}

// Need an inner method for dealloc to call, so unit test callbacks don't re-capture self.
//
// Returns whether persistence actually happened.
- (BOOL)_doPersistClusterData
{
    os_unfair_lock_assert_owner(&self->_lock);

    // Sanity check
    if (![self _dataStoreExists]) {
        MTR_LOG_ERROR("%@ storage behavior: no data store in _persistClusterData!", self);
        return NO;
    }

    // Nothing to persist
    if (!_clusterDataToPersist.count) {
        return NO;
    }

    MTR_LOG("%@ Storing cluster information (data version and attributes) count: %lu", self, static_cast<unsigned long>(_clusterDataToPersist.count));
    // We're going to hand out these MTRDeviceClusterData objects to our
    // storage implementation, which will try to read them later.  Make sure
    // we snapshot the state here instead of handing out live copies.
    NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> * clusterData = [self _clusterDataToPersistSnapshot];
    [self._concreteController.controllerDataStore storeClusterData:clusterData forNodeID:_nodeID];
    for (MTRClusterPath * clusterPath in _clusterDataToPersist) {
        [_persistedClusterData setObject:_clusterDataToPersist[clusterPath] forKey:clusterPath];
        [_persistedClusters addObject:clusterPath];
    }

    // TODO: There is one edge case not handled well here: if the
    // storeClusterData call above fails somehow, and then the data gets
    // evicted from _persistedClusterData, we could end up in a situation
    // where when we page things in from storage we have stale values and
    // hence effectively lose the delta that we failed to persist.
    //
    // The only way to handle this would be to detect it when it happens,
    // then re-subscribe at that point, which would cause the relevant data
    // to be sent to us via the priming read.
    _clusterDataToPersist = nil;

    return YES;
}

- (void)_persistClusterData
{
    if ([self _doPersistClusterData] == NO) {
        // Don't notify delegates if we did not actually persist anything.
        return;
    }

#ifdef DEBUG
    [self _callDelegatesWithBlock:^(id testDelegate) {
        if ([testDelegate respondsToSelector:@selector(unitTestClusterDataPersisted:)]) {
            [testDelegate unitTestClusterDataPersisted:self];
        }
    }];
#endif
}

- (BOOL)_deviceIsReportingExcessively
{
    os_unfair_lock_assert_owner(&self->_lock);

    if (!_deviceReportingExcessivelyStartTime) {
        return NO;
    }

    NSTimeInterval intervalSinceDeviceReportingExcessively = -[_deviceReportingExcessivelyStartTime timeIntervalSinceNow];
    BOOL deviceIsReportingExcessively = intervalSinceDeviceReportingExcessively > _storageBehaviorConfiguration.deviceReportingExcessivelyIntervalThreshold;
    if (deviceIsReportingExcessively) {
        MTR_LOG("%@ storage behavior: device has been reporting excessively for %.3lf seconds", self, intervalSinceDeviceReportingExcessively);
    }
    return deviceIsReportingExcessively;
}

- (void)_persistClusterDataAsNeeded
{
    std::lock_guard lock(_lock);

    // Nothing to persist
    if (!_clusterDataToPersist.count) {
        return;
    }

    // This is run with a dispatch_after, and need to check again if this device is reporting excessively
    if ([self _deviceIsReportingExcessively]) {
        return;
    }

    NSDate * lastReportTime = [_mostRecentReportTimes lastObject];
    NSTimeInterval intervalSinceLastReport = -[lastReportTime timeIntervalSinceNow];
    if (intervalSinceLastReport < [self _reportToPersistenceDelayTimeAfterMutiplier]) {
        // A report came in after this call was scheduled

        if (!_clusterDataPersistenceFirstScheduledTime) {
            MTR_LOG_ERROR("%@ storage behavior: expects _clusterDataPersistenceFirstScheduledTime if _clusterDataToPersist exists", self);
            return;
        }

        NSTimeInterval intervalSinceFirstScheduledPersistence = -[_clusterDataPersistenceFirstScheduledTime timeIntervalSinceNow];
        if (intervalSinceFirstScheduledPersistence < [self _reportToPersistenceDelayTimeMaxAfterMutiplier]) {
            MTR_LOG("%@ storage behavior: not persisting: intervalSinceLastReport %lf intervalSinceFirstScheduledPersistence %lf", self, intervalSinceLastReport, intervalSinceFirstScheduledPersistence);
            // The max delay is also not reached - do not persist yet
            return;
        }
    }

    // At this point, there is data to persist, and either _reportToPersistenceDelayTime was
    // reached, or _reportToPersistenceDelayTimeMax was reached. Time to persist:
    [self _persistClusterData];

    _clusterDataPersistenceFirstScheduledTime = nil;
}

#ifdef DEBUG
- (void)unitTestSetMostRecentReportTimes:(NSMutableArray<NSDate *> *)mostRecentReportTimes
{
    _mostRecentReportTimes = mostRecentReportTimes;

    {
        std::lock_guard lock(_descriptionLock);
        _mostRecentReportTimeForDescription = [mostRecentReportTimes lastObject];
    }
    std::lock_guard lock(_lock);
    [self _notifyDelegateOfPrivateInternalPropertiesChanges];
}
#endif

- (void)_scheduleClusterDataPersistence
{
    os_unfair_lock_assert_owner(&self->_lock);

    // No persisted data / lack of controller data store
    if (![self _dataStoreExists]) {
        MTR_LOG_DEBUG("%@ storage behavior: no data store", self);
        return;
    }

    // Nothing to persist
    if (!_clusterDataToPersist.count) {
        MTR_LOG_DEBUG("%@ storage behavior: nothing to persist", self);
        return;
    }

    // If there is no storage behavior configuration, make a default one
    if (!_storageBehaviorConfiguration) {
        _storageBehaviorConfiguration = [[MTRDeviceStorageBehaviorConfiguration alloc] init];
        [_storageBehaviorConfiguration checkValuesAndResetToDefaultIfNecessary];
    }

    // Directly store if the storage behavior optimization is disabled
    if (_storageBehaviorConfiguration.disableStorageBehaviorOptimization) {
        [self _persistClusterData];
        return;
    }

    // If we have nothing stored at all yet, store directly, so we move into a
    // primed state.
    if (!_deviceCachePrimed) {
        [self _persistClusterData];
        return;
    }

    // Ensure there is an array to keep the most recent report times
    if (!_mostRecentReportTimes) {
        _mostRecentReportTimes = [NSMutableArray array];
    }

    // Mark when first report comes in to know when _reportToPersistenceDelayTimeMax is hit
    if (!_clusterDataPersistenceFirstScheduledTime) {
        _clusterDataPersistenceFirstScheduledTime = [NSDate now];
    }

    // Make sure there is space in the array, and note report time
    while (_mostRecentReportTimes.count >= _storageBehaviorConfiguration.recentReportTimesMaxCount) {
        [_mostRecentReportTimes removeObjectAtIndex:0];
    }
    [_mostRecentReportTimes addObject:[NSDate now]];

    {
        std::lock_guard lock(_descriptionLock);
        _mostRecentReportTimeForDescription = [_mostRecentReportTimes lastObject];
    }

    // Calculate running average and update multiplier - need at least 2 items to calculate intervals
    if (_mostRecentReportTimes.count > 2) {
        NSTimeInterval cumulativeIntervals = 0;
        for (int i = 1; i < _mostRecentReportTimes.count; i++) {
            NSDate * lastDate = [_mostRecentReportTimes objectAtIndex:i - 1];
            NSDate * currentDate = [_mostRecentReportTimes objectAtIndex:i];
            NSTimeInterval intervalSinceLastReport = [currentDate timeIntervalSinceDate:lastDate];
            // Check to guard against clock change
            if (intervalSinceLastReport > 0) {
                cumulativeIntervals += intervalSinceLastReport;
            }
        }
        NSTimeInterval averageTimeBetweenReports = cumulativeIntervals / (_mostRecentReportTimes.count - 1);

        if (averageTimeBetweenReports < _storageBehaviorConfiguration.timeBetweenReportsTooShortThreshold) {
            // Multiplier goes from 1 to _reportToPersistenceDelayMaxMultiplier uniformly, as
            // averageTimeBetweenReports go from timeBetweenReportsTooShortThreshold to
            // timeBetweenReportsTooShortMinThreshold

            double intervalAmountBelowThreshold = _storageBehaviorConfiguration.timeBetweenReportsTooShortThreshold - averageTimeBetweenReports;
            double intervalAmountBetweenThresholdAndMinThreshold = _storageBehaviorConfiguration.timeBetweenReportsTooShortThreshold - _storageBehaviorConfiguration.timeBetweenReportsTooShortMinThreshold;
            double proportionTowardMinThreshold = intervalAmountBelowThreshold / intervalAmountBetweenThresholdAndMinThreshold;
            if (proportionTowardMinThreshold > 1) {
                // Clamp to 100%
                proportionTowardMinThreshold = 1;
            }

            // Set current multiplier to [1, MaxMultiplier]
            _reportToPersistenceDelayCurrentMultiplier = 1 + (proportionTowardMinThreshold * (_storageBehaviorConfiguration.reportToPersistenceDelayMaxMultiplier - 1));
            MTR_LOG("%@ storage behavior: device reporting frequently - setting delay multiplier to %lf", self, _reportToPersistenceDelayCurrentMultiplier);
        } else {
            _reportToPersistenceDelayCurrentMultiplier = 1;
        }

        // Also note when the running average first dips below the min threshold
        if (averageTimeBetweenReports < _storageBehaviorConfiguration.timeBetweenReportsTooShortMinThreshold) {
            if (!_deviceReportingExcessivelyStartTime) {
                _deviceReportingExcessivelyStartTime = [NSDate now];
                MTR_LOG_DEBUG("%@ storage behavior: device is reporting excessively @%@", self, _deviceReportingExcessivelyStartTime);
            }
        } else {
            _deviceReportingExcessivelyStartTime = nil;
        }
    }

    // Do not schedule persistence if device is reporting excessively
    if ([self _deviceIsReportingExcessively]) {
        return;
    }

    mtr_weakify(self);
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) ([self _reportToPersistenceDelayTimeAfterMutiplier] * NSEC_PER_SEC)), self.queue, ^{
        mtr_strongify(self);
        VerifyOrReturn(self, MTR_LOG_DEBUG("_scheduleClusterDataPersistence delayed store block called with nil MTRDevice"));

        [self _persistClusterDataAsNeeded];
    });
}

// Used to clear the storage behavior state when needed (system time change, or when new
// configuration is set.
//
// Also flushes unwritten cluster data to storage, if data store exists.
- (void)_resetStorageBehaviorState
{
    os_unfair_lock_assert_owner(&self->_lock);

    _clusterDataPersistenceFirstScheduledTime = nil;
    _mostRecentReportTimes = nil;
    {
        std::lock_guard lock(_descriptionLock);
        _mostRecentReportTimeForDescription = nil;
    }

    _deviceReportingExcessivelyStartTime = nil;
    _reportToPersistenceDelayCurrentMultiplier = 1;

    // Sanity check that there is a data
    if ([self _dataStoreExists]) {
        [self _persistClusterData];
    }

    [self _notifyDelegateOfPrivateInternalPropertiesChanges];
}

- (void)setStorageBehaviorConfiguration:(MTRDeviceStorageBehaviorConfiguration *)storageBehaviorConfiguration
{
    MTR_LOG("%@ storage behavior: setStorageBehaviorConfiguration %@", self, storageBehaviorConfiguration);
    std::lock_guard lock(_lock);
    _storageBehaviorConfiguration = storageBehaviorConfiguration;
    // Make sure the values are sane
    [_storageBehaviorConfiguration checkValuesAndResetToDefaultIfNecessary];
    [self _resetStorageBehaviorState];
}

- (void)_handleReportEnd
{
    MTR_LOG("%@ handling report end", self);

    std::lock_guard lock(_lock);
    _receivingReport = NO;
    _receivingPrimingReport = NO;
    _estimatedStartTimeFromGeneralDiagnosticsUpTime = nil;

    [self _scheduleClusterDataPersistence];

    // After the handling of the report, if we detected a device configuration change, notify the delegate
    // of the same.
    if (_deviceConfigurationChanged) {
        [self _callDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
            if ([delegate respondsToSelector:@selector(deviceConfigurationChanged:)]) {
                [delegate deviceConfigurationChanged:self];
            }
        }];
        [self _notifyDelegateOfPrivateInternalPropertiesChanges];
        _deviceConfigurationChanged = NO;
    }

    // Do this after the _deviceConfigurationChanged check, so that we don't
    // call deviceConfigurationChanged: immediately after telling our delegate
    // we are now primed.
    //
    // TODO: Maybe we shouldn't dispatch deviceConfigurationChanged: for the
    // initial priming bits?
    if (!_deviceCachePrimed) {
        // This is the end of the priming sequence of data reports, so we have
        // all the data for the device now.
        _deviceCachePrimed = YES;
        [self _callDelegateDeviceCachePrimed];
        [self _notifyDelegateOfPrivateInternalPropertiesChanges];
    }

// For unit testing only
#ifdef DEBUG
    [self _callDelegatesWithBlock:^(id testDelegate) {
        if ([testDelegate respondsToSelector:@selector(unitTestReportEndForDevice:)]) {
            [testDelegate unitTestReportEndForDevice:self];
        }
    }];
#endif
}

- (BOOL)_interestedPaths:(NSArray * _Nullable)interestedPaths includesAttributePath:(MTRAttributePath *)attributePath
{
    for (id interestedPath in interestedPaths) {
        if ([interestedPath isKindOfClass:[NSNumber class]]) {
            NSNumber * interestedEndpointIDNumber = interestedPath;
            if ([interestedEndpointIDNumber isEqualToNumber:attributePath.endpoint]) {
                return YES;
            }
        } else if ([interestedPath isKindOfClass:[MTRClusterPath class]]) {
            MTRClusterPath * interestedClusterPath = interestedPath;
            if ([interestedClusterPath.cluster isEqualToNumber:attributePath.cluster]) {
                return YES;
            }
        } else if ([interestedPath isKindOfClass:[MTRAttributePath class]]) {
            MTRAttributePath * interestedAttributePath = interestedPath;
            if (([interestedAttributePath.cluster isEqualToNumber:attributePath.cluster]) && ([interestedAttributePath.attribute isEqualToNumber:attributePath.attribute])) {
                return YES;
            }
        }
    }

    return NO;
}

// Returns filtered set of attributes using an interestedPaths array.
// Returns nil if no attribute report has a path that matches the paths in the interestedPaths array.
- (NSArray<NSDictionary<NSString *, id> *> *)_filteredAttributes:(NSArray<NSDictionary<NSString *, id> *> *)attributes forInterestedPaths:(NSArray * _Nullable)interestedPaths
{
    if (!interestedPaths) {
        return attributes;
    }

    if (!interestedPaths.count) {
        return nil;
    }

    NSMutableArray * filteredAttributes = nil;
    for (NSDictionary<NSString *, id> * responseValue in attributes) {
        MTRAttributePath * attributePath = responseValue[MTRAttributePathKey];
        if ([self _interestedPaths:interestedPaths includesAttributePath:attributePath]) {
            if (!filteredAttributes) {
                filteredAttributes = [NSMutableArray array];
            }
            [filteredAttributes addObject:responseValue];
        }
    }

    if (filteredAttributes.count && (filteredAttributes.count != attributes.count)) {
        MTR_LOG("%@ filtered attribute report %lu => %lu", self, static_cast<unsigned long>(attributes.count), static_cast<unsigned long>(filteredAttributes.count));
    }

    return filteredAttributes;
}

// assume lock is held
- (void)_reportAttributes:(NSArray<NSDictionary<NSString *, id> *> *)attributes
{
    os_unfair_lock_assert_owner(&self->_lock);
    if (attributes.count) {
        [self _iterateDelegatesWithBlock:^(MTRDeviceDelegateInfo * delegateInfo) {
            // _iterateDelegatesWithBlock calls this with an autorelease pool, and so temporary filtered attributes reports don't bloat memory
            NSArray<NSDictionary<NSString *, id> *> * filteredAttributes = [self _filteredAttributes:attributes forInterestedPaths:delegateInfo.interestedPathsForAttributes];
            if (filteredAttributes.count) {
                [delegateInfo callDelegateWithBlock:^(id<MTRDeviceDelegate> delegate) {
                    [delegate device:self receivedAttributeReport:filteredAttributes];
                }];
            }
        }];
    }
}

- (void)_handleAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport fromSubscription:(BOOL)isFromSubscription
{
    // Note: %p to avoid double-logging the whole big structure; can be matched
    // up to where we receive the attribute report, which logs pointer value and
    // actual value.
    MTR_LOG("%@ handling attribute report %p, fromSubscription: %@", self, attributeReport, MTR_YES_NO(isFromSubscription));

    std::lock_guard lock(_lock);

    // _getAttributesToReportWithReportedValues will log attribute paths reported
    [self _reportAttributes:[self _getAttributesToReportWithReportedValues:attributeReport fromSubscription:isFromSubscription]];
}

// BEGIN DRAGON: This is used by the XPC Server to inject reports into local cache and broadcast them
- (void)_injectAttributeReport:(NSArray<MTRDeviceResponseValueDictionary> *)attributeReport fromSubscription:(BOOL)isFromSubscription
{
    if (!MTRAttributeReportIsWellFormed(attributeReport)) {
        MTR_LOG_ERROR("%@ injected attribute report is not well-formed: %@", self, attributeReport);
        return;
    }

    mtr_weakify(self);
    [_deviceController asyncDispatchToMatterQueue:^{
        mtr_strongify(self);
        VerifyOrReturn(self, MTR_LOG_DEBUG("_injectAttributeReport asyncDispatchToMatterQueue called back with nil MTRDevice"));

        MTR_LOG("%@ injected attribute report (%p) %@", self, attributeReport, attributeReport);
        [self _handleReportBegin];
        dispatch_async(self.queue, ^{
            mtr_strongify(self);
            VerifyOrReturn(self, MTR_LOG_DEBUG("_injectAttributeReport async to self.queue with nil MTRDevice"));

            [self _handleAttributeReport:attributeReport fromSubscription:isFromSubscription];
            [self _handleReportEnd];
        });
    } errorHandler:nil];
}

- (void)_injectEventReport:(NSArray<MTRDeviceResponseValueDictionary> *)eventReport
{
    if (!MTREventReportIsWellFormed(eventReport)) {
        MTR_LOG_ERROR("%@ injected event report is not well-formed: %@", self, eventReport);
        return;
    }

    [self _injectPossiblyInvalidEventReport:eventReport];
}

- (void)_injectPossiblyInvalidEventReport:(NSArray<MTRDeviceResponseValueDictionary> *)eventReport
{
    mtr_weakify(self);
    dispatch_async(self.queue, ^{
        mtr_strongify(self);
        VerifyOrReturn(self, MTR_LOG_DEBUG("_injectPossiblyInvalidEventReport async to self.queue with nil MTRDevice"));

        [self _handleEventReport:eventReport];
    });
}

// END DRAGON: This is used by the XPC Server to inject attribute reports

#ifdef DEBUG
- (void)unitTestInjectEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport
{
    // Don't validate incoming event reports for unit tests, because we want to
    // allow incoming event reports without an MTREventIsHistoricalKey.
    [self _injectPossiblyInvalidEventReport:eventReport];
}

- (void)unitTestInjectAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport fromSubscription:(BOOL)isFromSubscription
{
    [self _injectAttributeReport:attributeReport fromSubscription:isFromSubscription];
}
#endif

- (BOOL)_interestedPaths:(NSArray * _Nullable)interestedPaths includesEventPath:(MTREventPath *)eventPath
{
    for (id interestedPath in interestedPaths) {
        if ([interestedPath isKindOfClass:[NSNumber class]]) {
            NSNumber * interestedEndpointIDNumber = interestedPath;
            if ([interestedEndpointIDNumber isEqualToNumber:eventPath.endpoint]) {
                return YES;
            }
        } else if ([interestedPath isKindOfClass:[MTRClusterPath class]]) {
            MTRClusterPath * interestedClusterPath = interestedPath;
            if ([interestedClusterPath.cluster isEqualToNumber:eventPath.cluster]) {
                return YES;
            }
        } else if ([interestedPath isKindOfClass:[MTREventPath class]]) {
            MTREventPath * interestedEventPath = interestedPath;
            if (([interestedEventPath.cluster isEqualToNumber:eventPath.cluster]) && ([interestedEventPath.event isEqualToNumber:eventPath.event])) {
                return YES;
            }
        }
    }

    return NO;
}

// Returns filtered set of events using an interestedPaths array.
// Returns nil if no event report has a path that matches the paths in the interestedPaths array.
- (NSArray<NSDictionary<NSString *, id> *> *)_filteredEvents:(NSArray<NSDictionary<NSString *, id> *> *)events forInterestedPaths:(NSArray * _Nullable)interestedPaths
{
    if (!interestedPaths) {
        return events;
    }

    if (!interestedPaths.count) {
        return nil;
    }

    NSMutableArray * filteredEvents = nil;
    for (NSDictionary<NSString *, id> * responseValue in events) {
        MTREventPath * eventPath = responseValue[MTREventPathKey];
        if ([self _interestedPaths:interestedPaths includesEventPath:eventPath]) {
            if (!filteredEvents) {
                filteredEvents = [NSMutableArray array];
            }
            [filteredEvents addObject:responseValue];
        }
    }

    if (filteredEvents.count && (filteredEvents.count != events.count)) {
        MTR_LOG("%@ filtered event report %lu => %lu", self, static_cast<unsigned long>(events.count), static_cast<unsigned long>(filteredEvents.count));
    }

    return filteredEvents;
}

- (void)_handleEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport
{
    std::lock_guard lock(_lock);

    NSDate * oldEstimatedStartTime = _estimatedStartTime;
    // Combine with previous unreported events, if they exist
    NSMutableArray * reportToReturn;
    if (_unreportedEvents) {
        reportToReturn = _unreportedEvents;
    } else {
        reportToReturn = [NSMutableArray array];
    }
    for (NSDictionary<NSString *, id> * eventDict in eventReport) {
        // Whenever a StartUp event is received, reset the estimated start time
        //   New subscription case
        //     - Starts Unreachable
        //     - Start CASE and send subscription request
        //     - Receive priming report ReportBegin
        //     - Optionally receive UpTime attribute - update time and save start time estimate
        //     - Optionally receive StartUp event
        //       - Set estimated system time from event receipt time, or saved UpTime estimate if exists
        //     - ReportEnd handler clears the saved start time estimate based on UpTime
        //   Subscription dropped from client point of view case
        //     - Starts Unreachable
        //     - Resubscribe happens after some time, and then same as the above
        //   Server resuming subscription after reboot case
        //     - Starts Reachable
        //     - Receive priming report ReportBegin
        //     - Optionally receive UpTime attribute - update time and save value
        //     - Optionally receive StartUp event
        //       - Set estimated system time from event receipt time, or saved UpTime estimate if exists
        //     - ReportEnd handler clears the saved start time estimate based on UpTime
        //   Server resuming subscription after timeout case
        //     - Starts Reachable
        //     - Receive priming report ReportBegin
        //     - Optionally receive UpTime attribute - update time and save value
        //     - ReportEnd handler clears the saved start time estimate based on UpTime
        MTREventPath * eventPath = eventDict[MTREventPathKey];
        BOOL isStartUpEvent = (eventPath.cluster.unsignedLongValue == MTRClusterIDTypeBasicInformationID)
            && (eventPath.event.unsignedLongValue == MTREventIDTypeClusterBasicInformationEventStartUpID);
        if (isStartUpEvent) {
            if (_estimatedStartTimeFromGeneralDiagnosticsUpTime) {
                // If UpTime was received, make use of it as mark of system start time
                MTR_LOG("%@ StartUp event: set estimated start time forward to %@", self,
                    _estimatedStartTimeFromGeneralDiagnosticsUpTime);
                _estimatedStartTime = _estimatedStartTimeFromGeneralDiagnosticsUpTime;
            } else {
                // If UpTime was not received, reset estimated start time in case of reboot
                MTR_LOG("%@ StartUp event: set estimated start time to nil", self);
                _estimatedStartTime = nil;
            }
        }

        // If event time is of MTREventTimeTypeSystemUpTime type, then update estimated start time as needed
        NSNumber * eventTimeTypeNumber = eventDict[MTREventTimeTypeKey];
        if (!eventTimeTypeNumber) {
            MTR_LOG_ERROR("%@ Event %@ missing event time type", self, eventDict);
            continue;
        }
        MTREventTimeType eventTimeType = (MTREventTimeType) eventTimeTypeNumber.unsignedIntegerValue;
        if (eventTimeType == MTREventTimeTypeSystemUpTime) {
            NSNumber * eventTimeValueNumber = eventDict[MTREventSystemUpTimeKey];
            if (!eventTimeValueNumber) {
                MTR_LOG_ERROR("%@ Event %@ missing event time value", self, eventDict);
                continue;
            }
            NSTimeInterval eventTimeValue = eventTimeValueNumber.doubleValue;
            NSDate * potentialSystemStartTime = [NSDate dateWithTimeIntervalSinceNow:-eventTimeValue];
            if (!_estimatedStartTime || ([potentialSystemStartTime compare:_estimatedStartTime] == NSOrderedAscending)) {
                _estimatedStartTime = potentialSystemStartTime;
            }
        }

        auto * eventNumber = MTR_SAFE_CAST(eventDict[MTREventNumberKey], NSNumber);
        if (!eventNumber) {
            MTR_LOG_ERROR("%@ Event %@ missing event number", self, eventDict);
            continue;
        }

        if (!self.highestObservedEventNumber ||
            [self.highestObservedEventNumber compare:eventNumber] == NSOrderedAscending) {
            // This is an event we have not seen before.
            self.highestObservedEventNumber = eventNumber;
        } else {
            // We have seen this event already; just filter it out.  But also, we must be getting
            // some sort of priming report if we are getting events we have seen before.
            if (_receivingReport) {
                _receivingPrimingReport = YES;
            }
            continue;
        }

        NSMutableDictionary * eventToReturn = eventDict.mutableCopy;
        // If MTREventIsHistoricalKey is already present, do not mess with the value.
        if (eventToReturn[MTREventIsHistoricalKey] == nil) {
            if (_receivingPrimingReport) {
                eventToReturn[MTREventIsHistoricalKey] = @(YES);
            } else {
                eventToReturn[MTREventIsHistoricalKey] = @(NO);
            }
        }

        [reportToReturn addObject:eventToReturn];
    }
    if (oldEstimatedStartTime != _estimatedStartTime) {
        MTR_LOG("%@ updated estimated start time to %@", self, _estimatedStartTime);
    }

    __block BOOL delegatesCalled = NO;
    [self _iterateDelegatesWithBlock:^(MTRDeviceDelegateInfo * delegateInfo) {
        // _iterateDelegatesWithBlock calls this with an autorelease pool, and so temporary filtered event reports don't bloat memory
        NSArray<NSDictionary<NSString *, id> *> * filteredEvents = [self _filteredEvents:reportToReturn forInterestedPaths:delegateInfo.interestedPathsForEvents];
        if (filteredEvents.count) {
            [delegateInfo callDelegateWithBlock:^(id<MTRDeviceDelegate> delegate) {
                [delegate device:self receivedEventReport:filteredEvents];
            }];
            delegatesCalled = YES;
        }
    }];
    if (delegatesCalled) {
        _unreportedEvents = nil;
    } else {
        // save unreported events
        _unreportedEvents = reportToReturn;
    }
}

#ifdef DEBUG
- (void)unitTestClearClusterData
{
    std::lock_guard lock(_lock);
    NSAssert([self _dataStoreExists], @"Test is not going to test what it thinks is testing!");
    [_persistedClusterData removeAllObjects];
}
#endif

- (void)_reconcilePersistedClustersWithStorage
{
    os_unfair_lock_assert_owner(&self->_lock);

    NSMutableSet * clusterPathsToRemove = [NSMutableSet set];
    for (MTRClusterPath * clusterPath in _persistedClusters) {
        MTRDeviceClusterData * data = [self._concreteController.controllerDataStore getStoredClusterDataForNodeID:_nodeID endpointID:clusterPath.endpoint clusterID:clusterPath.cluster];
        if (!data) {
            [clusterPathsToRemove addObject:clusterPath];
        }
    }

    MTR_LOG_ERROR("%@ Storage missing %lu / %lu clusters - reconciling in-memory records", self, static_cast<unsigned long>(clusterPathsToRemove.count), static_cast<unsigned long>(_persistedClusters.count));
    [_persistedClusters minusSet:clusterPathsToRemove];
}

- (nullable MTRDeviceClusterData *)_clusterDataForPath:(MTRClusterPath *)clusterPath
{
    os_unfair_lock_assert_owner(&self->_lock);

    if (_clusterDataToPersist != nil) {
        // Use the "dirty" values, if we have them.
        MTRDeviceClusterData * data = _clusterDataToPersist[clusterPath];
        if (data != nil) {
            return data;
        }
    }

    if ([self _dataStoreExists]) {
        MTRDeviceClusterData * data = [_persistedClusterData objectForKey:clusterPath];
        if (data != nil) {
            return data;
        }
    }

    if (![_persistedClusters containsObject:clusterPath]) {
        // We are not expected to have this cluster, so no point in paging it in
        // loading it from storage.
        return nil;
    }

    NSAssert(self._concreteController.controllerDataStore != nil,
        @"How can _persistedClusters have an entry if we have no persistence?");
    NSAssert(_persistedClusterData != nil,
        @"How can _persistedClusterData not exist if we have persisted clusters?");

    // Page in the stored value for the data.
    MTRDeviceClusterData * data = [self._concreteController.controllerDataStore getStoredClusterDataForNodeID:_nodeID endpointID:clusterPath.endpoint clusterID:clusterPath.cluster];
    MTR_LOG("%@ cluster path %@ cache miss - load from storage success %@", self, clusterPath, MTR_YES_NO(data));
    if (data != nil) {
        [_persistedClusterData setObject:data forKey:clusterPath];
    } else {
        // If clusterPath is in _persistedClusters and the data store returns nil for it, then the in-memory cache is now not dependable, and subscription should be reset and reestablished to reload cache from device

        // First make sure _persistedClusters is consistent with storage, so repeated calls don't immediately re-trigger this
        [self _reconcilePersistedClustersWithStorage];

        [self _resetSubscriptionWithReasonString:[NSString stringWithFormat:@"Data store has no data for cluster %@", clusterPath]];
    }

    return data;
}

- (NSSet<MTRClusterPath *> *)_knownClusters
{
    os_unfair_lock_assert_owner(&self->_lock);

    // We might have some clusters that have not been persisted at all yet, and
    // some that have been persisted but are still present in
    // _clusterDataToPersist because they have been modified since then.
    NSMutableSet<MTRClusterPath *> * clusterPaths = [_persistedClusters mutableCopy];
    if (_clusterDataToPersist != nil) {
        [clusterPaths unionSet:[NSSet setWithArray:[_clusterDataToPersist allKeys]]];
    }
    return clusterPaths;
}

- (NSDictionary<MTRClusterPath *, NSNumber *> *)_getCachedDataVersions
{
    NSMutableDictionary<MTRClusterPath *, NSNumber *> * dataVersions = [NSMutableDictionary dictionary];
    std::lock_guard lock(_lock);

    for (MTRClusterPath * path in [self _knownClusters]) {
        dataVersions[path] = [self _clusterDataForPath:path].dataVersion;
    }

    MTR_LOG_DEBUG("%@ _getCachedDataVersions dataVersions count: %lu", self, static_cast<unsigned long>(dataVersions.count));

    return dataVersions;
}

- (MTRDeviceDataValueDictionary _Nullable)_cachedAttributeValueForPath:(MTRAttributePath *)path
{
    os_unfair_lock_assert_owner(&self->_lock);

    // We need an actual MTRClusterPath, not a subsclass, to do _clusterDataForPath.
    auto * clusterPath = [MTRClusterPath clusterPathWithEndpointID:path.endpoint clusterID:path.cluster];

    MTRDeviceClusterData * clusterData = [self _clusterDataForPath:clusterPath];
    if (clusterData == nil) {
        return nil;
    }

    return clusterData.attributes[path.attribute];
}

- (void)_setCachedAttributeValue:(MTRDeviceDataValueDictionary _Nullable)value forPath:(MTRAttributePath *)path fromSubscription:(BOOL)isFromSubscription
{
    os_unfair_lock_assert_owner(&self->_lock);

    // We need an actual MTRClusterPath, not a subclass, to do _clusterDataForPath.
    auto * clusterPath = [MTRClusterPath clusterPathWithEndpointID:path.endpoint clusterID:path.cluster];

    MTRDeviceClusterData * clusterData = [self _clusterDataForPath:clusterPath];
    if (clusterData == nil) {
        if (value == nil) {
            // Nothing to do.
            return;
        }

        clusterData = [[MTRDeviceClusterData alloc] init];
    }

    [clusterData storeValue:value forAttribute:path.attribute];

    if ([self _attributePathAffectsDescriptionData:path]) {
        [self _updateAttributeDependentDescriptionData];
    }

    if (value != nil
        && isFromSubscription
        && !_receivingPrimingReport
        && AttributeHasChangesOmittedQuality(path)) {
        // Do not persist new values for Changes Omitted Quality (aka C Quality)
        // attributes unless they're part of a Priming Report or from a read response.
        // (removals are OK)

        // log when a device violates expectations for Changes Omitted Quality attributes.
        using namespace chip::Tracing::DarwinFramework;
        MATTER_LOG_METRIC_BEGIN(kMetricUnexpectedCQualityUpdate);
        [self _addInformationalAttributesToCurrentMetricScope];
        MATTER_LOG_METRIC_END(kMetricUnexpectedCQualityUpdate);

        return;
    }

    if (_clusterDataToPersist == nil) {
        _clusterDataToPersist = [NSMutableDictionary dictionary];
    }
    _clusterDataToPersist[clusterPath] = clusterData;
}

- (void)_removeCachedAttribute:(NSNumber *)attributeID fromCluster:(MTRClusterPath *)clusterPath
{
    os_unfair_lock_assert_owner(&self->_lock);

    if (_clusterDataToPersist == nil) {
        return;
    }
    auto * clusterData = _clusterDataToPersist[clusterPath];
    [clusterData removeValueForAttribute:attributeID];
}

- (void)_createDataVersionFilterListFromDictionary:(NSDictionary<MTRClusterPath *, NSNumber *> *)dataVersions dataVersionFilterList:(DataVersionFilter **)dataVersionFilterList count:(size_t *)count
{
    size_t dataVersionFilterSize = dataVersions.count;

    // Check if any filter list should be generated
    if (dataVersionFilterSize == 0) {
        *count = 0;
        *dataVersionFilterList = nullptr;
        return;
    }

    DataVersionFilter * dataVersionFilterArray = new DataVersionFilter[dataVersionFilterSize];
    size_t i = 0;
    for (MTRClusterPath * path in dataVersions) {
        NSNumber * dataVersionNumber = dataVersions[path];
        dataVersionFilterArray[i++] = DataVersionFilter(static_cast<chip::EndpointId>(path.endpoint.unsignedShortValue), static_cast<chip::ClusterId>(path.cluster.unsignedLongValue), static_cast<chip::DataVersion>(dataVersionNumber.unsignedLongValue));
    }

    *dataVersionFilterList = dataVersionFilterArray;
    *count = dataVersionFilterSize;
}

- (void)_setupConnectivityMonitoring
{
#if ENABLE_CONNECTIVITY_MONITORING
    // Dispatch to own queue because we used to need to do that to get the compressedFabricID, but
    // at this point that's not really needed anymore.
    mtr_weakify(self);
    dispatch_async(self.queue, ^{
        mtr_strongify(self);
        VerifyOrReturn(self, MTR_LOG_DEBUG("_setupConnectivityMonitoring dispatch to device queue called back with nil MTRDevice"));
        // Get the required info before setting up the connectivity monitor
        NSNumber * compressedFabricID = [self->_deviceController compressedFabricID];
        if (!compressedFabricID) {
            MTR_LOG_ERROR("%@ could not get compressed fabricID", self);
            return;
        }

        // Now lock for _connectivityMonitor
        std::lock_guard lock(self->_lock);
        if (self->_connectivityMonitor) {
            // already monitoring
            return;
        }

        self->_connectivityMonitor = [[MTRDeviceConnectivityMonitor alloc] initWithCompressedFabricID:compressedFabricID nodeID:self.nodeID];
        [self->_connectivityMonitor startMonitoringWithHandler:^{
            mtr_strongify(self);
            VerifyOrReturn(self, MTR_LOG_DEBUG("_setupConnectivityMonitoring startMonitoringWithHandler called back with nil MTRDevice"));
            [self->_deviceController asyncDispatchToMatterQueue:^{
                mtr_strongify(self);
                VerifyOrReturn(self, MTR_LOG_DEBUG("_setupConnectivityMonitoring asyncDispatchToMatterQueue called back with nil MTRDevice"));
                [self _triggerResubscribeWithReason:@"device connectivity changed" nodeLikelyReachable:YES];
            }
                                                   errorHandler:nil];
        } queue:self.queue];
    });
#endif
}

- (void)_stopConnectivityMonitoring
{
    os_unfair_lock_assert_owner(&_lock);

    if (_connectivityMonitor) {
        [_connectivityMonitor stopMonitoring];
        _connectivityMonitor = nil;
    }
}

- (void)_resetSubscriptionWithReasonString:(NSString *)reasonString
{
    os_unfair_lock_assert_owner(&self->_lock);
    MTR_LOG_ERROR("%@ %@ - resetting subscription", self, reasonString);

    mtr_weakify(self);
    [_deviceController asyncDispatchToMatterQueue:^{
        mtr_strongify(self);
        VerifyOrReturn(self, MTR_LOG_DEBUG("_resetSubscriptionWithReasonString asyncDispatchToMatterQueue called back with nil MTRDevice"));

        MTR_LOG("%@ subscription reset disconnecting ReadClient and SubscriptionCallback", self);

        std::lock_guard lock(self->_lock);

        [self _resetSubscription];

        // Use nil reset delay so that this keeps existing backoff timing
        [self _doHandleSubscriptionReset:nil];
    }
                                     errorHandler:nil];
}

- (void)_resetSubscription
{
    assertChipStackLockedByCurrentThread();
    os_unfair_lock_assert_owner(&_lock);

    [self.matterCPPObjectsHolder clearReadClientAndDeleteSubscriptionCallback];

    [self _doHandleSubscriptionError:nil];

#ifdef DEBUG
    [self _callFirstDelegateSynchronouslyWithBlock:^(id testDelegate) {
        if ([testDelegate respondsToSelector:@selector(unitTestSubscriptionResetForDevice:)]) {
            [testDelegate unitTestSubscriptionResetForDevice:self];
        }
    }];
#endif
}

#ifdef DEBUG
- (void)unitTestResetSubscription
{
    std::lock_guard lock(self->_lock);
    [self _resetSubscriptionWithReasonString:@"Unit test reset subscription"];
}
#endif

// assume lock is held
- (void)_setupSubscriptionWithReason:(NSString *)reason
{
    assertChipStackLockedByCurrentThread();

    os_unfair_lock_assert_owner(&self->_lock);

    // If we have a pending subscription reattempt, make sure it does not
    // actually happen, since we are trying to do a subscription now.
    self.reattemptingSubscription = NO;

    if (![self _subscriptionsAllowed]) {
        MTR_LOG("%@ _setupSubscription: Subscriptions not allowed. Do not set up subscription (reason: %@)", self, reason);
        [self _clearSubscriptionPoolWork];
        return;
    }

#ifdef DEBUG
    __block NSNumber * delegateMin = nil;
    Optional<System::Clock::Seconds32> maxIntervalOverride;
    [self _callFirstDelegateSynchronouslyWithBlock:^(id testDelegate) {
        if ([testDelegate respondsToSelector:@selector(unitTestMaxIntervalOverrideForSubscription:)]) {
            delegateMin = [testDelegate unitTestMaxIntervalOverrideForSubscription:self];
        }
    }];
    if (delegateMin) {
        maxIntervalOverride.Emplace(delegateMin.unsignedIntValue);
    }
#endif

    // for now just subscribe once
    if (!NeedToStartSubscriptionSetup(_internalDeviceState)) {
        MTR_LOG("%@ setupSubscription: no need to subscribe due to internal state %lu (reason: %@)", self, static_cast<unsigned long>(_internalDeviceState), reason);
        [self _clearSubscriptionPoolWork];
        return;
    }

    [self _changeInternalState:MTRInternalDeviceStateSubscribing];

    MTR_LOG("%@ setting up subscription with reason: %@", self, reason);
    if ([reason hasPrefix:sDeviceMayBeReachableReason]) {
        self.doingCASEAttemptForDeviceMayBeReachable = YES;
    }

    __block bool markUnreachableAfterWait = true;
#ifdef DEBUG
    [self _callFirstDelegateSynchronouslyWithBlock:^(id testDelegate) {
        if ([testDelegate respondsToSelector:@selector(unitTestSuppressTimeBasedReachabilityChanges:)]) {
            markUnreachableAfterWait = ![testDelegate unitTestSuppressTimeBasedReachabilityChanges:self];
        }
    }];
#endif

    if (markUnreachableAfterWait) {
        // Set up a timer to mark as not reachable if it takes too long to set up a subscription
        mtr_weakify(self);
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(kSecondsToWaitBeforeMarkingUnreachableAfterSettingUpSubscription) * static_cast<int64_t>(NSEC_PER_SEC)), self.queue, ^{
            mtr_strongify(self);
            VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason markUnreachableAfterWait called back with nil MTRDevice"));

            if (!HaveSubscriptionEstablishedRightNow(self->_internalDeviceState)) {
                [self _markDeviceAsUnreachableIfNeverSubscribed];
            }
        });
    }

    // This marks begin of initial subscription to the device (before CASE is established). The end is only marked after successfully setting
    // up the subscription since it is always retried as long as the MTRDevice is kept running.
    MATTER_LOG_METRIC_BEGIN(kMetricMTRDeviceInitialSubscriptionSetup);

    // Reference the object holder directly
    auto matterCPPObjectsHolder = self.matterCPPObjectsHolder;
    // Call directlyGetSessionForNode because the subscription setup already goes through the subscription pool queue
    mtr_weakify(self);
    [self._concreteController
        directlyGetSessionForNode:_nodeID.unsignedLongLongValue
                       completion:^(chip::Messaging::ExchangeManager * _Nullable exchangeManager,
                           const chip::Optional<chip::SessionHandle> & session, NSError * _Nullable error,
                           NSNumber * _Nullable retryDelay) {
                           mtr_strongify(self);
                           VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason directlyGetSessionForNode called back with nil MTRDevice"));

                           self.doingCASEAttemptForDeviceMayBeReachable = NO;

                           if (error != nil) {
                               MTR_LOG_ERROR("%@ getSessionForNode error %@", self, error);
                               [self->_deviceController asyncDispatchToMatterQueue:^{
                                   mtr_strongify(self);
                                   VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason asyncDispatchToMatterQueue called back with nil MTRDevice"));

                                   [self _handleSubscriptionError:error];
                                   [self _handleSubscriptionReset:retryDelay];
                               } errorHandler:nil];
                               return;
                           }

                           auto callback = std::make_unique<SubscriptionCallback>(
                               ^(NSArray * value) {
                                   mtr_strongify(self);
                                   VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason subscription attribute report called back with nil MTRDevice"));
                                   {
                                       std::lock_guard lock(self->_lock);
                                       self.lastSubscriptionActiveTime = [NSDate now];
                                   }

                                   MTR_LOG("%@ got attribute report (%p) %@", self, value, value);
                                   dispatch_async(self.queue, ^{
                                       mtr_strongify(self);
                                       VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason subscription attribute report async to device queue called back with nil MTRDevice"));

                                       // OnAttributeData
                                       [self _handleAttributeReport:value fromSubscription:YES];
#ifdef DEBUG
                                       self->_unitTestAttributesReportedSinceLastCheck += value.count;
#endif
                                   });
                               },
                               ^(NSArray * value) {
                                   mtr_strongify(self);
                                   VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason subscription event report called back with nil MTRDevice"));
                                   {
                                       std::lock_guard lock(self->_lock);
                                       self.lastSubscriptionActiveTime = [NSDate now];
                                   }

                                   MTR_LOG("%@ got event report %@", self, value);
                                   dispatch_async(self.queue, ^{
                                       mtr_strongify(self);
                                       VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason subscription event report async to device queue called back with nil MTRDevice"));

                                       // OnEventReport
                                       [self _handleEventReport:value];
                                   });
                               },
                               ^(NSError * error) {
                                   mtr_strongify(self);
                                   VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason subscription error called back with nil MTRDevice"));

                                   MTR_LOG_ERROR("%@ got subscription error %@", self, error);
                                   // OnError
                                   [self _handleSubscriptionError:error];
                               },
                               ^(NSError * error, NSNumber * resubscriptionDelayMs) {
                                   mtr_strongify(self);
                                   VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason subscription resubscription needed called back with nil MTRDevice"));

                                   MTR_LOG_ERROR("%@ got resubscription error %@ delay %@", self, error, resubscriptionDelayMs);
                                   // OnResubscriptionNeeded
                                   [self _handleResubscriptionNeededWithDelay:resubscriptionDelayMs];
                               },
                               ^(void) {
                                   mtr_strongify(self);
                                   VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason subscription established called back with nil MTRDevice"));

                                   MTR_LOG("%@ got subscription established", self);
                                   std::lock_guard lock(self->_lock);
                                   self.lastSubscriptionActiveTime = [NSDate now];

                                   // First synchronously change state
                                   if (HadSubscriptionEstablishedOnce(self->_internalDeviceState)) {
                                       [self _changeInternalState:MTRInternalDeviceStateLaterSubscriptionEstablished];
                                   } else {
                                       MATTER_LOG_METRIC_END(kMetricMTRDeviceInitialSubscriptionSetup, CHIP_NO_ERROR);
                                       [self _changeInternalState:MTRInternalDeviceStateInitialSubscriptionEstablished];
                                   }

                                   [self _changeState:MTRDeviceStateReachable];

                                   // Then async work that shouldn't be performed on the matter queue
                                   dispatch_async(self.queue, ^{
                                       mtr_strongify(self);
                                       VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason subscription established async to device queue called back with nil MTRDevice"));

                                       // OnSubscriptionEstablished
                                       [self _handleSubscriptionEstablished];
                                   });
                               },
                               ^(void) {
                                   mtr_strongify(self);
                                   VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason subscription done called back with nil MTRDevice"));

                                   MTR_LOG("%@ got subscription done", self);
                                   // Drop our pointer to the ReadClient immediately, since
                                   // it's about to be destroyed and we don't want to be
                                   // holding a dangling pointer.
                                   [matterCPPObjectsHolder setReadClient:nullptr subscriptionCallback:nullptr];

                                   // OnDone
                                   std::lock_guard lock(self->_lock);
                                   [self _doHandleSubscriptionReset:nil];
                               },
                               ^(void) {
                                   mtr_strongify(self);
                                   VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason subscription unsolicited message called back with nil MTRDevice"));

                                   MTR_LOG("%@ got unsolicited message from publisher", self);
                                   // OnUnsolicitedMessageFromPublisher
                                   [self _handleUnsolicitedMessageFromPublisher];
                               },
                               ^(void) {
                                   mtr_strongify(self);
                                   VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason subscription report begin called back with nil MTRDevice"));
                                   {
                                       std::lock_guard lock(self->_lock);
                                       self.lastSubscriptionActiveTime = [NSDate now];
                                   }

                                   MTR_LOG("%@ got report begin", self);
                                   [self _handleReportBegin];
                               },
                               ^(void) {
                                   mtr_strongify(self);
                                   VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason subscription report end called back with nil MTRDevice"));

                                   MTR_LOG("%@ got report end", self);
                                   dispatch_async(self.queue, ^{
                                       mtr_strongify(self);
                                       VerifyOrReturn(self, MTR_LOG_DEBUG("_setupSubscriptionWithReason subscription report end async to device queue called back with nil MTRDevice"));

                                       [self _handleReportEnd];
                                   });
                               });

                           // Set up a cluster state cache.  We just want this for the logic it has for
                           // tracking data versions and event numbers so we minimize the amount of data we
                           // request on resubscribes, so tell it not to store data.
                           Optional<EventNumber> highestObservedEventNumber;
                           {
                               std::lock_guard lock(self->_lock);
                               if (self.highestObservedEventNumber) {
                                   highestObservedEventNumber = MakeOptional(self.highestObservedEventNumber.unsignedLongLongValue);
                               }
                           }
                           auto clusterStateCache = std::make_unique<ClusterStateCache>(*callback.get(),
                               highestObservedEventNumber,
                               /* cacheData = */ false);
                           auto readClient = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), exchangeManager,
                               clusterStateCache->GetBufferedCallback(), ReadClient::InteractionType::Subscribe);

                           // Wildcard endpoint, cluster, attribute, event.
                           auto attributePath = std::make_unique<AttributePathParams>();
                           auto eventPath = std::make_unique<EventPathParams>();
                           // We want to get event reports at the minInterval, not the maxInterval.
                           eventPath->mIsUrgentEvent = true;
                           ReadPrepareParams readParams(session.Value());

                           readParams.mMinIntervalFloorSeconds = 0;
                           // Select a max interval based on the device's claimed idle sleep interval.
                           auto idleSleepInterval = std::chrono::duration_cast<System::Clock::Seconds32>(
                               session.Value()->GetRemoteMRPConfig().mIdleRetransTimeout);

                           auto maxIntervalCeilingMin = System::Clock::Seconds32(MTR_DEVICE_SUBSCRIPTION_MAX_INTERVAL_MIN);
                           if (idleSleepInterval < maxIntervalCeilingMin) {
                               idleSleepInterval = maxIntervalCeilingMin;
                           }

                           auto maxIntervalCeilingMax = System::Clock::Seconds32(MTR_DEVICE_SUBSCRIPTION_MAX_INTERVAL_MAX);
                           if (idleSleepInterval > maxIntervalCeilingMax) {
                               idleSleepInterval = maxIntervalCeilingMax;
                           }
#ifdef DEBUG
                           if (maxIntervalOverride.HasValue()) {
                               idleSleepInterval = maxIntervalOverride.Value();
                           }
#endif
                           readParams.mMaxIntervalCeilingSeconds = static_cast<uint16_t>(idleSleepInterval.count());

                           readParams.mpAttributePathParamsList = attributePath.get();
                           readParams.mAttributePathParamsListSize = 1;
                           readParams.mpEventPathParamsList = eventPath.get();
                           readParams.mEventPathParamsListSize = 1;
                           readParams.mKeepSubscriptions = true;
                           readParams.mIsFabricFiltered = false;

                           // Subscribe with data version filter list from our cache.
                           size_t dataVersionFilterListSize = 0;
                           DataVersionFilter * dataVersionFilterList;
                           [self _createDataVersionFilterListFromDictionary:[self _getCachedDataVersions] dataVersionFilterList:&dataVersionFilterList count:&dataVersionFilterListSize];

                           readParams.mDataVersionFilterListSize = dataVersionFilterListSize;
                           readParams.mpDataVersionFilterList = dataVersionFilterList;
                           attributePath.release();
                           eventPath.release();

                           // NOTE: We don't set the event number field in readParams, and just let
                           // the ReadClient get the min event number information from the cluster
                           // state cache.

                           // TODO: Change from local filter list generation to rehydrating ClusterStateCache to take advantage of existing filter list sorting algorithm

                           // SendAutoResubscribeRequest cleans up the params, even on failure.
                           CHIP_ERROR err = readClient->SendAutoResubscribeRequest(std::move(readParams));
                           if (err != CHIP_NO_ERROR) {
                               NSError * error = [MTRError errorForCHIPErrorCode:err logContext:self];
                               MTR_LOG_ERROR("%@ SendAutoResubscribeRequest error %@", self, error);
                               [self _handleSubscriptionError:error];
                               [self _handleSubscriptionReset:nil];

                               return;
                           }

                           MTR_LOG("%@ Subscribe with data version list size %lu", self, static_cast<unsigned long>(dataVersionFilterListSize));

                           // Callback and ClusterStateCache and ReadClient will be deleted
                           // when OnDone is called.
                           [matterCPPObjectsHolder setReadClient:readClient.get() subscriptionCallback:callback.get()];
                           callback->AdoptReadClient(std::move(readClient));
                           callback->AdoptClusterStateCache(std::move(clusterStateCache));
                           callback.release();
                       }];

    // Set up connectivity monitoring in case network becomes routable after any part of the subscription process goes into backoff retries.
    [self _setupConnectivityMonitoring];
}

#ifdef DEBUG
- (NSUInteger)unitTestAttributesReportedSinceLastCheck
{
    NSUInteger attributesReportedSinceLastCheck = _unitTestAttributesReportedSinceLastCheck;
    _unitTestAttributesReportedSinceLastCheck = 0;
    return attributesReportedSinceLastCheck;
}
#endif

#pragma mark Device Interactions

// Helper function to determine whether an attribute has "Changes Omitted" quality, which indicates that past the priming report in
// a subscription, this attribute is not expected to be reported when its value changes
//   * TODO: xml+codegen version to replace this hardcoded list.
static BOOL AttributeHasChangesOmittedQuality(MTRAttributePath * attributePath)
{
    switch (attributePath.cluster.unsignedLongValue) {
    case MTRClusterEthernetNetworkDiagnosticsID:
        switch (attributePath.attribute.unsignedLongValue) {
        case MTRClusterEthernetNetworkDiagnosticsAttributePacketRxCountID:
        case MTRClusterEthernetNetworkDiagnosticsAttributePacketTxCountID:
        case MTRClusterEthernetNetworkDiagnosticsAttributeTxErrCountID:
        case MTRClusterEthernetNetworkDiagnosticsAttributeCollisionCountID:
        case MTRClusterEthernetNetworkDiagnosticsAttributeOverrunCountID:
        case MTRClusterEthernetNetworkDiagnosticsAttributeCarrierDetectID:
        case MTRClusterEthernetNetworkDiagnosticsAttributeTimeSinceResetID:
            return YES;
        default:
            return NO;
        }
    case MTRClusterGeneralDiagnosticsID:
        switch (attributePath.attribute.unsignedLongValue) {
        case MTRClusterGeneralDiagnosticsAttributeUpTimeID:
        case MTRClusterGeneralDiagnosticsAttributeTotalOperationalHoursID:
            return YES;
        default:
            return NO;
        }
    case MTRClusterThreadNetworkDiagnosticsID:
        switch (attributePath.attribute.unsignedLongValue) {
        case MTRClusterThreadNetworkDiagnosticsAttributeOverrunCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeDetachedRoleCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeChildRoleCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRouterRoleCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeLeaderRoleCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeAttachAttemptCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributePartitionIdChangeCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeBetterPartitionAttachAttemptCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeParentChangeCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxTotalCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxUnicastCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxBroadcastCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxAckRequestedCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxAckedCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxNoAckRequestedCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxDataCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxDataPollCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxBeaconCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxBeaconRequestCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxOtherCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxRetryCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxDirectMaxRetryExpiryCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxIndirectMaxRetryExpiryCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxErrCcaCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxErrAbortCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeTxErrBusyChannelCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxTotalCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxUnicastCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxBroadcastCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxDataCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxDataPollCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxBeaconCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxBeaconRequestCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxOtherCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxAddressFilteredCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxDestAddrFilteredCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxDuplicatedCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxErrNoFrameCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxErrUnknownNeighborCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxErrInvalidSrcAddrCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxErrSecCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxErrFcsCountID:
        case MTRClusterThreadNetworkDiagnosticsAttributeRxErrOtherCountID:
            return YES;
        default:
            return NO;
        }
    case MTRClusterWiFiNetworkDiagnosticsID:
        switch (attributePath.attribute.unsignedLongValue) {
        case MTRClusterWiFiNetworkDiagnosticsAttributeRssiID:
        case MTRClusterWiFiNetworkDiagnosticsAttributeBeaconLostCountID:
        case MTRClusterWiFiNetworkDiagnosticsAttributeBeaconRxCountID:
        case MTRClusterWiFiNetworkDiagnosticsAttributePacketMulticastRxCountID:
        case MTRClusterWiFiNetworkDiagnosticsAttributePacketMulticastTxCountID:
        case MTRClusterWiFiNetworkDiagnosticsAttributePacketUnicastRxCountID:
        case MTRClusterWiFiNetworkDiagnosticsAttributePacketUnicastTxCountID:
        case MTRClusterWiFiNetworkDiagnosticsAttributeCurrentMaxRateID:
        case MTRClusterWiFiNetworkDiagnosticsAttributeOverrunCountID:
            return YES;
        default:
            return NO;
        }
    case MTRClusterOperationalCredentialsID:
        switch (attributePath.attribute.unsignedLongValue) {
        case MTRClusterOperationalCredentialsAttributeNOCsID:
        case MTRClusterOperationalCredentialsAttributeTrustedRootCertificatesID:
            return YES;
        default:
            return NO;
        }
    case MTRClusterPowerSourceID:
        switch (attributePath.attribute.unsignedLongValue) {
        case MTRClusterPowerSourceAttributeWiredAssessedInputVoltageID:
        case MTRClusterPowerSourceAttributeWiredAssessedInputFrequencyID:
        case MTRClusterPowerSourceAttributeWiredAssessedCurrentID:
        case MTRClusterPowerSourceAttributeBatVoltageID:
        case MTRClusterPowerSourceAttributeBatPercentRemainingID:
        case MTRClusterPowerSourceAttributeBatTimeRemainingID:
        case MTRClusterPowerSourceAttributeBatTimeToFullChargeID:
        case MTRClusterPowerSourceAttributeBatChargingCurrentID:
            return YES;
        default:
            return NO;
        }
    case MTRClusterTimeSynchronizationID:
        switch (attributePath.attribute.unsignedLongValue) {
        case MTRClusterTimeSynchronizationAttributeUTCTimeID:
        case MTRClusterTimeSynchronizationAttributeLocalTimeID:
            return YES;
        default:
            return NO;
        }
    default:
        return NO;
    }
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWithEndpointID:(NSNumber *)endpointID
                                                              clusterID:(NSNumber *)clusterID
                                                            attributeID:(NSNumber *)attributeID
                                                                 params:(MTRReadParams * _Nullable)params
{
    MTRAttributePath * attributePath = [MTRAttributePath attributePathWithEndpointID:endpointID
                                                                           clusterID:clusterID
                                                                         attributeID:attributeID];

    BOOL attributeIsSpecified = MTRAttributeIsSpecified(clusterID.unsignedIntValue, attributeID.unsignedIntValue);
    BOOL hasChangesOmittedQuality;
    if (attributeIsSpecified) {
        hasChangesOmittedQuality = AttributeHasChangesOmittedQuality(attributePath);
    } else {
        if (params == nil) {
            hasChangesOmittedQuality = NO;
        } else {
            hasChangesOmittedQuality = !params.assumeUnknownAttributesReportable;
        }
    }

    // Return current known / expected value right away
    NSDictionary<NSString *, id> * attributeValueToReturn = [self _attributeValueDictionaryForAttributePath:attributePath];

    // Send read request to device if any of the following are true:
    // 1. Subscription not in a state we can expect reports
    // 2. The attribute has the Changes Omitted quality, so we won't get reports for it.
    // 3. The attribute is not in the spec, and the read params asks to assume
    //    an unknown attribute has the Changes Omitted quality.
    //
    // But all this only happens if this device is not suspended.  If it's suspended, read-throughs will fail
    // anyway, so we should not bother trying.
    BOOL readThroughsAllowed;
    {
        std::lock_guard lock(_lock);
        readThroughsAllowed = !self.suspended;
    }
    if (readThroughsAllowed && (![self _subscriptionAbleToReport] || hasChangesOmittedQuality)) {
        // Read requests container will be a mutable array of items, each being an array containing:
        //   [attribute request path, params]
        // Batching handler should only coalesce when params are equal.

        // For this single read API there's only 1 array item. Use NSNull to stand in for nil params for easy comparison.
        MTRAttributeRequestPath * readRequestPath = [MTRAttributeRequestPath requestPathWithEndpointID:endpointID
                                                                                             clusterID:clusterID
                                                                                           attributeID:attributeID];
        NSArray * readRequestData = @[ readRequestPath, params ?: [NSNull null] ];

        // But first, check if a duplicate read request is already queued and return
        if ([_asyncWorkQueue hasDuplicateForTypeID:MTRDeviceWorkItemDuplicateReadTypeID workItemData:readRequestData]) {
            return attributeValueToReturn;
        }

        NSMutableArray<NSArray *> * readRequests = [NSMutableArray arrayWithObject:readRequestData];

        // Create work item, set ready handler to perform task, then enqueue the work
        MTRAsyncWorkItem * workItem = [[MTRAsyncWorkItem alloc] initWithQueue:self.queue];
        uint64_t workItemID = workItem.uniqueID; // capture only the ID, not the work item
        NSNumber * nodeID = [self nodeID];

        [workItem setBatchingID:MTRDeviceWorkItemBatchingReadID data:readRequests handler:^(id opaqueDataCurrent, id opaqueDataNext) {
            mtr_hide(self); // don't capture self accidentally
            NSMutableArray<NSArray *> * readRequestsCurrent = opaqueDataCurrent;
            NSMutableArray<NSArray *> * readRequestsNext = opaqueDataNext;

            MTRBatchingOutcome outcome = MTRNotBatched;
            while (readRequestsNext.count) {
                // Can only read up to 9 paths at a time, per spec
                if (readRequestsCurrent.count >= 9) {
                    MTR_LOG("Batching read attribute work item [%llu]: cannot add more work, item is full [0x%016llX:%@:0x%llx:0x%llx]", workItemID, nodeID.unsignedLongLongValue, endpointID, clusterID.unsignedLongLongValue, attributeID.unsignedLongLongValue);
                    return outcome;
                }

                // if params don't match then they cannot be merged
                if (![readRequestsNext[0][MTRDeviceReadRequestFieldParamsIndex]
                        isEqual:readRequestsCurrent[0][MTRDeviceReadRequestFieldParamsIndex]]) {
                    MTR_LOG("Batching read attribute work item [%llu]: cannot add more work, parameter mismatch [0x%016llX:%@:0x%llx:0x%llx]", workItemID, nodeID.unsignedLongLongValue, endpointID, clusterID.unsignedLongLongValue, attributeID.unsignedLongLongValue);
                    return outcome;
                }

                // merge the next item's first request into the current item's list
                auto readItem = readRequestsNext.firstObject;
                [readRequestsNext removeObjectAtIndex:0];
                [readRequestsCurrent addObject:readItem];
                MTR_LOG("Batching read attribute work item [%llu]: added %@ (now %lu requests total) [0x%016llX:%@:0x%llx:0x%llx]",
                    workItemID, readItem, static_cast<unsigned long>(readRequestsCurrent.count), nodeID.unsignedLongLongValue, endpointID, clusterID.unsignedLongLongValue, attributeID.unsignedLongLongValue);
                outcome = MTRBatchedPartially;
            }
            NSCAssert(readRequestsNext.count == 0, @"should have batched everything or returned early");
            return MTRBatchedFully;
        }];
        [workItem setDuplicateTypeID:MTRDeviceWorkItemDuplicateReadTypeID handler:^(id opaqueItemData, BOOL * isDuplicate, BOOL * stop) {
            mtr_hide(self); // don't capture self accidentally
            for (NSArray * readItem in readRequests) {
                if ([readItem isEqual:opaqueItemData]) {
                    MTR_LOG("Read attribute work item [%llu] report duplicate %@ [0x%016llX:%@:0x%llx:0x%llx]", workItemID, readItem, nodeID.unsignedLongLongValue, endpointID, clusterID.unsignedLongLongValue, attributeID.unsignedLongLongValue);
                    *isDuplicate = YES;
                    *stop = YES;
                    return;
                }
            }
            *stop = NO;
        }];
        [workItem setReadyHandler:^(MTRDevice_Concrete * self, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
            // Sanity check
            if (readRequests.count == 0) {
                MTR_LOG_ERROR("Read attribute work item [%llu] contained no read requests", workItemID);
                completion(MTRAsyncWorkComplete);
                return;
            }

            // Build the attribute paths from the read requests
            NSMutableArray<MTRAttributeRequestPath *> * attributePaths = [NSMutableArray array];
            for (NSArray * readItem in readRequests) {
                NSAssert(readItem.count == 2, @"invalid read attribute item");
                [attributePaths addObject:readItem[MTRDeviceReadRequestFieldPathIndex]];
            }
            // If param is the NSNull stand-in, then just use nil
            id readParamObject = readRequests[0][MTRDeviceReadRequestFieldParamsIndex];
            MTRReadParams * readParams = (![readParamObject isEqual:[NSNull null]]) ? readParamObject : nil;

            MTRBaseDevice * baseDevice = [self newBaseDevice];
            mtr_weakify(self);
            [baseDevice
                readAttributePaths:attributePaths
                        eventPaths:nil
                            params:readParams
                includeDataVersion:YES
                             queue:self.queue
                        completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                            mtr_strongify(self);
                            VerifyOrReturn(self, MTR_LOG_DEBUG("readAttributeWithEndpointID base device completion called back with nil MTRDevice"));
                            if (values) {
                                // Since the format is the same data-value dictionary, this looks like an
                                // attribute report
                                MTR_LOG("Read attribute work item [%llu] result: (%p) %@  [0x%016llX:%@:0x%llX:0x%llX]", workItemID, values, values, nodeID.unsignedLongLongValue, endpointID, clusterID.unsignedLongLongValue, attributeID.unsignedLongLongValue);
                                [self _handleAttributeReport:values fromSubscription:NO];
                            }

                            // TODO: better retry logic
                            if (error && (retryCount < 2)) {
                                MTR_LOG_ERROR("Read attribute work item [%llu] failed (will retry): %@   [0x%016llX:%@:0x%llx:0x%llx]", workItemID, error, nodeID.unsignedLongLongValue, endpointID, clusterID.unsignedLongLongValue, attributeID.unsignedLongLongValue);
                                completion(MTRAsyncWorkNeedsRetry);
                            } else {
                                if (error) {
                                    MTR_LOG("Read attribute work item [%llu] failed (giving up): %@   [0x%016llX:%@:0x%llx:0x%llx]", workItemID, error, nodeID.unsignedLongLongValue, endpointID, clusterID.unsignedLongLongValue, attributeID.unsignedLongLongValue);
                                }
                                completion(MTRAsyncWorkComplete);
                            }
                        }];
        }];
        [_asyncWorkQueue enqueueWorkItem:workItem descriptionWithFormat:@"read %@ 0x%llx (%@) 0x%llx (%@)",
                                                  endpointID,
                                                  clusterID.unsignedLongLongValue, MTRClusterNameForID(static_cast<MTRClusterIDType>(clusterID.unsignedLongLongValue)),
                                                  attributeID.unsignedLongLongValue, MTRAttributeNameForID(static_cast<MTRClusterIDType>(clusterID.unsignedLongLongValue), static_cast<MTRAttributeIDType>(attributeID.unsignedLongLongValue))];
    } else {
        [self _readThroughSkipped];
    }

    return attributeValueToReturn;
}

- (void)writeAttributeWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                         attributeID:(NSNumber *)attributeID
                               value:(id)value
               expectedValueInterval:(NSNumber *)expectedValueInterval
                   timedWriteTimeout:(NSNumber * _Nullable)timeout
{
    value = [value copy];

    if (timeout) {
        timeout = MTRClampedNumber(timeout, @(1), @(UINT16_MAX));
    }
    expectedValueInterval = MTRClampedNumber(expectedValueInterval, @(1), @(UINT32_MAX));
    MTRAttributePath * attributePath = [MTRAttributePath attributePathWithEndpointID:endpointID
                                                                           clusterID:clusterID

                                                                         attributeID:attributeID];

    __block BOOL useValueAsExpectedValue = YES;
#ifdef DEBUG
    os_unfair_lock_lock(&self->_lock);
    [self _callFirstDelegateSynchronouslyWithBlock:^(id delegate) {
        if ([delegate respondsToSelector:@selector(unitTestShouldSkipExpectedValuesForWrite:)]) {
            useValueAsExpectedValue = ![delegate unitTestShouldSkipExpectedValuesForWrite:self];
        }
    }];
    os_unfair_lock_unlock(&self->_lock);
#endif

    uint64_t expectedValueID = 0;
    if (useValueAsExpectedValue) {
        // Commit change into expected value cache
        NSDictionary * newExpectedValueDictionary = @{ MTRAttributePathKey : attributePath, MTRDataKey : value };
        [self setExpectedValues:@[ newExpectedValueDictionary ]
            expectedValueInterval:expectedValueInterval
                  expectedValueID:&expectedValueID];
    }

    MTRAsyncWorkItem * workItem = [[MTRAsyncWorkItem alloc] initWithQueue:self.queue];
    uint64_t workItemID = workItem.uniqueID; // capture only the ID, not the work item
    NSNumber * nodeID = _nodeID;

    // Write request data is an array of items (for now always length 1).  Each
    // item is an array containing:
    //
    //   [ attribute path, value, timedWriteTimeout, expectedValueID ]
    //
    // where expectedValueID is stored as NSNumber and NSNull represents nil timeouts
    auto * writeData = @[ attributePath, value, timeout ?: [NSNull null], @(expectedValueID) ];

    NSMutableArray<NSArray *> * writeRequests = [NSMutableArray arrayWithObject:writeData];

    [workItem setBatchingID:MTRDeviceWorkItemBatchingWriteID data:writeRequests handler:^(id opaqueDataCurrent, id opaqueDataNext) {
        mtr_hide(self); // don't capture self accidentally
        NSMutableArray<NSArray *> * writeRequestsCurrent = opaqueDataCurrent;
        NSMutableArray<NSArray *> * writeRequestsNext = opaqueDataNext;

        if (writeRequestsCurrent.count != 1) {
            // Very unexpected!
            MTR_LOG_ERROR("Batching write attribute work item [%llu]: Unexpected write request count %lu", workItemID, static_cast<unsigned long>(writeRequestsCurrent.count));
            return MTRNotBatched;
        }

        MTRBatchingOutcome outcome = MTRNotBatched;
        while (writeRequestsNext.count) {
            // If paths don't match, we cannot replace the earlier write
            // with the later one.
            if (![writeRequestsNext[0][MTRDeviceWriteRequestFieldPathIndex]
                    isEqual:writeRequestsCurrent[0][MTRDeviceWriteRequestFieldPathIndex]]) {
                MTR_LOG("Batching write attribute work item [%llu]: cannot replace with next work item due to path mismatch", workItemID);
                return outcome;
            }

            // Replace our one request with the first one from the next item.
            auto writeItem = writeRequestsNext.firstObject;
            [writeRequestsNext removeObjectAtIndex:0];
            [writeRequestsCurrent replaceObjectAtIndex:0 withObject:writeItem];
            MTR_LOG("Batching write attribute work item [%llu]: replaced with new write value %@ [0x%016llX]",
                workItemID, writeItem, nodeID.unsignedLongLongValue);
            outcome = MTRBatchedPartially;
        }
        NSCAssert(writeRequestsNext.count == 0, @"should have batched everything or returned early");
        return MTRBatchedFully;
    }];
    // The write operation will install a duplicate check handler, to return NO for "isDuplicate". Since a write operation may
    // change values, only read requests after this should be considered for duplicate requests.
    [workItem setDuplicateTypeID:MTRDeviceWorkItemDuplicateReadTypeID handler:^(id opaqueItemData, BOOL * isDuplicate, BOOL * stop) {
        *isDuplicate = NO;
        *stop = YES;
    }];
    [workItem setReadyHandler:^(MTRDevice_Concrete * self, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        MTRBaseDevice * baseDevice = [self newBaseDevice];
        // Make sure to use writeRequests here, because that's what our batching
        // handler will modify as needed.
        NSCAssert(writeRequests.count == 1, @"Incorrect number of write requests: %lu", static_cast<unsigned long>(writeRequests.count));

        auto * request = writeRequests[0];
        MTRAttributePath * path = request[MTRDeviceWriteRequestFieldPathIndex];

        id timedWriteTimeout = request[MTRDeviceWriteRequestFieldTimeoutIndex];
        if (timedWriteTimeout == [NSNull null]) {
            timedWriteTimeout = nil;
        }

        mtr_weakify(self);
        [baseDevice
            _writeAttributeWithEndpointID:path.endpoint
                                clusterID:path.cluster
                              attributeID:path.attribute
                                    value:request[MTRDeviceWriteRequestFieldValueIndex]
                        timedWriteTimeout:timedWriteTimeout
                                    queue:self.queue
                               completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                   mtr_strongify(self);
                                   VerifyOrReturn(self, MTR_LOG_DEBUG("writeAttributeWithEndpointID base device completion called back with nil MTRDevice"));
                                   if (error) {
                                       MTR_LOG_ERROR("Write attribute work item [%llu] failed: %@", workItemID, error);
                                       if (useValueAsExpectedValue) {
                                           NSNumber * expectedValueID = request[MTRDeviceWriteRequestFieldExpectedValueIDIndex];
                                           [self removeExpectedValueForAttributePath:attributePath expectedValueID:expectedValueID.unsignedLongLongValue];
                                       }
                                   }
                                   completion(MTRAsyncWorkComplete);
                               }];
    }];
    [_asyncWorkQueue enqueueWorkItem:workItem descriptionWithFormat:@"write %@ 0x%llx (%@) 0x%llx (%@): %@",
                                              endpointID,
                                              clusterID.unsignedLongLongValue, MTRClusterNameForID(static_cast<MTRClusterIDType>(clusterID.unsignedLongLongValue)),
                                              attributeID.unsignedLongLongValue, MTRAttributeNameForID(static_cast<MTRClusterIDType>(clusterID.unsignedLongLongValue), static_cast<MTRAttributeIDType>(attributeID.unsignedLongLongValue)),
                                              value];
}

- (NSArray<NSDictionary<NSString *, id> *> *)readAttributePaths:(NSArray<MTRAttributeRequestPath *> *)attributePaths
{
    // Determine the set of what the spec calls "existent paths" that correspond
    // to the request paths.  Building the whole set in-memory is OK, because
    // we're going to need all those paths for our return value anyway.
    //
    // Note that we don't use the structural attributes (PartsList, ServerList,
    // AttributeList) to determine this set, because we might be in the middle
    // of priming right now and have not gotten those yet.  Just use the set of
    // attribute paths we actually have.
    NSMutableSet<MTRAttributePath *> * existentPaths = [[NSMutableSet alloc] init];
    {
        std::lock_guard lock(_lock);
        for (MTRAttributeRequestPath * requestPath in attributePaths) {
            for (MTRClusterPath * clusterPath in [self _knownClusters]) {
                if (requestPath.endpoint != nil && ![requestPath.endpoint isEqual:clusterPath.endpoint]) {
                    continue;
                }
                if (requestPath.cluster != nil && ![requestPath.cluster isEqual:clusterPath.cluster]) {
                    continue;
                }
                MTRDeviceClusterData * clusterData = [self _clusterDataForPath:clusterPath];
                if (requestPath.attribute == nil) {
                    for (NSNumber * attributeID in clusterData.attributes) {
                        [existentPaths addObject:[MTRAttributePath attributePathWithEndpointID:clusterPath.endpoint clusterID:clusterPath.cluster attributeID:attributeID]];
                    }
                } else if ([clusterData.attributes objectForKey:requestPath.attribute] != nil) {
                    [existentPaths addObject:[MTRAttributePath attributePathWithEndpointID:clusterPath.endpoint clusterID:clusterPath.cluster attributeID:requestPath.attribute]];
                }
            }
        }
    }

    NSMutableArray<NSDictionary<NSString *, id> *> * result = [NSMutableArray arrayWithCapacity:existentPaths.count];
    for (MTRAttributePath * path in existentPaths) {
        auto * value = [self readAttributeWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute params:nil];
        if (!value) {
            continue;
        }
        [result addObject:@{
            MTRAttributePathKey : path,
            MTRDataKey : value,
        }];
    }

    return result;
}

- (void)_invokeCommandWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                           commandID:(NSNumber *)commandID
                       commandFields:(MTRDeviceDataValueDictionary)commandFields
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues
               expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval
                  timedInvokeTimeout:(NSNumber * _Nullable)timeout
         serverSideProcessingTimeout:(NSNumber * _Nullable)serverSideProcessingTimeout
                               queue:(dispatch_queue_t)queue
                          completion:(MTRDeviceResponseHandler)completion
{
    if (!expectedValueInterval || ([expectedValueInterval compare:@(0)] == NSOrderedAscending)) {
        expectedValues = nil;
    } else {
        expectedValueInterval = MTRClampedNumber(expectedValueInterval, @(1), @(UINT32_MAX));
    }

    serverSideProcessingTimeout = [serverSideProcessingTimeout copy];
    timeout = [timeout copy];
    commandFields = [commandFields copy];

    if (timeout == nil && MTRCommandNeedsTimedInvoke(clusterID, commandID)) {
        timeout = @(MTR_DEFAULT_TIMED_INTERACTION_TIMEOUT_MS);
    }

    NSDate * cutoffTime;
    if (timeout) {
        cutoffTime = [NSDate dateWithTimeIntervalSinceNow:(timeout.doubleValue / 1000)];
    }

    uint64_t expectedValueID = 0;
    NSMutableArray<MTRAttributePath *> * attributePaths = nil;
    if (expectedValues) {
        [self setExpectedValues:expectedValues expectedValueInterval:expectedValueInterval expectedValueID:&expectedValueID];
        attributePaths = [NSMutableArray array];
        for (NSDictionary<NSString *, id> * expectedValue in expectedValues) {
            [attributePaths addObject:expectedValue[MTRAttributePathKey]];
        }
    }
    MTRAsyncWorkItem * workItem = [[MTRAsyncWorkItem alloc] initWithQueue:self.queue];
    uint64_t workItemID = workItem.uniqueID; // capture only the ID, not the work item
    // The command operation will install a duplicate check handler, to return NO for "isDuplicate". Since a command operation may
    // change values, only read requests after this should be considered for duplicate requests.
    [workItem setDuplicateTypeID:MTRDeviceWorkItemDuplicateReadTypeID handler:^(id opaqueItemData, BOOL * isDuplicate, BOOL * stop) {
        *isDuplicate = NO;
        *stop = YES;
    }];
    [workItem setReadyHandler:^(MTRDevice_Concrete * self, NSInteger retryCount, MTRAsyncWorkCompletionBlock workCompletion) {
        auto workDone = ^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
            dispatch_async(queue, ^{
                completion(values, error);
            });
            if (error && expectedValues) {
                [self removeExpectedValuesForAttributePaths:attributePaths expectedValueID:expectedValueID];
            }
            workCompletion(MTRAsyncWorkComplete);
        };

        NSNumber * timedInvokeTimeout = nil;
        if (timeout) {
            auto * now = [NSDate now];
            if ([now compare:cutoffTime] == NSOrderedDescending) {
                // Our timed invoke timeout has expired already.  Command
                // was queued for too long.  Do not send it out.
                MTR_LOG("Invoke work item [%llu] timed out its timed invoke timeout before being dispatched", workItemID);
                workDone(nil, [MTRError errorForIMStatusCode:Status::Timeout]);
                return;
            }

            // Recompute the actual timeout left, accounting for time spent
            // in our queuing and retries.
            timedInvokeTimeout = @([cutoffTime timeIntervalSinceDate:now] * 1000);
        }
        MTRBaseDevice * baseDevice = [self newBaseDevice];
        mtr_weakify(self);
        [baseDevice
            _invokeCommandWithEndpointID:endpointID
                               clusterID:clusterID
                               commandID:commandID
                           commandFields:commandFields
                      timedInvokeTimeout:timedInvokeTimeout
             serverSideProcessingTimeout:serverSideProcessingTimeout
                                 logCall:NO
                                   queue:self.queue
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  mtr_strongify(self);
                                  VerifyOrReturn(self, MTR_LOG_DEBUG("_invokeCommandWithEndpointID base device completion called back with nil MTRDevice"));
                                  // Log the data at the INFO level (not usually persisted permanently),
                                  // but make sure we log the work completion at the DEFAULT level.
                                  MTR_LOG("Invoke work item [%llu] received command response: %@ error: %@", workItemID, values, error);
                                  // TODO: This 5-retry cap is very arbitrary.
                                  // TODO: Should there be some sort of backoff here?
                                  if (error != nil && error.domain == MTRInteractionErrorDomain && error.code == MTRInteractionErrorCodeBusy && retryCount < 5) {
                                      workCompletion(MTRAsyncWorkNeedsRetry);
                                      return;
                                  }

                                  workDone(values, error);
                              }];
    }];
    [_asyncWorkQueue enqueueWorkItem:workItem descriptionWithFormat:@"invoke %@ 0x%llx (%@) 0x%llx (%@): %@",
                                              endpointID,
                                              clusterID.unsignedLongLongValue, MTRClusterNameForID(static_cast<MTRClusterIDType>(clusterID.unsignedLongLongValue)),
                                              commandID.unsignedLongLongValue, MTRRequestCommandNameForID(static_cast<MTRClusterIDType>(clusterID.unsignedLongLongValue), static_cast<MTRCommandIDType>(commandID.unsignedLongLongValue)),
                                              commandFields];
}

- (BOOL)_invokeResponse:(MTRDeviceResponseValueDictionary)response matchesRequiredResponse:(NSDictionary<NSNumber *, MTRDeviceDataValueDictionary> *)requiredResponse
{
    if (response[MTRDataKey] == nil) {
        MTR_LOG_ERROR("%@ invokeCommands expects a data response for %@ but got no data", self, response[MTRCommandPathKey]);
        return NO;
    }

    MTRDeviceDataValueDictionary data = response[MTRDataKey];
    if (![MTRStructureValueType isEqual:data[MTRTypeKey]]) {
        MTR_LOG_ERROR("%@ invokeCommands data value %@ for command response for %@ is not a structure", self, data, response[MTRCommandPathKey]);
        return NO;
    }

    NSArray<NSDictionary<NSString *, id> *> * fields = data[MTRValueKey];

    for (NSNumber * fieldID in requiredResponse) {
        // Check that this field is present in the response.
        MTRDeviceDataValueDictionary _Nullable fieldValue = nil;
        for (NSDictionary<NSString *, id> * field in fields) {
            if ([fieldID isEqual:field[MTRContextTagKey]]) {
                fieldValue = field[MTRDataKey];
                break;
            }
        }

        if (fieldValue == nil) {
            MTR_LOG_ERROR("%@ invokeCommands response for %@ does not have a field with ID %@", self, response[MTRCommandPathKey], fieldID);
            return NO;
        }

        auto * expected = requiredResponse[fieldID];
        if (![expected isEqual:fieldValue]) {
            MTR_LOG_ERROR("%@ invokeCommands response for %@ field %@ got %@ but expected %@", self, response[MTRCommandPathKey], fieldID, fieldValue, expected);
            return NO;
        }
    }

    return YES;
}

- (void)invokeCommands:(NSArray<NSArray<MTRCommandWithRequiredResponse *> *> *)commands
                 queue:(dispatch_queue_t)queue
            completion:(MTRDeviceResponseHandler)completion
{
    // We will generally do our work on self.queue, and just dispatch to the provided queue when
    // calling the provided completion.
    auto nextCompletion = ^(BOOL allSucceededSoFar, NSArray<MTRDeviceResponseValueDictionary> * responses) {
        dispatch_async(queue, ^{
            completion(responses, nil);
        });
    };

    // We want to invoke the command groups in order, stopping after failures as needed.  Build up a
    // linked list of groups via chaining the completions, with calls out to the original
    // completion instead of going to the next list item when we want to stop.
    for (NSArray<MTRCommandWithRequiredResponse *> * commandGroup in [commands reverseObjectEnumerator]) {
        // We want to invoke all the commands in the group in order, propagating along the list of
        // current responses.  Build up that linked list of command invokes via chaining the completions.
        mtr_weakify(self);
        for (MTRCommandWithRequiredResponse * command in [commandGroup reverseObjectEnumerator]) {
            auto commandInvokeBlock = ^(BOOL allSucceededSoFar, NSArray<MTRDeviceResponseValueDictionary> * previousResponses) {
                mtr_strongify(self);
                VerifyOrReturn(self, MTR_LOG_DEBUG("invokeCommands commandInvokeBlock called back with nil MTRDevice"));

                [self invokeCommandWithEndpointID:command.path.endpoint
                                        clusterID:command.path.cluster
                                        commandID:command.path.command
                                    commandFields:command.commandFields
                                   expectedValues:nil
                            expectedValueInterval:nil
                                            queue:self.queue
                                       completion:^(NSArray<NSDictionary<NSString *, id> *> * responses, NSError * error) {
                                           mtr_strongify(self);
                                           VerifyOrReturn(self, MTR_LOG_DEBUG("invokeCommands invokeCommandWithEndpointID completion called back with nil MTRDevice"));
                                           if (error != nil) {
                                               nextCompletion(NO, [previousResponses arrayByAddingObject:@ {
                                                   MTRCommandPathKey : command.path,
                                                   MTRErrorKey : error,
                                               }]);
                                               return;
                                           }

                                           if (responses.count != 1) {
                                               // Very much unexpected for invoking a single command.
                                               MTR_LOG_ERROR("%@ invokeCommands unexpectedly got multiple responses for %@", self, command.path);
                                               nextCompletion(NO, [previousResponses arrayByAddingObject:@ {
                                                   MTRCommandPathKey : command.path,
                                                   MTRErrorKey : [MTRError errorForCHIPErrorCode:CHIP_ERROR_INTERNAL],
                                               }]);
                                               return;
                                           }

                                           BOOL nextAllSucceeded = allSucceededSoFar;
                                           MTRDeviceResponseValueDictionary response = responses[0];
                                           if (command.requiredResponse != nil && ![self _invokeResponse:response matchesRequiredResponse:command.requiredResponse]) {
                                               nextAllSucceeded = NO;
                                           }

                                           nextCompletion(nextAllSucceeded, [previousResponses arrayByAddingObject:response]);
                                       }];
            };

            nextCompletion = commandInvokeBlock;
        }

        auto commandGroupInvokeBlock = ^(BOOL allSucceededSoFar, NSArray<MTRDeviceResponseValueDictionary> * previousResponses) {
            mtr_strongify(self);
            VerifyOrReturn(self, MTR_LOG_DEBUG("invokeCommands commandGroupInvokeBlock called back with nil MTRDevice"));

            if (allSucceededSoFar == NO) {
                // Don't start a new command group if something failed in the
                // previous one.  Note that we might be running on self.queue here, so make sure we
                // dispatch to the correct queue.
                MTR_LOG_ERROR("%@ failed a preceding command, not invoking command group %@ or later ones", self, commandGroup);
                dispatch_async(queue, ^{
                    completion(previousResponses, nil);
                });
                return;
            }

            nextCompletion(allSucceededSoFar, previousResponses);
        };

        nextCompletion = commandGroupInvokeBlock;
    }

    // Kick things off with a "everything succeeded so far and we have no responses yet".
    nextCompletion(YES, @[]);
}

- (void)openCommissioningWindowWithSetupPasscode:(NSNumber *)setupPasscode
                                   discriminator:(NSNumber *)discriminator
                                        duration:(NSNumber *)duration
                                           queue:(dispatch_queue_t)queue
                                      completion:(MTRDeviceOpenCommissioningWindowHandler)completion
{
    auto * baseDevice = [self newBaseDevice];
    [baseDevice openCommissioningWindowWithSetupPasscode:setupPasscode
                                           discriminator:discriminator
                                                duration:duration
                                                   queue:queue
                                              completion:completion];
}

- (void)openCommissioningWindowWithDiscriminator:(NSNumber *)discriminator
                                        duration:(NSNumber *)duration
                                           queue:(dispatch_queue_t)queue
                                      completion:(MTRDeviceOpenCommissioningWindowHandler)completion
{
    auto * baseDevice = [self newBaseDevice];
    [baseDevice openCommissioningWindowWithDiscriminator:discriminator duration:duration queue:queue completion:completion];
}

- (void)downloadLogOfType:(MTRDiagnosticLogType)type
                  timeout:(NSTimeInterval)timeout
                    queue:(dispatch_queue_t)queue
               completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
{
    MTR_LOG("%@ downloadLogOfType: %lu, timeout: %f", self, static_cast<unsigned long>(type), timeout);

    auto * baseDevice = [self newBaseDevice];

    mtr_weakify(self);
    [baseDevice downloadLogOfType:type
                          timeout:timeout
                            queue:queue
                       completion:^(NSURL * _Nullable url, NSError * _Nullable error) {
                           mtr_strongify(self);
                           MTR_LOG("%@ downloadLogOfType %lu completed: %@", self, static_cast<unsigned long>(type), error);
                           completion(url, error);
                       }];
}

#pragma mark - Cache management

// assume lock is held
- (void)_checkExpiredExpectedValues
{
    os_unfair_lock_assert_owner(&self->_lock);

    // find expired attributes, and calculate next timer fire date
    NSDate * now = [NSDate date];
    NSDate * nextExpirationDate = nil;
    // Set of NSArray with 2 elements [path, value] - this is used in this method only
    NSMutableSet<NSArray *> * attributeInfoToRemove = [NSMutableSet set];
    for (MTRAttributePath * attributePath in _expectedValueCache) {
        NSArray * expectedValue = _expectedValueCache[attributePath];
        NSDate * attributeExpirationDate = expectedValue[MTRDeviceExpectedValueFieldExpirationTimeIndex];
        if (expectedValue) {
            if ([now compare:attributeExpirationDate] == NSOrderedDescending) {
                // expired - save [path, values] pair to attributeToRemove
                [attributeInfoToRemove addObject:@[ attributePath, expectedValue[MTRDeviceExpectedValueFieldValueIndex] ]];
            } else {
                // get the next expiration date
                if (!nextExpirationDate || [nextExpirationDate compare:attributeExpirationDate] == NSOrderedDescending) {
                    nextExpirationDate = attributeExpirationDate;
                }
            }
        }
    }

    // remove from expected value cache and report attributes as needed
    NSMutableArray * attributesToReport = [NSMutableArray array];
    NSMutableArray * attributePathsToReport = [NSMutableArray array];
    for (NSArray * attributeInfo in attributeInfoToRemove) {
        // compare with known value and mark for report if different
        MTRAttributePath * attributePath = attributeInfo[0];
        NSDictionary * attributeDataValue = attributeInfo[1];
        NSDictionary * cachedAttributeDataValue = [self _cachedAttributeValueForPath:attributePath];
        if (cachedAttributeDataValue
            && ![self _attributeDataValue:attributeDataValue isEqualToDataValue:cachedAttributeDataValue]) {
            [attributesToReport addObject:@{ MTRAttributePathKey : attributePath, MTRDataKey : cachedAttributeDataValue, MTRPreviousDataKey : attributeDataValue }];
            [attributePathsToReport addObject:attributePath];
        }

        _expectedValueCache[attributePath] = nil;
    }

    // log attribute paths
    MTR_LOG("%@ report from expired expected values %@", self, attributePathsToReport);
    [self _reportAttributes:attributesToReport];

// Have a reasonable minimum wait time for expiration timers
#define MTR_DEVICE_EXPIRATION_CHECK_TIMER_MINIMUM_WAIT_TIME (0.1)

    if (nextExpirationDate && _expectedValueCache.count && !self.expirationCheckScheduled) {
        NSTimeInterval waitTime = [nextExpirationDate timeIntervalSinceDate:now];
        if (waitTime < MTR_DEVICE_EXPIRATION_CHECK_TIMER_MINIMUM_WAIT_TIME) {
            waitTime = MTR_DEVICE_EXPIRATION_CHECK_TIMER_MINIMUM_WAIT_TIME;
        }
        mtr_weakify(self);
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (waitTime * NSEC_PER_SEC)), self.queue, ^{
            mtr_strongify(self);
            VerifyOrReturn(self);

            [self _performScheduledExpirationCheck];
        });
    }
}

- (void)_performScheduledExpirationCheck
{
    std::lock_guard lock(_lock);

    self.expirationCheckScheduled = NO;
    [self _checkExpiredExpectedValues];
}

// Get attribute value dictionary for an attribute path from the right cache
- (NSDictionary<NSString *, id> *)_attributeValueDictionaryForAttributePath:(MTRAttributePath *)attributePath
{
    std::lock_guard lock(_lock);
    return [self _lockedAttributeValueDictionaryForAttributePath:attributePath];
}

- (NSDictionary<NSString *, id> *)_lockedAttributeValueDictionaryForAttributePath:(MTRAttributePath *)attributePath
{
    os_unfair_lock_assert_owner(&self->_lock);

    // First check expected value cache
    NSArray * expectedValue = _expectedValueCache[attributePath];
    if (expectedValue) {
        NSDate * now = [NSDate date];
        if ([now compare:expectedValue[MTRDeviceExpectedValueFieldExpirationTimeIndex]] == NSOrderedDescending) {
            // expired - purge and fall through
            _expectedValueCache[attributePath] = nil;
        } else {
            // not yet expired - return result
            return expectedValue[MTRDeviceExpectedValueFieldValueIndex];
        }
    }

    // Then check read cache
    NSDictionary<NSString *, id> * cachedAttributeValue = [self _cachedAttributeValueForPath:attributePath];
    if (cachedAttributeValue) {
        return cachedAttributeValue;
    } else {
        // TODO: when not found in cache, generated default values should be used
        MTR_LOG("%@ _attributeValueDictionaryForAttributePath: could not find cached attribute values for attribute %@", self,
            attributePath);
    }

    return nil;
}

// Utility to return data value dictionary without data version
- (NSDictionary *)_dataValueWithoutDataVersion:(NSDictionary *)attributeValue
{
    // Sanity check for nil - return the same input to fail gracefully
    if (!attributeValue || !attributeValue[MTRTypeKey]) {
        return attributeValue;
    }

    if (attributeValue[MTRValueKey]) {
        return @{ MTRTypeKey : attributeValue[MTRTypeKey], MTRValueKey : attributeValue[MTRValueKey] };
    } else {
        return @{ MTRTypeKey : attributeValue[MTRTypeKey] };
    }
}

// Update cluster data version and also note the change, so at onReportEnd it can be persisted
- (void)_noteDataVersion:(NSNumber *)dataVersion forClusterPath:(MTRClusterPath *)clusterPath
{
    os_unfair_lock_assert_owner(&self->_lock);

    BOOL dataVersionChanged = NO;
    // Update data version used for subscription filtering
    MTRDeviceClusterData * clusterData = [self _clusterDataForPath:clusterPath];
    if (!clusterData) {
        clusterData = [[MTRDeviceClusterData alloc] initWithDataVersion:dataVersion attributes:nil];
        dataVersionChanged = YES;
    } else if (![clusterData.dataVersion isEqualToNumber:dataVersion]) {
        clusterData.dataVersion = dataVersion;
        dataVersionChanged = YES;
    }

    if (dataVersionChanged) {
        if (_clusterDataToPersist == nil) {
            _clusterDataToPersist = [NSMutableDictionary dictionary];
        }
        _clusterDataToPersist[clusterPath] = clusterData;

        MTR_LOG("%@ updated DataVersion for %@ to %@", self, clusterPath, dataVersion);
    }
}

- (BOOL)_attributeAffectsDeviceConfiguration:(MTRAttributePath *)attributePath
{
    // Check for attributes in the descriptor cluster that affect device configuration.
    if (attributePath.cluster.unsignedLongValue == MTRClusterIDTypeDescriptorID) {
        switch (attributePath.attribute.unsignedLongValue) {
        case MTRAttributeIDTypeClusterDescriptorAttributePartsListID:
        case MTRAttributeIDTypeClusterDescriptorAttributeServerListID:
        case MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID: {
            return YES;
        }
        }
    }

    // Check for global attributes that affect device configuration.
    switch (attributePath.attribute.unsignedLongValue) {
    case MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID:
    case MTRAttributeIDTypeGlobalAttributeAttributeListID:
    case MTRAttributeIDTypeGlobalAttributeClusterRevisionID:
    case MTRAttributeIDTypeGlobalAttributeFeatureMapID:
        return YES;
    }
    return NO;
}

- (void)_removeClusters:(NSSet<MTRClusterPath *> *)clusterPathsToRemove
    doRemoveFromDataStore:(BOOL)doRemoveFromDataStore
{
    os_unfair_lock_assert_owner(&self->_lock);

    [_persistedClusters minusSet:clusterPathsToRemove];

    for (MTRClusterPath * path in clusterPathsToRemove) {
        [_persistedClusterData removeObjectForKey:path];
        [_clusterDataToPersist removeObjectForKey:path];
        if (doRemoveFromDataStore) {
            [self._concreteController.controllerDataStore clearStoredClusterDataForNodeID:self.nodeID endpointID:path.endpoint clusterID:path.cluster];
        }
    }
}

- (void)_removeAttributes:(NSSet<NSNumber *> *)attributes fromCluster:(MTRClusterPath *)clusterPath
{
    os_unfair_lock_assert_owner(&self->_lock);

    for (NSNumber * attribute in attributes) {
        [self _removeCachedAttribute:attribute fromCluster:clusterPath];
    }
    // Just clear out the NSCache entry for this cluster, so we'll load it from storage as needed.
    [_persistedClusterData removeObjectForKey:clusterPath];
    [self._concreteController.controllerDataStore removeAttributes:attributes fromCluster:clusterPath forNodeID:self.nodeID];
}

- (void)_pruneEndpointsIn:(MTRDeviceDataValueDictionary)previousPartsListValue
              missingFrom:(MTRDeviceDataValueDictionary)newPartsListValue
{
    // If the parts list changed and one or more endpoints were removed, remove all the
    // clusters for all those endpoints from our data structures.
    // Also remove those endpoints from the data store.
    NSMutableSet<NSNumber *> * toBeRemovedEndpoints = [NSMutableSet setWithArray:[self arrayOfNumbersFromAttributeValue:previousPartsListValue]];
    NSSet<NSNumber *> * endpointsOnDevice = [NSSet setWithArray:[self arrayOfNumbersFromAttributeValue:newPartsListValue]];
    [toBeRemovedEndpoints minusSet:endpointsOnDevice];

    for (NSNumber * endpoint in toBeRemovedEndpoints) {
        NSMutableSet<MTRClusterPath *> * clusterPathsToRemove = [[NSMutableSet alloc] init];
        for (MTRClusterPath * path in _persistedClusters) {
            if ([path.endpoint isEqualToNumber:endpoint]) {
                [clusterPathsToRemove addObject:path];
            }
        }
        [self _removeClusters:clusterPathsToRemove doRemoveFromDataStore:NO];
        [self._concreteController.controllerDataStore clearStoredClusterDataForNodeID:self.nodeID endpointID:endpoint];

        mtr_weakify(self);
        [_deviceController asyncDispatchToMatterQueue:^{
            mtr_strongify(self);
            VerifyOrReturn(self);

            @synchronized(self.matterCPPObjectsHolder) {
                if (self.matterCPPObjectsHolder.subscriptionCallback) {
                    self.matterCPPObjectsHolder.subscriptionCallback->ClearCachedAttributeState(static_cast<EndpointId>(endpoint.unsignedLongLongValue));
                }
            }
        } errorHandler:nil];
    }
}

- (void)_pruneClustersIn:(MTRDeviceDataValueDictionary)previousServerListValue
             missingFrom:(MTRDeviceDataValueDictionary)newServerListValue
             forEndpoint:(NSNumber *)endpointID
{
    // If the server list changed and clusters were removed, remove those clusters from our data structures.
    // Also remove them from the data store.
    NSMutableSet<NSNumber *> * toBeRemovedClusters = [NSMutableSet setWithArray:[self arrayOfNumbersFromAttributeValue:previousServerListValue]];
    NSSet<NSNumber *> * clustersStillOnEndpoint = [NSSet setWithArray:[self arrayOfNumbersFromAttributeValue:newServerListValue]];
    [toBeRemovedClusters minusSet:clustersStillOnEndpoint];

    NSMutableSet<MTRClusterPath *> * clusterPathsToRemove = [[NSMutableSet alloc] init];
    for (MTRClusterPath * path in _persistedClusters) {
        if ([path.endpoint isEqualToNumber:endpointID] && [toBeRemovedClusters containsObject:path.cluster]) {
            [clusterPathsToRemove addObject:path];
        }
    }
    [self _removeClusters:clusterPathsToRemove doRemoveFromDataStore:YES];

    mtr_weakify(self);
    [_deviceController asyncDispatchToMatterQueue:^{
        mtr_strongify(self);
        VerifyOrReturn(self);

        @synchronized(self.matterCPPObjectsHolder) {
            if (self.matterCPPObjectsHolder.subscriptionCallback) {
                for (NSNumber * cluster in toBeRemovedClusters) {
                    ConcreteClusterPath clusterPath(static_cast<EndpointId>(endpointID.unsignedLongLongValue),
                        static_cast<ClusterId>(cluster.unsignedLongLongValue));
                    self.matterCPPObjectsHolder.subscriptionCallback->ClearCachedAttributeState(clusterPath);
                }
            }
        }
    } errorHandler:nil];
}

- (void)_pruneAttributesIn:(MTRDeviceDataValueDictionary)previousAttributeListValue
               missingFrom:(MTRDeviceDataValueDictionary)newAttributeListValue
                forCluster:(MTRClusterPath *)clusterPath
{
    // If the attribute list changed and attributes were removed, remove the attributes from our
    // data structures.
    NSMutableSet<NSNumber *> * toBeRemovedAttributes = [NSMutableSet setWithArray:[self arrayOfNumbersFromAttributeValue:previousAttributeListValue]];
    NSSet<NSNumber *> * attributesStillInCluster = [NSSet setWithArray:[self arrayOfNumbersFromAttributeValue:newAttributeListValue]];

    [toBeRemovedAttributes minusSet:attributesStillInCluster];
    [self _removeAttributes:toBeRemovedAttributes fromCluster:clusterPath];

    mtr_weakify(self);
    [_deviceController asyncDispatchToMatterQueue:^{
        mtr_strongify(self);
        VerifyOrReturn(self);

        @synchronized(self.matterCPPObjectsHolder) {
            if (self.matterCPPObjectsHolder.subscriptionCallback) {
                for (NSNumber * attribute in toBeRemovedAttributes) {
                    ConcreteAttributePath attributePath(static_cast<EndpointId>(clusterPath.endpoint.unsignedLongLongValue),
                        static_cast<ClusterId>(clusterPath.cluster.unsignedLongLongValue),
                        static_cast<AttributeId>(attribute.unsignedLongLongValue));
                    self.matterCPPObjectsHolder.subscriptionCallback->ClearCachedAttributeState(attributePath);
                }
            }
        }
    } errorHandler:nil];
}

- (void)_pruneStoredDataForPath:(MTRAttributePath *)attributePath
                    missingFrom:(MTRDeviceDataValueDictionary)newAttributeDataValue
{
    os_unfair_lock_assert_owner(&self->_lock);

    if (![self _dataStoreExists] && !_clusterDataToPersist.count) {
        MTR_LOG_DEBUG("%@ No data store to prune from", self);
        return;
    }

    // Check if parts list changed or server list changed for the descriptor cluster or the attribute list changed for a cluster.
    // If yes, we might need to prune any deleted endpoints, clusters or attributes from the storage and persisted cluster data.
    if (attributePath.cluster.unsignedLongValue == MTRClusterIDTypeDescriptorID) {
        if (attributePath.attribute.unsignedLongValue == MTRAttributeIDTypeClusterDescriptorAttributePartsListID && [attributePath.endpoint isEqualToNumber:@(kRootEndpointId)]) {
            [self _pruneEndpointsIn:[self _cachedAttributeValueForPath:attributePath] missingFrom:newAttributeDataValue];
            return;
        }

        if (attributePath.attribute.unsignedLongValue == MTRAttributeIDTypeClusterDescriptorAttributeServerListID) {
            [self _pruneClustersIn:[self _cachedAttributeValueForPath:attributePath] missingFrom:newAttributeDataValue forEndpoint:attributePath.endpoint];
            return;
        }
    }

    if (attributePath.attribute.unsignedLongValue == MTRAttributeIDTypeGlobalAttributeAttributeListID) {
        [self _pruneAttributesIn:[self _cachedAttributeValueForPath:attributePath] missingFrom:newAttributeDataValue forCluster:[MTRClusterPath clusterPathWithEndpointID:attributePath.endpoint clusterID:attributePath.cluster]];
    }
}

// assume lock is held
- (NSArray *)_getAttributesToReportWithReportedValues:(NSArray<NSDictionary<NSString *, id> *> *)reportedAttributeValues fromSubscription:(BOOL)isFromSubscription
{
    os_unfair_lock_assert_owner(&self->_lock);

    NSMutableArray * attributesToReport = [NSMutableArray array];
    NSMutableArray * attributePathsToReport = [NSMutableArray array];
    for (NSDictionary<NSString *, id> * attributeResponseValue in reportedAttributeValues) {
        MTRAttributePath * attributePath = attributeResponseValue[MTRAttributePathKey];
        MTRDeviceDataValueDictionary _Nullable attributeDataValue = attributeResponseValue[MTRDataKey];
        NSError * _Nullable attributeError = attributeResponseValue[MTRErrorKey];
        MTRDeviceDataValueDictionary _Nullable previousValue;

        // sanity check either data value or error must exist
        if (!attributeDataValue && !attributeError) {
            MTR_LOG("%@ report %@ no data value or error: %@", self, attributePath, attributeResponseValue);
            continue;
        }

        // check if value is different than cache, and report if needed
        BOOL shouldReportAttribute = NO;

        // if this is an error, report and purge cache
        if (attributeError) {
            shouldReportAttribute = YES;
            previousValue = [self _cachedAttributeValueForPath:attributePath];
            MTR_LOG_ERROR("%@ report %@ error %@ purge expected value %@ read cache %@", self, attributePath, attributeError,
                _expectedValueCache[attributePath], previousValue);
            _expectedValueCache[attributePath] = nil;
            // TODO: Is this clearing business really what we want?
            [self _setCachedAttributeValue:nil forPath:attributePath fromSubscription:isFromSubscription];
        } else {
            // First separate data version and restore data value to a form without data version
            NSNumber * dataVersion = attributeDataValue[MTRDataVersionKey];
            MTRClusterPath * clusterPath = [MTRClusterPath clusterPathWithEndpointID:attributePath.endpoint clusterID:attributePath.cluster];
            if (dataVersion) {
                [self _noteDataVersion:dataVersion forClusterPath:clusterPath];

                // Remove data version from what we cache in memory
                attributeDataValue = [self _dataValueWithoutDataVersion:attributeDataValue];
            }

            previousValue = [self _cachedAttributeValueForPath:attributePath];
#ifdef DEBUG
            __block BOOL readCacheValueChanged = ![self _attributeDataValue:attributeDataValue isEqualToDataValue:previousValue];
#else
            BOOL readCacheValueChanged = ![self _attributeDataValue:attributeDataValue isEqualToDataValue:previousValue];
#endif
            // Now that we have grabbed previousValue, update our cache with the attribute value.
            if (readCacheValueChanged) {
                [self _pruneStoredDataForPath:attributePath missingFrom:attributeDataValue];

                if (!_deviceConfigurationChanged) {
                    _deviceConfigurationChanged = [self _attributeAffectsDeviceConfiguration:attributePath];
                    if (_deviceConfigurationChanged) {
                        MTR_LOG("%@ device configuration changed due to changes in attribute %@", self, attributePath);
                    }
                }

                [self _setCachedAttributeValue:attributeDataValue forPath:attributePath fromSubscription:isFromSubscription];

                [self _attributeValue:attributeDataValue reportedForPath:attributePath];
            }

#ifdef DEBUG
            // Unit test only code.
            if (!readCacheValueChanged) {
                [self _callFirstDelegateSynchronouslyWithBlock:^(id delegate) {
                    if ([delegate respondsToSelector:@selector(unitTestForceAttributeReportsIfMatchingCache:)]) {
                        readCacheValueChanged = [delegate unitTestForceAttributeReportsIfMatchingCache:self];
                    }
                }];
            }
#endif // DEBUG

            NSArray * expectedValue = _expectedValueCache[attributePath];

            // Report the attribute if a read would get a changed value.  This happens
            // when our cached value changes and no expected value exists.
            if (readCacheValueChanged && !expectedValue) {
                shouldReportAttribute = YES;
            }

            if (!shouldReportAttribute) {
                // If an expected value exists, the attribute will not be reported at this time.
                // When the expected value interval expires, the correct value will be reported,
                // if needed.
                if (expectedValue) {
                    MTR_LOG("%@ report %@ value %@ filtered - expected value still present", self, attributePath, attributeDataValue);
                } else {
                    MTR_LOG("%@ report %@ value %@ filtered - same as read cache", self, attributePath, attributeDataValue);
                }
            }

            // If General Diagnostics UpTime attribute, update the estimated start time as needed.
            if ((attributePath.cluster.unsignedLongValue == MTRClusterGeneralDiagnosticsID)
                && (attributePath.attribute.unsignedLongValue == MTRClusterGeneralDiagnosticsAttributeUpTimeID)) {
                // verify that the uptime is indeed the data type we want
                if ([attributeDataValue[MTRTypeKey] isEqual:MTRUnsignedIntegerValueType]) {
                    NSNumber * upTimeNumber = attributeDataValue[MTRValueKey];
                    NSTimeInterval upTime = upTimeNumber.unsignedLongLongValue; // UpTime unit is defined as seconds in the spec
                    NSDate * potentialSystemStartTime = [NSDate dateWithTimeIntervalSinceNow:-upTime];
                    NSDate * oldSystemStartTime = _estimatedStartTime;
                    if (!_estimatedStartTime || ([potentialSystemStartTime compare:_estimatedStartTime] == NSOrderedAscending)) {
                        MTR_LOG("%@ General Diagnostics UpTime %.3lf: estimated start time %@ => %@", self, upTime,
                            oldSystemStartTime, potentialSystemStartTime);
                        _estimatedStartTime = potentialSystemStartTime;
                    }

                    // Save estimate in the subscription resumption case, for when StartUp event uses it
                    _estimatedStartTimeFromGeneralDiagnosticsUpTime = potentialSystemStartTime;
                }
            }
        }

        if (shouldReportAttribute) {
            if (previousValue) {
                NSMutableDictionary * mutableAttributeResponseValue = attributeResponseValue.mutableCopy;
                mutableAttributeResponseValue[MTRPreviousDataKey] = previousValue;
                [attributesToReport addObject:mutableAttributeResponseValue];
            } else {
                [attributesToReport addObject:attributeResponseValue];
            }
            [attributePathsToReport addObject:attributePath];
        }
    }

    if (attributePathsToReport.count > 0) {
        MTR_LOG("%@ report from reported values %@", self, attributePathsToReport);
    }

    return attributesToReport;
}

// TODO: Figure out whether we can get rid of this in favor of readAttributePaths.  This differs from
// readAttributePaths in one respect: that function will do read-through for
// C-quality attributes, but this one does not.
- (NSArray<NSDictionary<NSString *, id> *> *)getAllAttributesReport
{
    std::lock_guard lock(_lock);

    NSMutableArray * attributeReport = [NSMutableArray array];
    for (MTRClusterPath * clusterPath in [self _knownClusters]) {
        MTRDeviceClusterData * clusterData = [self _clusterDataForPath:clusterPath];

        for (NSNumber * attributeID in clusterData.attributes) {
            auto * attributePath = [MTRAttributePath attributePathWithEndpointID:clusterPath.endpoint
                                                                       clusterID:clusterPath.cluster
                                                                     attributeID:attributeID];

            // Construct response-value dictionary with the data-value dictionary returned by
            // _lockedAttributeValueDictionaryForAttributePath, to takes into consideration expected values as well.
            [attributeReport addObject:@{
                MTRAttributePathKey : attributePath,
                MTRDataKey : [self _lockedAttributeValueDictionaryForAttributePath:attributePath]
            }];
        }
    }

    return attributeReport;
}

#ifdef DEBUG
- (NSUInteger)unitTestAttributeCount
{
    std::lock_guard lock(_lock);
    NSUInteger count = 0;
    for (MTRClusterPath * path in [self _knownClusters]) {
        count += [self _clusterDataForPath:path].attributes.count;
    }
    return count;
}
#endif

- (void)setPersistedClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)clusterData
{
    [self _doSetPersistedClusterData:clusterData];

    // Log after we do the state update, so the log includes the data we just set.
    MTR_LOG("%@ setPersistedClusterData count: %lu", self, static_cast<unsigned long>(clusterData.count));
}

- (void)_doSetPersistedClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)clusterData
{
    if (!clusterData.count) {
        return;
    }

    std::lock_guard lock(_lock);

    NSAssert([self _dataStoreExists], @"Why is controller setting persisted data when we shouldn't have it?");

    for (MTRClusterPath * clusterPath in clusterData) {
        // The caller has mutable references to MTRDeviceClusterData and
        // MTRClusterPath, but that should be OK, since we control all the
        // callers.  If that stops being OK, we'll need to copy the key and
        // value here.
        [_persistedClusters addObject:clusterPath];
        [_persistedClusterData setObject:clusterData[clusterPath] forKey:clusterPath];
    }

    [self _updateAttributeDependentDescriptionData];

    NSNumber * networkFeatures = nil;
    {
        std::lock_guard descriptionLock(_descriptionLock);
        networkFeatures = _allNetworkFeatures;
    }

    if ((networkFeatures.unsignedLongLongValue & MTRNetworkCommissioningFeatureWiFiNetworkInterface) == 0 && (networkFeatures.unsignedLongLongValue & MTRNetworkCommissioningFeatureThreadNetworkInterface) == 0) {
        // We had persisted data, but apparently this device does not have any
        // known network technologies?  Log some more information about what's
        // going on.
        auto * rootNetworkCommissioningPath = [MTRClusterPath clusterPathWithEndpointID:@(kRootEndpointId) clusterID:@(MTRClusterIDTypeNetworkCommissioningID)];
        auto * networkCommisioningData = clusterData[rootNetworkCommissioningPath];
        MTR_LOG("%@ after setting persisted data, network features: %@, root network commissioning featureMap: %@", self, networkFeatures,
            networkCommisioningData.attributes[@(MTRClusterGlobalAttributeFeatureMapID)]);
    }

    // We have some stored data.  Since we don't store data until the end of the
    // initial priming report, our device cache must be primed.
    _deviceCachePrimed = YES;
}

- (void)_setLastInitialSubscribeLatency:(id)latency
{
    os_unfair_lock_assert_owner(&self->_lock);

    if (![latency isKindOfClass:NSNumber.class]) {
        // Unexpected value of some sort; just ignore it.
        return;
    }

    _estimatedSubscriptionLatency = latency;
}

- (void)setPersistedDeviceData:(NSDictionary<NSString *, id> *)data
{
    MTR_LOG_DEBUG("%@ setPersistedDeviceData: %@", self, data);

    std::lock_guard lock(_lock);

    // For now the only data we care about is our initial subscribe latency.
    id initialSubscribeLatency = data[sLastInitialSubscribeLatencyKey];
    if (initialSubscribeLatency != nil) {
        [self _setLastInitialSubscribeLatency:initialSubscribeLatency];
    }
}

- (void)_storePersistedDeviceData
{
    os_unfair_lock_assert_owner(&self->_lock);

    auto datastore = self._concreteController.controllerDataStore;
    if (datastore == nil) {
        // No way to store.
        return;
    }

    // For now the only data we have is our initial subscribe latency.
    NSMutableDictionary<NSString *, id> * data = [NSMutableDictionary dictionary];
    if (_estimatedSubscriptionLatency != nil) {
        data[sLastInitialSubscribeLatencyKey] = _estimatedSubscriptionLatency;
    }

    [datastore storeDeviceData:[data copy] forNodeID:self.nodeID];
}

#ifdef DEBUG
- (MTRDeviceClusterData *)unitTestGetClusterDataForPath:(MTRClusterPath *)path
{
    std::lock_guard lock(_lock);

    return [[self _clusterDataForPath:path] copy];
}

- (NSSet<MTRClusterPath *> *)unitTestGetPersistedClusters
{
    std::lock_guard lock(_lock);

    return [_persistedClusters copy];
}

- (BOOL)unitTestClusterHasBeenPersisted:(MTRClusterPath *)path
{
    std::lock_guard lock(_lock);

    return [_persistedClusters containsObject:path];
}
#endif

- (BOOL)deviceCachePrimed
{
    std::lock_guard lock(_lock);
    return _deviceCachePrimed;
}

// If value is non-nil, associate with expectedValueID
// If value is nil, remove only if expectedValueID matches
// previousValue is an out parameter
- (void)_setExpectedValue:(NSDictionary<NSString *, id> *)expectedAttributeValue
             attributePath:(MTRAttributePath *)attributePath
            expirationTime:(NSDate *)expirationTime
         shouldReportValue:(BOOL *)shouldReportValue
    attributeValueToReport:(NSDictionary<NSString *, id> **)attributeValueToReport
           expectedValueID:(uint64_t)expectedValueID
             previousValue:(NSDictionary **)previousValue
{
    os_unfair_lock_assert_owner(&self->_lock);

    *shouldReportValue = NO;

    NSArray * previousExpectedValue = _expectedValueCache[attributePath];
    if (previousExpectedValue) {
        if (expectedAttributeValue
            && ![self _attributeDataValue:expectedAttributeValue
                       isEqualToDataValue:previousExpectedValue[MTRDeviceExpectedValueFieldValueIndex]]) {
            // Case where new expected value overrides previous expected value - report new expected value
            *shouldReportValue = YES;
            *attributeValueToReport = expectedAttributeValue;
            *previousValue = previousExpectedValue[MTRDeviceExpectedValueFieldValueIndex];
        } else if (!expectedAttributeValue) {
            // Remove previous expected value only if it's from the same setExpectedValues operation
            NSNumber * previousExpectedValueID = previousExpectedValue[MTRDeviceExpectedValueFieldIDIndex];
            if (previousExpectedValueID.unsignedLongLongValue == expectedValueID) {
                MTRDeviceDataValueDictionary cachedValue = [self _cachedAttributeValueForPath:attributePath];
                if (![self _attributeDataValue:previousExpectedValue[MTRDeviceExpectedValueFieldValueIndex]
                            isEqualToDataValue:cachedValue]) {
                    // Case of removing expected value that is different than read cache - report read cache value
                    *shouldReportValue = YES;
                    *attributeValueToReport = cachedValue;
                    *previousValue = previousExpectedValue[MTRDeviceExpectedValueFieldValueIndex];
                    _expectedValueCache[attributePath] = nil;
                }
            }
        }
    } else {
        MTRDeviceDataValueDictionary cachedValue = [self _cachedAttributeValueForPath:attributePath];
        if (expectedAttributeValue
            && ![self _attributeDataValue:expectedAttributeValue isEqualToDataValue:cachedValue]) {
            // Case where new expected value is different than read cache - report new expected value
            *shouldReportValue = YES;
            *attributeValueToReport = expectedAttributeValue;
            *previousValue = cachedValue;
        } else {
            *previousValue = nil;
        }

        // No need to report if new and previous expected value are both nil
    }

    if (expectedAttributeValue) {
        _expectedValueCache[attributePath] = @[ expirationTime, expectedAttributeValue, @(expectedValueID) ];
    }
}

// assume lock is held
- (NSArray *)_getAttributesToReportWithNewExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedAttributeValues
                                          expirationTime:(NSDate *)expirationTime
                                         expectedValueID:(uint64_t *)expectedValueID
{
    os_unfair_lock_assert_owner(&self->_lock);
    uint64_t expectedValueIDToReturn = _expectedValueNextID++;

    NSMutableArray * attributesToReport = [NSMutableArray array];
    NSMutableArray * attributePathsToReport = [NSMutableArray array];
    for (NSDictionary<NSString *, id> * attributeResponseValue in expectedAttributeValues) {
        MTRAttributePath * attributePath = attributeResponseValue[MTRAttributePathKey];
        NSDictionary * attributeDataValue = attributeResponseValue[MTRDataKey];

        BOOL shouldReportValue = NO;
        NSDictionary<NSString *, id> * attributeValueToReport;
        NSDictionary<NSString *, id> * previousValue;
        [self _setExpectedValue:attributeDataValue
                     attributePath:attributePath
                    expirationTime:expirationTime
                 shouldReportValue:&shouldReportValue
            attributeValueToReport:&attributeValueToReport
                   expectedValueID:expectedValueIDToReturn
                     previousValue:&previousValue];

        if (shouldReportValue) {
            if (previousValue) {
                [attributesToReport addObject:@{ MTRAttributePathKey : attributePath, MTRDataKey : attributeValueToReport, MTRPreviousDataKey : previousValue }];
            } else {
                [attributesToReport addObject:@{ MTRAttributePathKey : attributePath, MTRDataKey : attributeValueToReport }];
            }
            [attributePathsToReport addObject:attributePath];
        }
    }
    if (expectedValueID) {
        *expectedValueID = expectedValueIDToReturn;
    }

    MTR_LOG("%@ report from new expected values %@", self, attributePathsToReport);

    return attributesToReport;
}

// expectedValueID is an out-argument that returns an identifier to be used when removing expected values
- (void)setExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)values
    expectedValueInterval:(NSNumber *)expectedValueInterval
          expectedValueID:(uint64_t *)expectedValueID
{
    // since NSTimeInterval is in seconds, convert ms into seconds in double
    NSDate * expirationTime = [NSDate dateWithTimeIntervalSinceNow:expectedValueInterval.doubleValue / 1000];

    MTR_LOG(
        "%@ Setting expected values %@ with expiration time %f seconds from now", self, values, [expirationTime timeIntervalSinceNow]);

    std::lock_guard lock(_lock);

    // _getAttributesToReportWithNewExpectedValues will log attribute paths reported
    NSArray * attributesToReport = [self _getAttributesToReportWithNewExpectedValues:values
                                                                      expirationTime:expirationTime
                                                                     expectedValueID:expectedValueID];
    [self _reportAttributes:attributesToReport];

    [self _checkExpiredExpectedValues];
}

- (void)removeExpectedValuesForAttributePaths:(NSArray<MTRAttributePath *> *)attributePaths
                              expectedValueID:(uint64_t)expectedValueID
{
    std::lock_guard lock(_lock);

    for (MTRAttributePath * attributePath in attributePaths) {
        [self _removeExpectedValueForAttributePath:attributePath expectedValueID:expectedValueID];
    }
}

- (void)removeExpectedValueForAttributePath:(MTRAttributePath *)attributePath expectedValueID:(uint64_t)expectedValueID
{
    std::lock_guard lock(_lock);
    [self _removeExpectedValueForAttributePath:attributePath expectedValueID:expectedValueID];
}

- (void)_removeExpectedValueForAttributePath:(MTRAttributePath *)attributePath expectedValueID:(uint64_t)expectedValueID
{
    os_unfair_lock_assert_owner(&self->_lock);

    BOOL shouldReportValue;
    NSDictionary<NSString *, id> * attributeValueToReport;
    NSDictionary<NSString *, id> * previousValue;
    [self _setExpectedValue:nil
                 attributePath:attributePath
                expirationTime:nil
             shouldReportValue:&shouldReportValue
        attributeValueToReport:&attributeValueToReport
               expectedValueID:expectedValueID
                 previousValue:&previousValue];

    MTR_LOG("%@ remove expected value for path %@ should report %@", self, attributePath, shouldReportValue ? @"YES" : @"NO");

    if (shouldReportValue) {
        NSMutableDictionary * attribute = [NSMutableDictionary dictionaryWithObject:attributePath forKey:MTRAttributePathKey];
        if (attributeValueToReport) {
            attribute[MTRDataKey] = attributeValueToReport;
        }
        if (previousValue) {
            attribute[MTRPreviousDataKey] = previousValue;
        }
        [self _reportAttributes:@[ attribute ]];
    }
}

- (MTRBaseDevice *)newBaseDevice
{
    return [MTRBaseDevice deviceWithNodeID:self.nodeID controller:self.deviceController];
}

#pragma mark Log Help

- (nullable NSNumber *)_informationalNumberAtAttributePath:(MTRAttributePath *)attributePath
{
    auto * cachedData = [self _cachedAttributeValueForPath:attributePath];

    if (cachedData == nil) {
        return nil;
    }

    auto * attrReport = [[MTRAttributeReport alloc] initWithResponseValue:@{
        MTRAttributePathKey : attributePath,
        MTRDataKey : cachedData,
    }
                                                                    error:nil];

    return attrReport.value;
}

- (nullable NSNumber *)_informationalVendorID
{
    auto * vendorIDPath = [MTRAttributePath attributePathWithEndpointID:@(kRootEndpointId)
                                                              clusterID:@(MTRClusterIDTypeBasicInformationID)
                                                            attributeID:@(MTRClusterBasicAttributeVendorIDID)];

    return [self _informationalNumberAtAttributePath:vendorIDPath];
}

- (nullable NSNumber *)_informationalProductID
{
    auto * productIDPath = [MTRAttributePath attributePathWithEndpointID:@(kRootEndpointId)
                                                               clusterID:@(MTRClusterIDTypeBasicInformationID)
                                                             attributeID:@(MTRClusterBasicAttributeProductIDID)];

    return [self _informationalNumberAtAttributePath:productIDPath];
}

- (void)_addInformationalAttributesToCurrentMetricScope
{
    os_unfair_lock_assert_owner(&self->_lock);

    using namespace chip::Tracing::DarwinFramework;
    MATTER_LOG_METRIC(kMetricDeviceVendorID, [self _informationalVendorID].unsignedShortValue);
    MATTER_LOG_METRIC(kMetricDeviceProductID, [self _informationalProductID].unsignedShortValue);
    BOOL usesThread = [self _deviceUsesThread];
    MATTER_LOG_METRIC(kMetricDeviceUsesThread, usesThread);
}

#pragma mark - Description handling

- (BOOL)_attributePathAffectsDescriptionData:(MTRAttributePath *)path
{
    // Technically this does not need to be called while locked, but in
    // practice it is, and we want to make sure it's clear that this function
    // should never start taking our data lock.
    os_unfair_lock_assert_owner(&_lock);

    switch (path.cluster.unsignedLongLongValue) {
    case MTRClusterIDTypeBasicInformationID: {
        switch (path.attribute.unsignedLongLongValue) {
        case MTRAttributeIDTypeClusterBasicInformationAttributeVendorIDID:
        case MTRAttributeIDTypeClusterBasicInformationAttributeProductIDID:
            return YES;
        default:
            return NO;
        }
    }
    case MTRClusterIDTypeNetworkCommissioningID: {
        return path.attribute.unsignedLongLongValue == MTRAttributeIDTypeGlobalAttributeFeatureMapID;
    }
    default:
        return NO;
    }
}

- (void)_updateAttributeDependentDescriptionData
{
    os_unfair_lock_assert_owner(&_lock);

    NSNumber * _Nullable vid = [self _informationalVendorID];
    NSNumber * _Nullable pid = [self _informationalProductID];
    NSNumber * _Nullable networkFeatures = [self _networkFeatures];

    std::lock_guard lock(_descriptionLock);
    _vid = vid;
    _pid = pid;
    _allNetworkFeatures = networkFeatures;
}

- (NSArray<NSNumber *> *)_endpointList
{
    os_unfair_lock_assert_owner(&_lock);

    auto * partsList = [self _cachedListOfNumbersValueForEndpointID:@(kRootEndpointId)
                                                          clusterID:@(MTRClusterIDTypeDescriptorID)
                                                        attributeID:@(MTRAttributeIDTypeClusterDescriptorAttributePartsListID)];
    NSMutableArray<NSNumber *> * endpointsOnDevice = [partsList mutableCopy];
    // Add Root Node endpoint.
    [endpointsOnDevice addObject:@(kRootEndpointId)];
    return endpointsOnDevice;
}

/**
 * Returns the cached value of the relevant attribute as a list of numbers.
 * Returns an empty list if the value does not exist or can't be converted to a
 * list of numbers.
 */
- (NSArray<NSNumber *> *)_cachedListOfNumbersValueForEndpointID:(NSNumber *)endpointID
                                                      clusterID:(NSNumber *)clusterID
                                                    attributeID:(NSNumber *)attributeID
{
    os_unfair_lock_assert_owner(&_lock);

    auto * path = [MTRAttributePath attributePathWithEndpointID:endpointID
                                                      clusterID:clusterID
                                                    attributeID:attributeID];
    auto * value = [self _cachedAttributeValueForPath:path];
    NSArray<NSNumber *> * arrayValue = [self arrayOfNumbersFromAttributeValue:value];
    if (arrayValue) {
        return arrayValue;
    }
    return [NSArray array];
}

- (NSArray<NSNumber *> *)_serverListForEndpointID:(NSNumber *)endpointID
{
    os_unfair_lock_assert_owner(&_lock);

    return [self _cachedListOfNumbersValueForEndpointID:endpointID
                                              clusterID:@(MTRClusterIDTypeDescriptorID)
                                            attributeID:@(MTRAttributeIDTypeClusterDescriptorAttributeServerListID)];
}

- (NSArray<NSNumber *> *)_attributeListForEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID
{
    os_unfair_lock_assert_owner(&_lock);

    return [self _cachedListOfNumbersValueForEndpointID:endpointID
                                              clusterID:clusterID
                                            attributeID:@(MTRAttributeIDTypeGlobalAttributeAttributeListID)];
}

- (NSNumber * _Nullable)_networkFeatures
{
    NSNumber * _Nullable result = nil;
    auto * endpoints = [self _endpointList];
    for (NSNumber * endpoint in endpoints) {
        auto * featureMapPath = [MTRAttributePath attributePathWithEndpointID:endpoint
                                                                    clusterID:@(MTRClusterIDTypeNetworkCommissioningID)
                                                                  attributeID:@(MTRAttributeIDTypeGlobalAttributeFeatureMapID)];
        auto * featureMap = [self _informationalNumberAtAttributePath:featureMapPath];
        if (featureMap == nil) {
            // No network commissioning cluster on this endpoint, or no known
            // FeatureMap attribute value for it yet.
            continue;
        }

        if (result == nil) {
            result = featureMap;
        } else {
            result = @(featureMap.unsignedLongLongValue | result.unsignedLongLongValue);
        }
    }

    return result;
}

- (void)controllerSuspended
{
    [super controllerSuspended];

    std::lock_guard lock(self->_lock);
    self.suspended = YES;
    [self _resetSubscriptionWithReasonString:@"Controller suspended"];

    // Ensure that any pre-existing resubscribe attempts we control don't try to
    // do anything.
    _reattemptingSubscription = NO;
}

- (void)controllerResumed
{
    [super controllerResumed];

    std::lock_guard lock(self->_lock);
    self.suspended = NO;

    if (![self _delegateExists]) {
        MTR_LOG("%@ ignoring controller resume: no delegates", self);
        return;
    }

    // Use _ensureSubscriptionForExistingDelegates so that the subscriptions
    // will go through the pool as needed, not necessarily happen immediately.
    [self _ensureSubscriptionForExistingDelegates:@"Controller resumed"];
}

- (nullable MTRDeviceController_Concrete *)_concreteController
{
    // We know our _deviceController is actually an MTRDeviceController_Concrete, since that's what
    // gets passed to initWithNodeID.
    return static_cast<MTRDeviceController_Concrete *>(_deviceController);
}

@end

/* BEGIN DRAGONS: Note methods here cannot be renamed, and are used by private callers, do not rename, remove or modify behavior here */

@implementation MTRDevice_Concrete (MatterPrivateForInternalDragonsDoNotFeed)

- (BOOL)_deviceHasActiveSubscription
{
    std::lock_guard lock(_lock);

    // TODO: This should always return YES for thread devices
    return HaveSubscriptionEstablishedRightNow(_internalDeviceState);
}

// TODO: make this configurable - for now use 1.5 second
#define MTRDEVICE_ACTIVE_COMMUNICATION_THRESHOLD_SECONDS (1.5)

- (void)_deviceMayBeReachable
{
    // Ignore this call if actively receiving communication from this device
    {
        std::lock_guard lock(self->_lock);
        if (self.lastSubscriptionActiveTime) {
            NSTimeInterval intervalSinceDeviceLastActive = -[self.lastSubscriptionActiveTime timeIntervalSinceNow];
            if (intervalSinceDeviceLastActive < MTRDEVICE_ACTIVE_COMMUNICATION_THRESHOLD_SECONDS) {
                MTR_LOG("%@ _deviceMayBeReachable called and ignored, because last received communication from device %.6lf seconds ago", self, intervalSinceDeviceLastActive);
                return;
            }
        }
    }

    MTR_LOG("%@ _deviceMayBeReachable called, resetting subscription", self);
    // TODO: This should only be allowed for thread devices
    mtr_weakify(self);
    [self._concreteController asyncGetCommissionerOnMatterQueue:^(Controller::DeviceCommissioner * commissioner) {
        mtr_strongify(self);
        VerifyOrReturn(self);

        // Reset all of our subscription/session state and re-establish it all
        // from the start.  Reset our subscription first, before tearing
        // down the session, so we don't have to worry about the
        // notifications from the latter coming through async and
        // complicating the situation.  Unfortunately, we do not want to
        // hold the lock when destroying the session, just in case it still
        // ends up calling into us somehow, so we have to break the work up
        // into two separate locked sections...
        {
            std::lock_guard lock(self->_lock);
            [self _clearSubscriptionPoolWork];
            [self _resetSubscription];
        }

        auto peerScopeId = commissioner->GetPeerScopedId(self->_nodeID.unsignedLongLongValue);
        auto caseSessionMgr = commissioner->CASESessionMgr();
        VerifyOrDie(caseSessionMgr != nullptr);
        caseSessionMgr->ReleaseSession(peerScopeId);

// TODO: make this configurable - for now use 1.5 second
#define MTRDEVICE_ACTIVE_SESSION_THRESHOLD_MILLISECONDS (15000)
        auto sessionMgr = commissioner->SessionMgr();
        VerifyOrDie(sessionMgr != nullptr);
        sessionMgr->ForEachMatchingSession(peerScopeId, [](auto * session) {
            auto secureSession = session->AsSecureSession();
            if (!secureSession) {
                return;
            }

            auto threshold = System::Clock::Timeout(MTRDEVICE_ACTIVE_SESSION_THRESHOLD_MILLISECONDS);
            if ((System::SystemClock().GetMonotonicTimestamp() - session->GetLastPeerActivityTime()) < threshold) {
                return;
            }

            session->MarkAsDefunct();
        });

        std::lock_guard lock(self->_lock);
        // Use _ensureSubscriptionForExistingDelegates so that the subscriptions
        // will go through the pool as needed, not necessarily happen immediately.
        [self _ensureSubscriptionForExistingDelegates:sDeviceMayBeReachableReason];
    } errorHandler:nil];
}

/* END DRAGONS */

@end

@implementation MTRDevice_Concrete (Deprecated)

+ (MTRDevice *)deviceWithNodeID:(uint64_t)nodeID deviceController:(MTRDeviceController *)deviceController
{
    return [self deviceWithNodeID:@(nodeID) controller:deviceController];
}

@end

#pragma mark - SubscriptionCallback
namespace {
void SubscriptionCallback::OnSubscriptionEstablished(SubscriptionId aSubscriptionId)
{
    // The next time we need to do a resubscribe, we should start a new backoff
    // sequence.
    ResetResubscriptionBackoff();
    MTRBaseSubscriptionCallback::OnSubscriptionEstablished(aSubscriptionId);
}

void SubscriptionCallback::OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus)
{
    if (mEventReports == nil) {
        // Never got a OnReportBegin?  Not much to do other than tear things down.
        ReportError(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    MTREventPath * eventPath = [[MTREventPath alloc] initWithPath:aEventHeader.mPath];
    if (apStatus != nullptr) {
        [mEventReports addObject:@ { MTREventPathKey : eventPath, MTRErrorKey : [MTRError errorForIMStatus:*apStatus] }];
    } else if (apData == nullptr) {
        [mEventReports addObject:@ {
            MTREventPathKey : eventPath,
            MTRErrorKey : [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT]
        }];
    } else {
        id value = MTRDecodeDataValueDictionaryFromCHIPTLV(apData);
        if (value == nil) {
            MTR_LOG_ERROR("Failed to decode event data for path %@", eventPath);
            [mEventReports addObject:@ {
                MTREventPathKey : eventPath,
                MTRErrorKey : [MTRError errorForCHIPErrorCode:CHIP_ERROR_DECODE_FAILED],
            }];
        } else {
            [mEventReports addObject:[MTRBaseDevice eventReportForHeader:aEventHeader andData:value]];
        }
    }

    QueueInterimReport();
}

void SubscriptionCallback::OnAttributeData(
    const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus)
{
    if (aPath.IsListItemOperation()) {
        ReportError(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    if (mAttributeReports == nil) {
        // Never got a OnReportBegin?  Not much to do other than tear things down.
        ReportError(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    MTRAttributePath * attributePath = [[MTRAttributePath alloc] initWithPath:aPath];
    if (aStatus.mStatus != Status::Success) {
        [mAttributeReports addObject:@ { MTRAttributePathKey : attributePath, MTRErrorKey : [MTRError errorForIMStatus:aStatus] }];
    } else if (apData == nullptr) {
        [mAttributeReports addObject:@ {
            MTRAttributePathKey : attributePath,
            MTRErrorKey : [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT]
        }];
    } else {
        NSNumber * dataVersionNumber = aPath.mDataVersion.HasValue() ? @(aPath.mDataVersion.Value()) : nil;
        NSDictionary * value = MTRDecodeDataValueDictionaryFromCHIPTLV(apData, dataVersionNumber);
        if (value == nil) {
            MTR_LOG_ERROR("Failed to decode attribute data for path %@", attributePath);
            [mAttributeReports addObject:@ {
                MTRAttributePathKey : attributePath,
                MTRErrorKey : [MTRError errorForCHIPErrorCode:CHIP_ERROR_DECODE_FAILED],
            }];
        } else {
            [mAttributeReports addObject:@ { MTRAttributePathKey : attributePath, MTRDataKey : value }];
        }
    }

    QueueInterimReport();
}

uint32_t SubscriptionCallback::ComputeTimeTillNextSubscription()
{
    uint32_t maxWaitTimeInMsec = 0;
    uint32_t waitTimeInMsec = 0;
    uint32_t minWaitTimeInMsec = 0;

    if (mResubscriptionNumRetries <= CHIP_RESUBSCRIBE_MAX_FIBONACCI_STEP_INDEX) {
        maxWaitTimeInMsec = GetFibonacciForIndex(mResubscriptionNumRetries) * CHIP_RESUBSCRIBE_WAIT_TIME_MULTIPLIER_MS;
    } else {
        maxWaitTimeInMsec = CHIP_RESUBSCRIBE_MAX_RETRY_WAIT_INTERVAL_MS;
    }

    if (maxWaitTimeInMsec != 0) {
        minWaitTimeInMsec = (CHIP_RESUBSCRIBE_MIN_WAIT_TIME_INTERVAL_PERCENT_PER_STEP * maxWaitTimeInMsec) / 100;
        waitTimeInMsec = minWaitTimeInMsec + (Crypto::GetRandU32() % (maxWaitTimeInMsec - minWaitTimeInMsec));
    }

    return waitTimeInMsec;
}

CHIP_ERROR SubscriptionCallback::OnResubscriptionNeeded(ReadClient * apReadClient, CHIP_ERROR aTerminationCause)
{
    // No need to check ReadClient internal state is Idle because ReadClient only calls OnResubscriptionNeeded after calling ClearActiveSubscriptionState(), which sets the state to Idle.

    // This part is copied from ReadClient's DefaultResubscribePolicy:
    auto timeTillNextResubscriptionMs = ComputeTimeTillNextSubscription();
    ChipLogProgress(DataManagement,
        "Will try to resubscribe to %02x:" ChipLogFormatX64 " at retry index %" PRIu32 " after %" PRIu32
        "ms due to error %" CHIP_ERROR_FORMAT,
        apReadClient->GetFabricIndex(), ChipLogValueX64(apReadClient->GetPeerNodeId()), mResubscriptionNumRetries, timeTillNextResubscriptionMs,
        aTerminationCause.Format());

    // Schedule a maximum time resubscription, to be triggered with TriggerResubscribeIfScheduled after a separate timer.
    // This way the aReestablishCASE value is saved, and the sanity checks in ScheduleResubscription are observed and returned.
    ReturnErrorOnFailure(apReadClient->ScheduleResubscription(UINT32_MAX, NullOptional, aTerminationCause == CHIP_ERROR_TIMEOUT));

    // Not as good a place to increment as when resubscription timer fires, but as is, this should be as good, because OnResubscriptionNeeded is only called from ReadClient's Close() while Idle, and nothing should cause this to happen
    mResubscriptionNumRetries++;

    auto error = [MTRError errorForCHIPErrorCode:aTerminationCause];
    CallResubscriptionScheduledHandler(error, @(timeTillNextResubscriptionMs));

    return CHIP_NO_ERROR;
}
} // anonymous namespace
