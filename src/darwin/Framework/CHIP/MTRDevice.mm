/**
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
#import "MTRDevice_Internal.h"
#import "MTRError_Internal.h"
#import "MTREventTLVValueDecoder_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRMetricKeys.h"
#import "MTRMetricsCollector.h"
#import "MTRTimeUtils.h"
#import "MTRUnfairLock.h"
#import "zap-generated/MTRCommandPayloads_Internal.h"

#include "lib/core/CHIPError.h"
#include "lib/core/DataModelTypes.h"
#include <app/ConcreteAttributePath.h>
#include <lib/support/FibonacciUtils.h>

#include <app/AttributePathParams.h>
#include <app/BufferedReadCallback.h>
#include <app/ClusterStateCache.h>
#include <app/InteractionModelEngine.h>
#include <platform/LockTracker.h>
#include <platform/PlatformManager.h>

typedef void (^MTRDeviceAttributeReportHandler)(NSArray * _Nonnull);

NSString * const MTRPreviousDataKey = @"previousData";
NSString * const MTRDataVersionKey = @"dataVersion";

#define kSecondsToWaitBeforeMarkingUnreachableAfterSettingUpSubscription 10

// Consider moving utility classes to their own file
#pragma mark - Utility Classes
// This class is for storing weak references in a container
@interface MTRWeakReference<ObjectType> : NSObject
+ (instancetype)weakReferenceWithObject:(ObjectType)object;
- (instancetype)initWithObject:(ObjectType)object;
- (ObjectType)strongObject; // returns strong object or NULL
@end

@interface MTRWeakReference () {
@private
    __weak id _object;
}
@end

@implementation MTRWeakReference
- (instancetype)initWithObject:(id)object
{
    if (self = [super init]) {
        _object = object;
    }
    return self;
}
+ (instancetype)weakReferenceWithObject:(id)object
{
    return [[self alloc] initWithObject:object];
}
- (id)strongObject
{
    return _object;
}
@end

// convenience object for commonly-logged device attributes
@interface MTRDeviceInformationalAttributes : NSObject
@property (readonly) UInt16 vendorID;
@property (readonly) UInt16 productID;
@property (readonly) BOOL usesThread;

- (void)addInformationalAttributesToCurrentMetricScope;

@end

@implementation MTRDeviceInformationalAttributes

- (instancetype)initWithVendorID:(UInt16)vendorID productID:(UInt16)productID usesThread:(BOOL)usesThread {
    self = [super init];
    
    if (self) {
        _vendorID = vendorID;
        _productID = productID;
        _usesThread = usesThread;
    }
    
    return self;
}

- (void)addInformationalAttributesToCurrentMetricScope {
    using namespace chip::Tracing::DarwinFramework;
    MATTER_LOG_METRIC(kMetricDeviceVendorID, _vendorID);
    MATTER_LOG_METRIC(kMetricDeviceProductID, _productID);
    MATTER_LOG_METRIC(kMetricDeviceUsesThread, _usesThread);
}

@end

NSNumber * MTRClampedNumber(NSNumber * aNumber, NSNumber * min, NSNumber * max)
{
    if ([aNumber compare:min] == NSOrderedAscending) {
        return min;
    } else if ([aNumber compare:max] == NSOrderedDescending) {
        return max;
    }
    return aNumber;
}

/* BEGIN DRAGONS: Note methods here cannot be renamed, and are used by private callers, do not rename, remove or modify behavior here */

@interface NSObject (MatterPrivateForInternalDragonsDoNotFeed)
- (void)_deviceInternalStateChanged:(MTRDevice *)device;
@end

/* END DRAGONS */

#pragma mark - SubscriptionCallback class declaration
using namespace chip;
using namespace chip::app;
using namespace chip::Protocols::InteractionModel;

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
    void OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus) override;

    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override;

    CHIP_ERROR OnResubscriptionNeeded(chip::app::ReadClient * apReadClient, CHIP_ERROR aTerminationCause) override;

    // Copied from ReadClient and customized for MTRDevice resubscription time reset
    uint32_t ComputeTimeTillNextSubscription();
    uint32_t mResubscriptionNumRetries = 0;
};

} // anonymous namespace

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

@implementation MTRDeviceClusterData {
    NSMutableDictionary<NSNumber *, MTRDeviceDataValueDictionary> * _attributes;
}

static NSString * const sDataVersionKey = @"dataVersion";
static NSString * const sAttributesKey = @"attributes";
static NSString * const sLastInitialSubscribeLatencyKey = @"lastInitialSubscribeLatency";

- (void)storeValue:(MTRDeviceDataValueDictionary _Nullable)value forAttribute:(NSNumber *)attribute
{
    _attributes[attribute] = value;
}

- (void)removeValueForAttribute:(NSNumber *)attribute
{
    [_attributes removeObjectForKey:attribute];
}

- (NSDictionary<NSNumber *, MTRDeviceDataValueDictionary> *)attributes
{
    return _attributes;
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRDeviceClusterData: dataVersion %@ attributes count %lu>", _dataVersion, static_cast<unsigned long>(_attributes.count)];
}

- (nullable instancetype)init
{
    return [self initWithDataVersion:nil attributes:nil];
}

// Attributes dictionary is: attributeID => data-value dictionary
- (nullable instancetype)initWithDataVersion:(NSNumber * _Nullable)dataVersion attributes:(NSDictionary<NSNumber *, MTRDeviceDataValueDictionary> * _Nullable)attributes
{
    self = [super init];
    if (self == nil) {
        return nil;
    }

    _dataVersion = [dataVersion copy];
    _attributes = [NSMutableDictionary dictionaryWithCapacity:attributes.count];
    [_attributes addEntriesFromDictionary:attributes];

    return self;
}

- (nullable instancetype)initWithCoder:(NSCoder *)decoder
{
    self = [super init];
    if (self == nil) {
        return nil;
    }

    _dataVersion = [decoder decodeObjectOfClass:[NSNumber class] forKey:sDataVersionKey];
    if (_dataVersion != nil && ![_dataVersion isKindOfClass:[NSNumber class]]) {
        MTR_LOG_ERROR("MTRDeviceClusterData got %@ for data version, not NSNumber.", _dataVersion);
        return nil;
    }

    static NSSet * const sAttributeValueClasses = [NSSet setWithObjects:[NSDictionary class], [NSArray class], [NSData class], [NSString class], [NSNumber class], nil];
    _attributes = [decoder decodeObjectOfClasses:sAttributeValueClasses forKey:sAttributesKey];
    if (_attributes != nil && ![_attributes isKindOfClass:[NSDictionary class]]) {
        MTR_LOG_ERROR("MTRDeviceClusterData got %@ for attributes, not NSDictionary.", _attributes);
        return nil;
    }

    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:self.dataVersion forKey:sDataVersionKey];
    [coder encodeObject:self.attributes forKey:sAttributesKey];
}

- (id)copyWithZone:(NSZone *)zone
{
    return [[MTRDeviceClusterData alloc] initWithDataVersion:_dataVersion attributes:_attributes];
}

- (BOOL)isEqualToClusterData:(MTRDeviceClusterData *)otherClusterData
{
    return [_dataVersion isEqual:otherClusterData.dataVersion] && [_attributes isEqual:otherClusterData.attributes];
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }

    return [self isEqualToClusterData:object];
}

@end

// Minimal time to wait since our last resubscribe failure before we will allow
// a read attempt to prod our subscription.
//
// TODO: Figure out a better value for this, but for now don't allow this to
// happen more often than once every 10 minutes.
#define MTRDEVICE_MIN_RESUBSCRIBE_DUE_TO_READ_INTERVAL_SECONDS (10 * 60)

@interface MTRDevice ()
@property (nonatomic, readonly) os_unfair_lock lock; // protects the caches and device state
// protects against concurrent time updates by guarding timeUpdateScheduled flag which manages time updates scheduling,
// and protects device calls to setUTCTime and setDSTOffset
@property (nonatomic, readonly) os_unfair_lock timeSyncLock;
@property (nonatomic) chip::FabricIndex fabricIndex;
@property (nonatomic) MTRWeakReference<id<MTRDeviceDelegate>> * weakDelegate;
@property (nonatomic) dispatch_queue_t delegateQueue;
@property (nonatomic) NSMutableArray<NSDictionary<NSString *, id> *> * unreportedEvents;
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

@property (nonatomic) BOOL timeUpdateScheduled;

@property (nonatomic) NSDate * estimatedStartTimeFromGeneralDiagnosticsUpTime;

@property (nonatomic) NSMutableDictionary * temporaryMetaDataCache;

/**
 * If currentReadClient is non-null, that means that we successfully
 * called SendAutoResubscribeRequest on the ReadClient and have not yet gotten
 * an OnDone for that ReadClient.
 */
@property (nonatomic) ReadClient * currentReadClient;
@property (nonatomic) SubscriptionCallback * currentSubscriptionCallback; // valid when and only when currentReadClient is valid

@end

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
@end
#endif

@implementation MTRDevice {
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
}

- (instancetype)initWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller
{
    if (self = [super init]) {
        _lock = OS_UNFAIR_LOCK_INIT;
        _timeSyncLock = OS_UNFAIR_LOCK_INIT;
        _nodeID = [nodeID copy];
        _fabricIndex = controller.fabricIndex;
        _deviceController = controller;
        _queue
            = dispatch_queue_create("org.csa-iot.matter.framework.device.workqueue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
        _expectedValueCache = [NSMutableDictionary dictionary];
        _asyncWorkQueue = [[MTRAsyncWorkQueue alloc] initWithContext:self];
        _state = MTRDeviceStateUnknown;
        _internalDeviceState = MTRInternalDeviceStateUnsubscribed;
        if (controller.controllerDataStore) {
            _persistedClusterData = [[NSCache alloc] init];
        } else {
            _persistedClusterData = nil;
        }
        _clusterDataToPersist = nil;
        _persistedClusters = [NSMutableSet set];

        // If there is a data store, make sure we have an observer to
        if (_persistedClusterData) {
            mtr_weakify(self);
            _systemTimeChangeObserverToken = [[NSNotificationCenter defaultCenter] addObserverForName:NSSystemClockDidChangeNotification object:nil queue:nil usingBlock:^(NSNotification * _Nonnull notification) {
                mtr_strongify(self);
                std::lock_guard lock(self->_lock);
                [self _resetStorageBehaviorState];
            }];
        }

        MTR_LOG_DEBUG("%@ init with hex nodeID 0x%016llX", self, _nodeID.unsignedLongLongValue);
    }
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:_systemTimeChangeObserverToken];
}

- (NSString *)description
{
    return [NSString
        stringWithFormat:@"<MTRDevice: %p>[fabric: %u, nodeID: 0x%016llX]", self, _fabricIndex, _nodeID.unsignedLongLongValue];
}

+ (MTRDevice *)deviceWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller
{
    return [controller deviceForNodeID:nodeID];
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
    MTRWeakReference<MTRDevice *> * weakSelf = [MTRWeakReference weakReferenceWithObject:self];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (nextUpdateInSeconds * NSEC_PER_SEC)), self.queue, ^{
        MTR_LOG_DEBUG("%@ Timer expired, start Device Time Update", self);
        MTRDevice * strongSelf = weakSelf.strongObject;
        if (strongSelf) {
            [strongSelf _performScheduledTimeUpdate];
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
    auto partsList = [self readAttributeWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeDescriptorID) attributeID:@(MTRAttributeIDTypeClusterDescriptorAttributePartsListID) params:nil];
    NSMutableArray<NSNumber *> * endpointsOnDevice = [self arrayOfNumbersFromAttributeValue:partsList];
    if (!endpointsOnDevice) {
        endpointsOnDevice = [[NSMutableArray<NSNumber *> alloc] init];
    }
    // Add Root node!
    [endpointsOnDevice addObject:@(0)];

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
    auto setUTCTimeResponseHandler = ^(id _Nullable response, NSError * _Nullable error) {
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

    auto setDSTOffsetResponseHandler = ^(id _Nullable response, NSError * _Nullable error) {
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

- (BOOL)_subscriptionsAllowed
{
    os_unfair_lock_assert_owner(&self->_lock);

    // We should not allow a subscription for device controllers over XPC.
    return ![_deviceController isKindOfClass:MTRDeviceControllerOverXPC.class];
}

- (void)setDelegate:(id<MTRDeviceDelegate>)delegate queue:(dispatch_queue_t)queue
{
    MTR_LOG("%@ setDelegate %@", self, delegate);

    std::lock_guard lock(_lock);

    BOOL setUpSubscription = [self _subscriptionsAllowed];

    // For unit testing only. If this ever changes to not being for unit testing purposes,
    // we would need to move the code outside of where we acquire the lock above.
#ifdef DEBUG
    id testDelegate = delegate;
    if ([testDelegate respondsToSelector:@selector(unitTestShouldSetUpSubscriptionForDevice:)]) {
        setUpSubscription = [testDelegate unitTestShouldSetUpSubscriptionForDevice:self];
    }
#endif

    _weakDelegate = [MTRWeakReference weakReferenceWithObject:delegate];
    _delegateQueue = queue;

    if (setUpSubscription) {
        _initialSubscribeStart = [NSDate now];
        if ([self _deviceUsesThread]) {
            [self _scheduleSubscriptionPoolWork:^{
                std::lock_guard lock(self->_lock);
                [self _setupSubscriptionWithReason:@"delegate is set and scheduled subscription is happening"];
            } inNanoseconds:0 description:@"MTRDevice setDelegate first subscription"];
        } else {
            [self _setupSubscriptionWithReason:@"delegate is set and subscription is needed"];
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

    _state = MTRDeviceStateUnknown;

    _weakDelegate = nil;

    // Make sure we don't try to resubscribe if we have a pending resubscribe
    // attempt, since we now have no delegate.
    _reattemptingSubscription = NO;

    // We do not change _internalDeviceState here, because we might still have a
    // subscription.  In that case, _internalDeviceState will update when the
    // subscription is actually terminated.

    [self _stopConnectivityMonitoring];

    os_unfair_lock_unlock(&self->_lock);
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
    if (self.reattemptingSubscription) {
        [self _reattemptSubscriptionNowIfNeededWithReason:reason];
    } else {
        readClientToResubscribe = self->_currentReadClient;
        subscriptionCallback = self->_currentSubscriptionCallback;
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
    }
}

// Return YES if we are in a state where, apart from communication issues with
// the device, we will be able to get reports via our subscription.
- (BOOL)_subscriptionAbleToReport
{
    std::lock_guard lock(_lock);
    id<MTRDeviceDelegate> delegate = _weakDelegate.strongObject;
    if (delegate == nil) {
        // No delegate definitely means no subscription.
        return NO;
    }

    // For unit testing only, matching logic in setDelegate
#ifdef DEBUG
    id testDelegate = delegate;
    if ([testDelegate respondsToSelector:@selector(unitTestShouldSetUpSubscriptionForDevice:)]) {
        if (![testDelegate unitTestShouldSetUpSubscriptionForDevice:self]) {
            return NO;
        }
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
    [_deviceController asyncDispatchToMatterQueue:^{
        [self _triggerResubscribeWithReason:@"read-through skipped while not subscribed" nodeLikelyReachable:NO];
    }
                                     errorHandler:nil];
}

- (BOOL)_callDelegateWithBlock:(void (^)(id<MTRDeviceDelegate>))block
{
    os_unfair_lock_assert_owner(&self->_lock);
    id<MTRDeviceDelegate> delegate = _weakDelegate.strongObject;
    if (delegate) {
        dispatch_async(_delegateQueue, ^{
            block(delegate);
        });
        return YES;
    }
    return NO;
}

- (void)_callDelegateDeviceCachePrimed
{
    os_unfair_lock_assert_owner(&self->_lock);
    [self _callDelegateWithBlock:^(id<MTRDeviceDelegate> delegate) {
        if ([delegate respondsToSelector:@selector(deviceCachePrimed:)]) {
            [delegate deviceCachePrimed:self];
        }
    }];
}

// assume lock is held
- (void)_changeState:(MTRDeviceState)state
{
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
        id<MTRDeviceDelegate> delegate = _weakDelegate.strongObject;
        if (delegate) {
            dispatch_async(_delegateQueue, ^{
                [delegate device:self stateChanged:state];
            });
        }
    } else {
        MTR_LOG(
            "%@ Not reporting reachability state change, since no change in state %lu => %lu", self, static_cast<unsigned long>(lastState), static_cast<unsigned long>(state));
    }
}

- (void)_changeInternalState:(MTRInternalDeviceState)state
{
    os_unfair_lock_assert_owner(&self->_lock);
    MTRInternalDeviceState lastState = _internalDeviceState;
    _internalDeviceState = state;
    if (lastState != state) {
        MTR_LOG("%@ internal state change %lu => %lu", self, static_cast<unsigned long>(lastState), static_cast<unsigned long>(state));

        /* BEGIN DRAGONS: This is a huge hack for a specific use case, do not rename, remove or modify behavior here */
        // TODO: This should only be called for thread devices
        id<MTRDeviceDelegate> delegate = _weakDelegate.strongObject;
        if ([delegate respondsToSelector:@selector(_deviceInternalStateChanged:)]) {
            dispatch_async(_delegateQueue, ^{
                [(id) delegate _deviceInternalStateChanged:self];
            });
        }
        /* END DRAGONS */
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

    // We have completed the subscription work - remove from the subscription pool.
    [self _clearSubscriptionPoolWork];

    // reset subscription attempt wait time when subscription succeeds
    _lastSubscriptionAttemptWait = 0;
    if (HadSubscriptionEstablishedOnce(_internalDeviceState)) {
        [self _changeInternalState:MTRInternalDeviceStateLaterSubscriptionEstablished];
    } else {
        [self _changeInternalState:MTRInternalDeviceStateInitialSubscriptionEstablished];
    }

    [self _changeState:MTRDeviceStateReachable];

    // No need to monitor connectivity after subscription establishment
    [self _stopConnectivityMonitoring];

    auto initialSubscribeStart = _initialSubscribeStart;
    // We no longer need to track subscribe latency for this device.
    _initialSubscribeStart = nil;

    if (initialSubscribeStart != nil) {
        // We want time interval from initialSubscribeStart to now, not the other
        // way around.
        NSTimeInterval subscriptionLatency = -[initialSubscribeStart timeIntervalSinceNow];
        _estimatedSubscriptionLatency = @(subscriptionLatency);
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
    std::lock_guard lock(_lock);

    [self _changeInternalState:MTRInternalDeviceStateUnsubscribed];
    _unreportedEvents = nil;

    [self _changeState:MTRDeviceStateUnreachable];
}

// This method is used for signaling whether to use the subscription pool. This functions as
// a heuristic for whether to throttle subscriptions to the device via a pool of subscriptions.
// If products appear that have both Thread and Wifi enabled but are primarily on wifi, this
// method will need to be updated to reflect that.
- (BOOL)_deviceUsesThread
{
    os_unfair_lock_assert_owner(&self->_lock);

#ifdef DEBUG
    id testDelegate = _weakDelegate.strongObject;
    if (testDelegate) {
        // Note: This is a hack to allow our unit tests to test the subscription pooling behavior we have implemented for thread, so we mock devices to be a thread device
        if ([testDelegate respondsToSelector:@selector(unitTestPretendThreadEnabled:)]) {
            if ([testDelegate unitTestPretendThreadEnabled:self]) {
                return YES;
            }
        }
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
        id delegate = self->_weakDelegate.strongObject;
        if (delegate) {
            dispatch_async(self->_delegateQueue, ^{
                if ([delegate respondsToSelector:@selector(unitTestSubscriptionPoolWorkComplete:)]) {
                    [delegate unitTestSubscriptionPoolWorkComplete:self];
                }
            });
        }
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
        MTR_LOG_ERROR("%@ already scheduled in subscription pool for this device - ignoring: %@", self, description);
        return;
    }

    // Wait the required amount of time, then put it in the subscription pool to wait additionally for a spot, if needed
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, inNanoseconds), dispatch_get_main_queue(), ^{
        // In the case where a resubscription triggering event happened and already established, running the work block should result in a no-op
        MTRAsyncWorkItem * workItem = [[MTRAsyncWorkItem alloc] initWithQueue:self.queue];
        [workItem setReadyHandler:^(id _Nonnull context, NSInteger retryCount, MTRAsyncWorkCompletionBlock _Nonnull completion) {
            os_unfair_lock_lock(&self->_lock);
#ifdef DEBUG
            id delegate = self->_weakDelegate.strongObject;
            if (delegate) {
                dispatch_async(self->_delegateQueue, ^{
                    if ([delegate respondsToSelector:@selector(unitTestSubscriptionPoolDequeue:)]) {
                        [delegate unitTestSubscriptionPoolDequeue:self];
                    }
                });
            }
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
        [self->_deviceController.concurrentSubscriptionPool enqueueWorkItem:workItem description:description];
    });
}

- (void)_handleResubscriptionNeededWithDelay:(NSNumber *)resubscriptionDelayMs
{
    BOOL deviceUsesThread;

    os_unfair_lock_lock(&self->_lock);

    [self _changeState:MTRDeviceStateUnknown];
    [self _changeInternalState:MTRInternalDeviceStateResubscribing];

    // If we are here, then the ReadClient either just detected a subscription
    // drop or just tried again and failed.  Either way, count it as "tried and
    // failed to subscribe": in the latter case it's actually true, and in the
    // former case we recently had a subscription and do not want to be forcing
    // retries immediately.
    _lastSubscriptionFailureTime = [NSDate now];

    deviceUsesThread = [self _deviceUsesThread];

    // If a previous resubscription failed, remove the item from the subscription pool.
    [self _clearSubscriptionPoolWork];

    os_unfair_lock_unlock(&self->_lock);

    // Use the existing _triggerResubscribeWithReason mechanism, which does the right checks when
    // this block is run -- if other triggering events had happened, this would become a no-op.
    auto resubscriptionBlock = ^{
        [self->_deviceController asyncDispatchToMatterQueue:^{
            [self _triggerResubscribeWithReason:@"ResubscriptionNeeded timer fired" nodeLikelyReachable:NO];
        } errorHandler:^(NSError * _Nonnull error) {
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
        [self _scheduleSubscriptionPoolWork:resubscriptionBlock inNanoseconds:resubscriptionDelayNs description:@"ReadClient resubscription"];
    } else {
        // For non-Thread-enabled devices, just call the resubscription block after the specified time
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, resubscriptionDelayNs), self.queue, resubscriptionBlock);
    }

    // Set up connectivity monitoring in case network routability changes for the positive, to accellerate resubscription
    [self _setupConnectivityMonitoring];
}

- (void)_handleSubscriptionReset:(NSNumber * _Nullable)retryDelay
{
    std::lock_guard lock(_lock);

    // If we are here, then either we failed to establish initil CASE, or we
    // failed to send the initial SubscribeRequest message, or our ReadClient
    // has given up completely.  Those all count as "we have tried and failed to
    // subscribe".
    _lastSubscriptionFailureTime = [NSDate now];

    // if there is no delegate then also do not retry
    id<MTRDeviceDelegate> delegate = _weakDelegate.strongObject;
    if (!delegate) {
        // NOTE: Do not log anything here: we have been invalidated, and the
        // Matter stack might already be torn down.
        return;
    }

    // don't schedule multiple retries
    if (self.reattemptingSubscription) {
        MTR_LOG("%@ already reattempting subscription", self);
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
        _lastSubscriptionAttemptWait = 0;
        secondsToWait = retryDelay.doubleValue;
        MTR_LOG("%@ resetting resubscribe attempt counter, and delaying by the server-provided delay: %f",
            self, secondsToWait);
    } else {
        _lastSubscriptionAttemptWait *= 2;
        if (_lastSubscriptionAttemptWait > MTRDEVICE_SUBSCRIPTION_ATTEMPT_MAX_WAIT_SECONDS) {
            _lastSubscriptionAttemptWait = MTRDEVICE_SUBSCRIPTION_ATTEMPT_MAX_WAIT_SECONDS;
        }
        secondsToWait = _lastSubscriptionAttemptWait;
    }

    MTR_LOG("%@ scheduling to reattempt subscription in %f seconds", self, secondsToWait);

    // If we started subscription or session establishment but failed, remove item from the subscription pool so we can re-queue.
    [self _clearSubscriptionPoolWork];

    // Call _reattemptSubscriptionNowIfNeededWithReason when timer fires - if subscription is
    // in a better state at that time this will be a no-op.
    auto resubscriptionBlock = ^{
        os_unfair_lock_lock(&self->_lock);
        [self _reattemptSubscriptionNowIfNeededWithReason:@"got subscription reset"];
        os_unfair_lock_unlock(&self->_lock);
    };

    int64_t resubscriptionDelayNs = static_cast<int64_t>(secondsToWait * NSEC_PER_SEC);
    if ([self _deviceUsesThread]) {
        // For Thread-enabled devices, schedule the _reattemptSubscriptionNowIfNeededWithReason call to run in the subscription pool
        [self _scheduleSubscriptionPoolWork:resubscriptionBlock inNanoseconds:resubscriptionDelayNs description:@"MTRDevice resubscription"];
    } else {
        // For non-Thread-enabled devices, just call the resubscription block after the specified time
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, resubscriptionDelayNs), self.queue, resubscriptionBlock);
    }
}

- (void)_reattemptSubscriptionNowIfNeededWithReason:(NSString *)reason
{
    os_unfair_lock_assert_owner(&self->_lock);
    if (!self.reattemptingSubscription) {
        return;
    }

    MTR_LOG("%@ reattempting subscription", self);
    self.reattemptingSubscription = NO;
    [self _setupSubscriptionWithReason:reason];
}

- (void)_handleUnsolicitedMessageFromPublisher
{
    std::lock_guard lock(_lock);

    [self _changeState:MTRDeviceStateReachable];

    id<MTRDeviceDelegate> delegate = _weakDelegate.strongObject;
    if (delegate) {
        dispatch_async(_delegateQueue, ^{
            if ([delegate respondsToSelector:@selector(deviceBecameActive:)]) {
                [delegate deviceBecameActive:self];
            }
        });
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
    os_unfair_lock_assert_owner(&self->_lock);

    if (HadSubscriptionEstablishedOnce(_internalDeviceState)) {
        return;
    }

    MTR_LOG("%@ still not subscribed, marking the device as unreachable", self);
    [self _changeState:MTRDeviceStateUnreachable];
}

- (void)_handleReportBegin
{
    std::lock_guard lock(_lock);

    _receivingReport = YES;
    if (_state != MTRDeviceStateReachable) {
        [self _changeState:MTRDeviceStateReachable];
    }

    // If we currently don't have an established subscription, this must be a
    // priming report.
    _receivingPrimingReport = !HaveSubscriptionEstablishedRightNow(_internalDeviceState);
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

- (void)_persistClusterData
{
    os_unfair_lock_assert_owner(&self->_lock);

    // Nothing to persist
    if (!_clusterDataToPersist.count) {
        return;
    }

    MTR_LOG("%@ Storing cluster information (data version and attributes) count: %lu", self, static_cast<unsigned long>(_clusterDataToPersist.count));
    // We're going to hand out these MTRDeviceClusterData objects to our
    // storage implementation, which will try to read them later.  Make sure
    // we snapshot the state here instead of handing out live copies.
    NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> * clusterData = [self _clusterDataToPersistSnapshot];
    [_deviceController.controllerDataStore storeClusterData:clusterData forNodeID:_nodeID];
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

#ifdef DEBUG
    id delegate = _weakDelegate.strongObject;
    if (delegate) {
        dispatch_async(_delegateQueue, ^{
            if ([delegate respondsToSelector:@selector(unitTestClusterDataPersisted:)]) {
                [delegate unitTestClusterDataPersisted:self];
            }
        });
    }
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

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) ([self _reportToPersistenceDelayTimeAfterMutiplier] * NSEC_PER_SEC)), self.queue, ^{
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
    _deviceReportingExcessivelyStartTime = nil;
    _reportToPersistenceDelayCurrentMultiplier = 1;

    if (_persistedClusters) {
        [self _persistClusterData];
    }
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
    std::lock_guard lock(_lock);
    _receivingReport = NO;
    _receivingPrimingReport = NO;
    _estimatedStartTimeFromGeneralDiagnosticsUpTime = nil;

    [self _scheduleClusterDataPersistence];

    // After the handling of the report, if we detected a device configuration change, notify the delegate
    // of the same.
    if (_deviceConfigurationChanged) {
        id<MTRDeviceDelegate> delegate = _weakDelegate.strongObject;
        if (delegate) {
            dispatch_async(_delegateQueue, ^{
                if ([delegate respondsToSelector:@selector(deviceConfigurationChanged:)])
                    [delegate deviceConfigurationChanged:self];
            });
        }
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
    }

// For unit testing only
#ifdef DEBUG
    id delegate = _weakDelegate.strongObject;
    if (delegate) {
        dispatch_async(_delegateQueue, ^{
            if ([delegate respondsToSelector:@selector(unitTestReportEndForDevice:)]) {
                [delegate unitTestReportEndForDevice:self];
            }
        });
    }
#endif
}

// assume lock is held
- (void)_reportAttributes:(NSArray<NSDictionary<NSString *, id> *> *)attributes
{
    os_unfair_lock_assert_owner(&self->_lock);
    if (attributes.count) {
        id<MTRDeviceDelegate> delegate = _weakDelegate.strongObject;
        if (delegate) {
            dispatch_async(_delegateQueue, ^{
                [delegate device:self receivedAttributeReport:attributes];
            });
        }
    }
}

- (void)_handleAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport fromSubscription:(BOOL)isFromSubscription
{
    std::lock_guard lock(_lock);

    // _getAttributesToReportWithReportedValues will log attribute paths reported
    [self _reportAttributes:[self _getAttributesToReportWithReportedValues:attributeReport fromSubscription:isFromSubscription]];
}

#ifdef DEBUG
- (void)unitTestInjectEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport
{
    dispatch_async(self.queue, ^{
        [self _handleEventReport:eventReport];
    });
}

- (void)unitTestInjectAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport fromSubscription:(BOOL)isFromSubscription
{
    dispatch_async(self.queue, ^{
        [self _handleReportBegin];
        [self _handleAttributeReport:attributeReport fromSubscription:isFromSubscription];
        [self _handleReportEnd];
    });
}
#endif

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

        NSMutableDictionary * eventToReturn = eventDict.mutableCopy;
        if (_receivingPrimingReport) {
            eventToReturn[MTREventIsHistoricalKey] = @(YES);
        } else {
            eventToReturn[MTREventIsHistoricalKey] = @(NO);
        }

        [reportToReturn addObject:eventToReturn];
    }
    if (oldEstimatedStartTime != _estimatedStartTime) {
        MTR_LOG("%@ updated estimated start time to %@", self, _estimatedStartTime);
    }

    id<MTRDeviceDelegate> delegate = _weakDelegate.strongObject;
    if (delegate) {
        _unreportedEvents = nil;
        dispatch_async(_delegateQueue, ^{
            [delegate device:self receivedEventReport:reportToReturn];
        });
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

    NSAssert(_deviceController.controllerDataStore != nil,
        @"How can _persistedClusters have an entry if we have no persistence?");
    NSAssert(_persistedClusterData != nil,
        @"How can _persistedClusterData not exist if we have persisted clusters?");

    // Page in the stored value for the data.
    MTRDeviceClusterData * data = [_deviceController.controllerDataStore getStoredClusterDataForNodeID:_nodeID endpointID:clusterPath.endpoint clusterID:clusterPath.cluster];
    if (data != nil) {
        [_persistedClusterData setObject:data forKey:clusterPath];
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

    if (value != nil
        && isFromSubscription
        && !_receivingPrimingReport
        && AttributeHasChangesOmittedQuality(path)) {
        // Do not persist new values for Changes Omitted Quality (aka C Quality)
        // attributes unless they're part of a Priming Report or from a read response.
        // (removals are OK)
        
        // log when a device violates expectations for Changes Omitted Quality attributes.
        MTRDeviceInformationalAttributes * attributes = [self _informationalAttributesForCurrentState];
        
        using namespace chip::Tracing::DarwinFramework;
        MATTER_LOG_METRIC_BEGIN(kMetricUnexpectedCQualityUpdate);
        [attributes addInformationalAttributesToCurrentMetricScope];
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

- (void)_createDataVersionFilterListFromDictionary:(NSDictionary<MTRClusterPath *, NSNumber *> *)dataVersions dataVersionFilterList:(DataVersionFilter **)dataVersionFilterList count:(size_t *)count sizeReduction:(size_t)sizeReduction
{
    size_t maxDataVersionFilterSize = dataVersions.count;

    // Check if any filter list should be generated
    if (!dataVersions.count || (maxDataVersionFilterSize <= sizeReduction)) {
        *count = 0;
        *dataVersionFilterList = nullptr;
        return;
    }
    maxDataVersionFilterSize -= sizeReduction;

    DataVersionFilter * dataVersionFilterArray = new DataVersionFilter[maxDataVersionFilterSize];
    size_t i = 0;
    for (MTRClusterPath * path in dataVersions) {
        NSNumber * dataVersionNumber = dataVersions[path];
        if (dataVersionNumber) {
            dataVersionFilterArray[i++] = DataVersionFilter(static_cast<chip::EndpointId>(path.endpoint.unsignedShortValue), static_cast<chip::ClusterId>(path.cluster.unsignedLongValue), static_cast<chip::DataVersion>(dataVersionNumber.unsignedLongValue));
        }
        if (i == maxDataVersionFilterSize) {
            break;
        }
    }

    *dataVersionFilterList = dataVersionFilterArray;
    *count = maxDataVersionFilterSize;
}

- (void)_setupConnectivityMonitoring
{
    // Dispatch to own queue first to avoid deadlock with syncGetCompressedFabricID
    dispatch_async(self.queue, ^{
        // Get the required info before setting up the connectivity monitor
        NSNumber * compressedFabricID = [self->_deviceController syncGetCompressedFabricID];
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
            [self->_deviceController asyncDispatchToMatterQueue:^{
                [self _triggerResubscribeWithReason:@"device connectivity changed" nodeLikelyReachable:YES];
            }
                                                   errorHandler:nil];
        } queue:self.queue];
    });
}

- (void)_stopConnectivityMonitoring
{
    os_unfair_lock_assert_owner(&_lock);

    if (_connectivityMonitor) {
        [_connectivityMonitor stopMonitoring];
        _connectivityMonitor = nil;
    }
}

// assume lock is held
- (void)_setupSubscriptionWithReason:(NSString *)reason
{
    os_unfair_lock_assert_owner(&self->_lock);

    if (![self _subscriptionsAllowed]) {
        MTR_LOG("%@ _setupSubscription: Subscriptions not allowed. Do not set up subscription", self);
        return;
    }

#ifdef DEBUG
    id delegate = _weakDelegate.strongObject;
    Optional<System::Clock::Seconds32> maxIntervalOverride;
    if (delegate) {
        if ([delegate respondsToSelector:@selector(unitTestMaxIntervalOverrideForSubscription:)]) {
            NSNumber * delegateMin = [delegate unitTestMaxIntervalOverrideForSubscription:self];
            maxIntervalOverride.Emplace(delegateMin.unsignedIntValue);
        }
    }
#endif

    // for now just subscribe once
    if (!NeedToStartSubscriptionSetup(_internalDeviceState)) {
        return;
    }

    [self _changeInternalState:MTRInternalDeviceStateSubscribing];

    MTR_LOG("%@ setting up subscription with reason: %@", self, reason);

    // Set up a timer to mark as not reachable if it takes too long to set up a subscription
    MTRWeakReference<MTRDevice *> * weakSelf = [MTRWeakReference weakReferenceWithObject:self];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(kSecondsToWaitBeforeMarkingUnreachableAfterSettingUpSubscription) * static_cast<int64_t>(NSEC_PER_SEC)), self.queue, ^{
        MTRDevice * strongSelf = weakSelf.strongObject;
        if (strongSelf != nil) {
            std::lock_guard lock(strongSelf->_lock);
            [strongSelf _markDeviceAsUnreachableIfNeverSubscribed];
        }
    });

    [_deviceController
        getSessionForNode:_nodeID.unsignedLongLongValue
               completion:^(chip::Messaging::ExchangeManager * _Nullable exchangeManager,
                   const chip::Optional<chip::SessionHandle> & session, NSError * _Nullable error,
                   NSNumber * _Nullable retryDelay) {
                   if (error != nil) {
                       MTR_LOG_ERROR("%@ getSessionForNode error %@", self, error);
                       dispatch_async(self.queue, ^{
                           [self _handleSubscriptionError:error];
                           [self _handleSubscriptionReset:retryDelay];
                       });
                       return;
                   }

                   auto callback = std::make_unique<SubscriptionCallback>(
                       ^(NSArray * value) {
                           MTR_LOG("%@ got attribute report %@", self, value);
                           dispatch_async(self.queue, ^{
                               // OnAttributeData
                               [self _handleAttributeReport:value fromSubscription:YES];
#ifdef DEBUG
                               self->_unitTestAttributesReportedSinceLastCheck += value.count;
#endif
                           });
                       },
                       ^(NSArray * value) {
                           MTR_LOG("%@ got event report %@", self, value);
                           dispatch_async(self.queue, ^{
                               // OnEventReport
                               [self _handleEventReport:value];
                           });
                       },
                       ^(NSError * error) {
                           MTR_LOG_ERROR("%@ got subscription error %@", self, error);
                           dispatch_async(self.queue, ^{
                               // OnError
                               [self _handleSubscriptionError:error];
                           });
                       },
                       ^(NSError * error, NSNumber * resubscriptionDelayMs) {
                           MTR_LOG_ERROR("%@ got resubscription error %@ delay %@", self, error, resubscriptionDelayMs);
                           dispatch_async(self.queue, ^{
                               // OnResubscriptionNeeded
                               [self _handleResubscriptionNeededWithDelay:resubscriptionDelayMs];
                           });
                       },
                       ^(void) {
                           MTR_LOG("%@ got subscription established", self);
                           dispatch_async(self.queue, ^{
                               // OnSubscriptionEstablished
                               [self _handleSubscriptionEstablished];
                           });
                       },
                       ^(void) {
                           MTR_LOG("%@ got subscription done", self);
                           // Drop our pointer to the ReadClient immediately, since
                           // it's about to be destroyed and we don't want to be
                           // holding a dangling pointer.
                           std::lock_guard lock(self->_lock);
                           self->_currentReadClient = nullptr;
                           self->_currentSubscriptionCallback = nullptr;

                           dispatch_async(self.queue, ^{
                               // OnDone
                               [self _handleSubscriptionReset:nil];
                           });
                       },
                       ^(void) {
                           MTR_LOG("%@ got unsolicited message from publisher", self);
                           dispatch_async(self.queue, ^{
                               // OnUnsolicitedMessageFromPublisher
                               [self _handleUnsolicitedMessageFromPublisher];
                           });
                       },
                       ^(void) {
                           MTR_LOG("%@ got report begin", self);
                           dispatch_async(self.queue, ^{
                               [self _handleReportBegin];
                           });
                       },
                       ^(void) {
                           MTR_LOG("%@ got report end", self);
                           dispatch_async(self.queue, ^{
                               [self _handleReportEnd];
                           });
                       });

                   // Set up a cluster state cache.  We just want this for the logic it has for
                   // tracking data versions and event numbers so we minimize the amount of data we
                   // request on resubscribes, so tell it not to store data.
                   auto clusterStateCache = std::make_unique<ClusterStateCache>(*callback.get(),
                       /* highestReceivedEventNumber = */ NullOptional,
                       /* cacheData = */ false);
                   auto readClient = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), exchangeManager,
                       clusterStateCache->GetBufferedCallback(), ReadClient::InteractionType::Subscribe);

                   // Subscribe with data version filter list and retry with smaller list if out of packet space
                   CHIP_ERROR err;
                   NSDictionary<MTRClusterPath *, NSNumber *> * dataVersions = [self _getCachedDataVersions];
                   size_t dataVersionFilterListSizeReduction = 0;
                   for (;;) {
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
                       size_t dataVersionFilterListSize = 0;
                       DataVersionFilter * dataVersionFilterList;
                       [self _createDataVersionFilterListFromDictionary:dataVersions dataVersionFilterList:&dataVersionFilterList count:&dataVersionFilterListSize sizeReduction:dataVersionFilterListSizeReduction];
                       readParams.mDataVersionFilterListSize = dataVersionFilterListSize;
                       readParams.mpDataVersionFilterList = dataVersionFilterList;
                       attributePath.release();
                       eventPath.release();

                       // TODO: Change from local filter list generation to rehydrating ClusterStateCache ot take advantage of existing filter list sorting algorithm

                       // SendAutoResubscribeRequest cleans up the params, even on failure.
                       err = readClient->SendAutoResubscribeRequest(std::move(readParams));
                       if (err == CHIP_NO_ERROR) {
                           break;
                       }

                       // If error is not a "no memory" issue, then break and go through regular resubscribe logic
                       if (err != CHIP_ERROR_NO_MEMORY) {
                           break;
                       }

                       // If "no memory" error is not caused by data version filter list, break as well
                       if (!dataVersionFilterListSize) {
                           break;
                       }

                       // Now "no memory" could mean subscribe request packet space ran out. Reduce size and try again immediately
                       dataVersionFilterListSizeReduction++;
                   }

                   if (err != CHIP_NO_ERROR) {
                       NSError * error = [MTRError errorForCHIPErrorCode:err logContext:self];
                       MTR_LOG_ERROR("%@ SendAutoResubscribeRequest error %@", self, error);
                       dispatch_async(self.queue, ^{
                           [self _handleSubscriptionError:error];
                           [self _handleSubscriptionReset:nil];
                       });

                       return;
                   }

                   MTR_LOG("%@ Subscribe with data version list size %lu, reduced by %lu", self, static_cast<unsigned long>(dataVersions.count), static_cast<unsigned long>(dataVersionFilterListSizeReduction));

                   // Callback and ClusterStateCache and ReadClient will be deleted
                   // when OnDone is called.
                   os_unfair_lock_lock(&self->_lock);
                   self->_currentReadClient = readClient.get();
                   self->_currentSubscriptionCallback = callback.get();
                   os_unfair_lock_unlock(&self->_lock);
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
    if (![self _subscriptionAbleToReport] || hasChangesOmittedQuality) {
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
        [workItem setReadyHandler:^(MTRDevice * self, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
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
            [baseDevice
                readAttributePaths:attributePaths
                        eventPaths:nil
                            params:readParams
                includeDataVersion:YES
                             queue:self.queue
                        completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                            if (values) {
                                // Since the format is the same data-value dictionary, this looks like an
                                // attribute report
                                MTR_LOG("Read attribute work item [%llu] result: %@  [0x%016llX:%@:0x%llX:0x%llX]", workItemID, values, nodeID.unsignedLongLongValue, endpointID, clusterID.unsignedLongLongValue, attributeID.unsignedLongLongValue);
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
        [_asyncWorkQueue enqueueWorkItem:workItem descriptionWithFormat:@"read %@ 0x%llx 0x%llx", endpointID, clusterID.unsignedLongLongValue, attributeID.unsignedLongLongValue];
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
    if (timeout) {
        timeout = MTRClampedNumber(timeout, @(1), @(UINT16_MAX));
    }
    expectedValueInterval = MTRClampedNumber(expectedValueInterval, @(1), @(UINT32_MAX));
    MTRAttributePath * attributePath = [MTRAttributePath attributePathWithEndpointID:endpointID
                                                                           clusterID:clusterID

                                                                         attributeID:attributeID];

    BOOL useValueAsExpectedValue = YES;
#ifdef DEBUG
    os_unfair_lock_lock(&self->_lock);
    id delegate = _weakDelegate.strongObject;
    os_unfair_lock_unlock(&self->_lock);
    if ([delegate respondsToSelector:@selector(unitTestShouldSkipExpectedValuesForWrite:)]) {
        useValueAsExpectedValue = ![delegate unitTestShouldSkipExpectedValuesForWrite:self];
    }
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
    auto * writeData = @[ attributePath, [value copy], timeout ?: [NSNull null], @(expectedValueID) ];

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
    [workItem setReadyHandler:^(MTRDevice * self, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
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

        [baseDevice
            writeAttributeWithEndpointID:path.endpoint
                               clusterID:path.cluster
                             attributeID:path.attribute
                                   value:request[MTRDeviceWriteRequestFieldValueIndex]
                       timedWriteTimeout:timedWriteTimeout
                                   queue:self.queue
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
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
    [_asyncWorkQueue enqueueWorkItem:workItem descriptionWithFormat:@"write %@ 0x%llx 0x%llx", endpointID, clusterID.unsignedLongLongValue, attributeID.unsignedLongLongValue];
}

- (void)invokeCommandWithEndpointID:(NSNumber *)endpointID
                          clusterID:(NSNumber *)clusterID
                          commandID:(NSNumber *)commandID
                      commandFields:(NSDictionary<NSString *, id> * _Nullable)commandFields
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues
              expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval
                              queue:(dispatch_queue_t)queue
                         completion:(MTRDeviceResponseHandler)completion
{
    if (commandFields == nil) {
        commandFields = @{
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[],
        };
    }

    [self invokeCommandWithEndpointID:endpointID
                            clusterID:clusterID
                            commandID:commandID
                        commandFields:commandFields
                       expectedValues:expectedValues
                expectedValueInterval:expectedValueInterval
                   timedInvokeTimeout:nil
                                queue:queue
                           completion:completion];
}

- (void)invokeCommandWithEndpointID:(NSNumber *)endpointID
                          clusterID:(NSNumber *)clusterID
                          commandID:(NSNumber *)commandID
                      commandFields:(id)commandFields
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues
              expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval
                 timedInvokeTimeout:(NSNumber * _Nullable)timeout
                              queue:(dispatch_queue_t)queue
                         completion:(MTRDeviceResponseHandler)completion
{
    // We don't have a way to communicate a non-default invoke timeout
    // here for now.
    // TODO: https://github.com/project-chip/connectedhomeip/issues/24563

    [self _invokeCommandWithEndpointID:endpointID
                             clusterID:clusterID
                             commandID:commandID
                         commandFields:commandFields
                        expectedValues:expectedValues
                 expectedValueInterval:expectedValueInterval
                    timedInvokeTimeout:timeout
           serverSideProcessingTimeout:nil
                                 queue:queue
                            completion:completion];
}

- (void)_invokeCommandWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                           commandID:(NSNumber *)commandID
                       commandFields:(id)commandFields
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
    [workItem setReadyHandler:^(MTRDevice * self, NSInteger retryCount, MTRAsyncWorkCompletionBlock workCompletion) {
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
                workDone(nil, [MTRError errorForIMStatusCode:Status::Timeout]);
                return;
            }

            // Recompute the actual timeout left, accounting for time spent
            // in our queuing and retries.
            timedInvokeTimeout = @([cutoffTime timeIntervalSinceDate:now] * 1000);
        }
        MTRBaseDevice * baseDevice = [self newBaseDevice];
        [baseDevice
            _invokeCommandWithEndpointID:endpointID
                               clusterID:clusterID
                               commandID:commandID
                           commandFields:commandFields
                      timedInvokeTimeout:timedInvokeTimeout
             serverSideProcessingTimeout:serverSideProcessingTimeout
                                   queue:self.queue
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
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
    [_asyncWorkQueue enqueueWorkItem:workItem descriptionWithFormat:@"invoke %@ 0x%llx 0x%llx", endpointID, clusterID.unsignedLongLongValue, commandID.unsignedLongLongValue];
}

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
                               completion:(void (^)(id _Nullable response, NSError * _Nullable error))completion
{
    if (![commandPayload respondsToSelector:@selector(_encodeAsDataValue:)]) {
        dispatch_async(queue, ^{
            completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT]);
        });
        return;
    }

    NSError * encodingError;
    auto * commandFields = [commandPayload _encodeAsDataValue:&encodingError];
    if (commandFields == nil) {
        dispatch_async(queue, ^{
            completion(nil, encodingError);
        });
        return;
    }

    auto responseHandler = ^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
        id _Nullable response = nil;
        if (error == nil) {
            if (values.count != 1) {
                error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:nil];
            } else if (responseClass != nil) {
                response = [[responseClass alloc] initWithResponseValue:values[0] error:&error];
            }
        }
        completion(response, error);
    };

    [self _invokeCommandWithEndpointID:endpointID
                             clusterID:clusterID
                             commandID:commandID
                         commandFields:commandFields
                        expectedValues:expectedValues
                 expectedValueInterval:expectedValueInterval
                    timedInvokeTimeout:timeout
           serverSideProcessingTimeout:serverSideProcessingTimeout
                                 queue:queue
                            completion:responseHandler];
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
    auto * baseDevice = [self newBaseDevice];
    [baseDevice downloadLogOfType:type
                          timeout:timeout
                            queue:queue
                       completion:completion];
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
        MTRWeakReference<MTRDevice *> * weakSelf = [MTRWeakReference weakReferenceWithObject:self];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (waitTime * NSEC_PER_SEC)), self.queue, ^{
            MTRDevice * strongSelf = weakSelf.strongObject;
            [strongSelf _performScheduledExpirationCheck];
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

- (BOOL)_attributeDataValue:(NSDictionary *)one isEqualToDataValue:(NSDictionary *)theOther
{
    // Sanity check for nil cases
    if (!one && !theOther) {
        MTR_LOG_ERROR("%@ attribute data-value comparison does not expect comparing two nil dictionaries", self);
        return YES;
    }
    if (!one || !theOther) {
        // Comparing against nil is expected, and should return NO quietly
        return NO;
    }

    // Attribute data-value dictionaries are equal if type and value are equal, and specifically, this should return true if values are both nil
    return [one[MTRTypeKey] isEqual:theOther[MTRTypeKey]] && ((one[MTRValueKey] == theOther[MTRValueKey]) || [one[MTRValueKey] isEqual:theOther[MTRValueKey]]);
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
            [self.deviceController.controllerDataStore clearStoredClusterDataForNodeID:self.nodeID endpointID:path.endpoint clusterID:path.cluster];
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
    [self.deviceController.controllerDataStore removeAttributes:attributes fromCluster:clusterPath forNodeID:self.nodeID];
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
        [self.deviceController.controllerDataStore clearStoredClusterDataForNodeID:self.nodeID endpointID:endpoint];
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
        NSDictionary * attributeDataValue = attributeResponseValue[MTRDataKey];
        NSError * attributeError = attributeResponseValue[MTRErrorKey];
        NSDictionary * previousValue;

        // sanity check either data value or error must exist
        if (!attributeDataValue && !attributeError) {
            MTR_LOG("%@ report %@ no data value or error: %@", self, attributePath, attributeResponseValue);
            continue;
        }

        // Additional signal to help mark events as being received during priming report in the event the device rebooted and we get a subscription resumption priming report without noticing it became unreachable first
        if (_receivingReport && AttributeHasChangesOmittedQuality(attributePath)) {
            _receivingPrimingReport = YES;
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
                // Remove data version from what we cache in memory
                attributeDataValue = [self _dataValueWithoutDataVersion:attributeDataValue];
            }

            previousValue = [self _cachedAttributeValueForPath:attributePath];
            BOOL readCacheValueChanged = ![self _attributeDataValue:attributeDataValue isEqualToDataValue:previousValue];
            // Now that we have grabbed previousValue, update our cache with the attribute value.
            if (readCacheValueChanged) {
                if (dataVersion) {
                    [self _noteDataVersion:dataVersion forClusterPath:clusterPath];
                }

                [self _pruneStoredDataForPath:attributePath missingFrom:attributeDataValue];

                if (!_deviceConfigurationChanged) {
                    _deviceConfigurationChanged = [self _attributeAffectsDeviceConfiguration:attributePath];
                    if (_deviceConfigurationChanged) {
                        MTR_LOG("Device configuration changed due to changes in attribute %@", attributePath);
                    }
                }

                [self _setCachedAttributeValue:attributeDataValue forPath:attributePath fromSubscription:isFromSubscription];
            }

#ifdef DEBUG
            // Unit test only code.
            if (!readCacheValueChanged) {
                id delegate = _weakDelegate.strongObject;
                if (delegate) {
                    if ([delegate respondsToSelector:@selector(unitTestForceAttributeReportsIfMatchingCache:)]) {
                        readCacheValueChanged = [delegate unitTestForceAttributeReportsIfMatchingCache:self];
                    }
                }
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
                    MTR_LOG("%@ report %@ value filtered - expected value still present", self, attributePath);
                } else {
                    MTR_LOG("%@ report %@ value filtered - same as read cache", self, attributePath);
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

    MTR_LOG("%@ report from reported values %@", self, attributePathsToReport);

    return attributesToReport;
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
    MTR_LOG("%@ setPersistedClusterData count: %lu", self, static_cast<unsigned long>(clusterData.count));
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

    auto datastore = _deviceController.controllerDataStore;
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
- (MTRDeviceClusterData *)_getClusterDataForPath:(MTRClusterPath *)path
{
    std::lock_guard lock(_lock);

    return [[self _clusterDataForPath:path] copy];
}

- (BOOL)_clusterHasBeenPersisted:(MTRClusterPath *)path
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

- (void)setExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)values expectedValueInterval:(NSNumber *)expectedValueInterval
{
    [self setExpectedValues:values expectedValueInterval:expectedValueInterval expectedValueID:nil];
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

// Client Metadata Storage

- (NSArray *)supportedClientDataClasses
{
    return @[ [NSData class], [NSString class], [NSNumber class], [NSDictionary class], [NSArray class] ];
}

- (NSArray * _Nullable)clientDataKeys
{
    return [self.temporaryMetaDataCache allKeys];
}

- (id<NSSecureCoding> _Nullable)clientDataForKey:(NSString *)key
{
    if (key == nil)
        return nil;

    return [self.temporaryMetaDataCache objectForKey:[NSString stringWithFormat:@"%@:-1", key]];
}

- (void)setClientDataForKey:(NSString *)key value:(id<NSSecureCoding>)value
{
    // TODO: Check supported data types, and also if they conform to NSSecureCoding, when we store these
    // TODO: Need to add a delegate method, so when this value changes we call back to the client

    if (key == nil || value == nil)
        return;

    if (self.temporaryMetaDataCache == nil) {
        self.temporaryMetaDataCache = [NSMutableDictionary dictionary];
    }

    [self.temporaryMetaDataCache setObject:value forKey:[NSString stringWithFormat:@"%@:-1", key]];
}

- (void)removeClientDataForKey:(NSString *)key
{
    if (key == nil)
        return;

    [self.temporaryMetaDataCache removeObjectForKey:[NSString stringWithFormat:@"%@:-1", key]];
}

- (NSArray * _Nullable)clientDataKeysForEndpointID:(NSNumber *)endpointID
{
    if (endpointID == nil)
        return nil;
    // TODO: When hooked up to storage, enumerate this better

    return [self.temporaryMetaDataCache allKeys];
}

- (id<NSSecureCoding> _Nullable)clientDataForKey:(NSString *)key endpointID:(NSNumber *)endpointID
{
    if (key == nil || endpointID == nil)
        return nil;

    return [self.temporaryMetaDataCache objectForKey:[NSString stringWithFormat:@"%@:%@", key, endpointID]];
}

- (void)setClientDataForKey:(NSString *)key endpointID:(NSNumber *)endpointID value:(id<NSSecureCoding>)value
{
    if (key == nil || value == nil || endpointID == nil)
        return;

    if (self.temporaryMetaDataCache == nil) {
        self.temporaryMetaDataCache = [NSMutableDictionary dictionary];
    }

    [self.temporaryMetaDataCache setObject:value forKey:[NSString stringWithFormat:@"%@:%@", key, endpointID]];
}

- (void)removeClientDataForKey:(NSString *)key endpointID:(NSNumber *)endpointID
{
    if (key == nil || endpointID == nil)
        return;

    [self.temporaryMetaDataCache removeObjectForKey:[NSString stringWithFormat:@"%@:%@", key, endpointID]];
}

#pragma mark Log Help

- (MTRDeviceInformationalAttributes *)_informationalAttributesForCurrentState {
    MTRClusterPath * basicInfoClusterPath = [MTRClusterPath clusterPathWithEndpointID:@(kRootEndpointId) clusterID:@(MTRClusterIDTypeBasicInformationID)];
    MTRDeviceClusterData * basicInfoClusterData = [self _clusterDataForPath:basicInfoClusterPath];
    
    NSNumber * vidObj = basicInfoClusterData.attributes[@(MTRAttributeIDTypeClusterBasicInformationAttributeVendorIDID)][MTRValueKey];
    UInt16 vendorID = vidObj.unsignedShortValue;
    NSNumber * pidObj = basicInfoClusterData.attributes[@(MTRAttributeIDTypeClusterBasicInformationAttributeProductIDID)][MTRValueKey];
    UInt16 productID = pidObj.unsignedShortValue;
    
    BOOL usesThread = [self _deviceUsesThread];
    
    return [[MTRDeviceInformationalAttributes alloc] initWithVendorID:vendorID productID:productID usesThread:usesThread];
}

@end

/* BEGIN DRAGONS: Note methods here cannot be renamed, and are used by private callers, do not rename, remove or modify behavior here */

@implementation MTRDevice (MatterPrivateForInternalDragonsDoNotFeed)

- (BOOL)_deviceHasActiveSubscription
{
    std::lock_guard lock(_lock);

    // TODO: This should always return YES for thread devices
    return HaveSubscriptionEstablishedRightNow(_internalDeviceState);
}

- (void)_deviceMayBeReachable
{
    MTR_LOG("%@ _deviceMayBeReachable called", self);
    // TODO: This should only be allowed for thread devices
    [_deviceController asyncDispatchToMatterQueue:^{
        [self _triggerResubscribeWithReason:@"SPI client indicated the device may now be reachable"
                        nodeLikelyReachable:YES];
    } errorHandler:nil];
}

/* END DRAGONS */

@end

@implementation MTRDevice (Deprecated)

+ (MTRDevice *)deviceWithNodeID:(uint64_t)nodeID deviceController:(MTRDeviceController *)deviceController
{
    return [self deviceWithNodeID:@(nodeID) controller:deviceController];
}

- (void)invokeCommandWithEndpointID:(NSNumber *)endpointID
                          clusterID:(NSNumber *)clusterID
                          commandID:(NSNumber *)commandID
                      commandFields:(id)commandFields
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues
              expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval
                 timedInvokeTimeout:(NSNumber * _Nullable)timeout
                        clientQueue:(dispatch_queue_t)queue
                         completion:(MTRDeviceResponseHandler)completion
{
    [self invokeCommandWithEndpointID:endpointID
                            clusterID:clusterID
                            commandID:commandID
                        commandFields:commandFields
                       expectedValues:expectedValues
                expectedValueInterval:expectedValueInterval
                   timedInvokeTimeout:timeout
                                queue:queue
                           completion:completion];
}

@end

#pragma mark - SubscriptionCallback
namespace {
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
