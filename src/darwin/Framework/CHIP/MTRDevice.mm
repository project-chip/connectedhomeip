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

#import <Matter/Matter.h>
#import <os/lock.h>

#import "MTRAsyncWorkQueue.h"
#import "MTRAttributeSpecifiedCheck.h"
#import "MTRBaseClusters.h"
#import "MTRBaseDevice_Internal.h"
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
#import "MTRUtilities.h"
#import "zap-generated/MTRCommandPayloads_Internal.h"

#import "lib/core/CHIPError.h"

#import <platform/LockTracker.h>

typedef void (^MTRDeviceAttributeReportHandler)(NSArray * _Nonnull);

NSString * const MTRPreviousDataKey = @"previousData";
NSString * const MTRDataVersionKey = @"dataVersion";

#define kSecondsToWaitBeforeMarkingUnreachableAfterSettingUpSubscription 10

// Disabling pending crashes
#define ENABLE_CONNECTIVITY_MONITORING 0

@implementation MTRDeviceDelegateInfo
- (instancetype)initWithDelegate:(id<MTRDeviceDelegate>)delegate queue:(dispatch_queue_t)queue interestedPathsForAttributes:(NSArray * _Nullable)interestedPathsForAttributes interestedPathsForEvents:(NSArray * _Nullable)interestedPathsForEvents
{
    if (self = [super init]) {
        _delegate = delegate;
        _delegatePointerValue = (__bridge void *) delegate;
        _queue = queue;
        _interestedPathsForAttributes = [interestedPathsForAttributes copy];
        _interestedPathsForEvents = [interestedPathsForEvents copy];
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRDeviceDelegateInfo: %p delegate value %p interested attribute paths count %lu event paths count %lu>", self, _delegatePointerValue, static_cast<unsigned long>(_interestedPathsForAttributes.count), static_cast<unsigned long>(_interestedPathsForEvents.count)];
}

- (BOOL)callDelegateWithBlock:(void (^)(id<MTRDeviceDelegate>))block
{
    id<MTRDeviceDelegate> strongDelegate = _delegate;
    VerifyOrReturnValue(strongDelegate, NO);
    dispatch_async(_queue, ^{
        block(strongDelegate);
    });
    return YES;
}

#ifdef DEBUG
- (BOOL)callDelegateSynchronouslyWithBlock:(void (^)(id<MTRDeviceDelegate>))block
{
    id<MTRDeviceDelegate> strongDelegate = _delegate;
    VerifyOrReturnValue(strongDelegate, NO);

    block(strongDelegate);

    return YES;
}
#endif
@end

/* BEGIN DRAGONS: Note methods here cannot be renamed, and are used by private callers, do not rename, remove or modify behavior here */

@interface NSObject (MatterPrivateForInternalDragonsDoNotFeed)
- (void)_deviceInternalStateChanged:(MTRDevice *)device;
@end

/* END DRAGONS */

using namespace chip;
using namespace chip::app;
using namespace chip::Protocols::InteractionModel;
using namespace chip::Tracing::DarwinFramework;

#pragma mark - MTRDevice

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
    return MTREqualObjects(_dataVersion, otherClusterData.dataVersion)
        && MTREqualObjects(_attributes, otherClusterData.attributes);
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

// Weight of new data in determining subscription latencies.  To avoid random
// outliers causing too much noise in the value, treat an existing value (if
// any) as having 2/3 weight and the new value as having 1/3 weight.  These
// weights are subject to change, if it's determined that different ones give
// better behavior.
#define MTRDEVICE_SUBSCRIPTION_LATENCY_NEW_VALUE_WEIGHT (1.0 / 3.0)

@interface MTRDevice ()
// protects against concurrent time updates by guarding timeUpdateScheduled flag which manages time updates scheduling,
// and protects device calls to setUTCTime and setDSTOffset.  This can't just be replaced with "lock", because the time
// update code calls public APIs like readAttributeWithEndpointID:.. (which attempt to take "lock") while holding
// whatever lock protects the time sync bits.
@property (nonatomic, readonly) os_unfair_lock timeSyncLock;

@property (nonatomic) chip::FabricIndex fabricIndex;

// TODO: instead of all the BOOL properties that are some facet of the state, move to internal state machine that has (at least):
//   Actively receiving report
//   Actively receiving priming report

@property (nonatomic) MTRInternalDeviceState internalDeviceState;

#define MTRDEVICE_SUBSCRIPTION_ATTEMPT_MIN_WAIT_SECONDS (1)
#define MTRDEVICE_SUBSCRIPTION_ATTEMPT_MAX_WAIT_SECONDS (3600)
@property (nonatomic) uint32_t lastSubscriptionAttemptWait;

// Expected value cache is attributePath => NSArray of [NSDate of expiration time, NSDictionary of value, expected value ID]
//   - See MTRDeviceExpectedValueFieldIndex for the definitions of indices into this array.
// See MTRDeviceResponseHandler definition for value dictionary details.
@property (nonatomic) NSMutableDictionary<MTRAttributePath *, NSArray *> * expectedValueCache;

// This is a monotonically increasing value used when adding entries to expectedValueCache
// Currently used/updated only in _getAttributesToReportWithNewExpectedValues:expirationTime:expectedValueID:
@property (nonatomic) uint64_t expectedValueNextID;

@property (nonatomic) BOOL expirationCheckScheduled;

@property (nonatomic) BOOL timeUpdateScheduled;

@property (nonatomic) NSMutableDictionary * temporaryMetaDataCache;

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
- (BOOL)unitTestSuppressTimeBasedReachabilityChanges:(MTRDevice *)device;
@end
#endif

@implementation MTRDevice {
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
    // Most recent entry in _mostRecentReportTimes, if any.
    NSDate * _Nullable _mostRecentReportTimeForDescription;
}

- (instancetype)initForSubclassesWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller
{
    if (self = [super init]) {
        _lock = OS_UNFAIR_LOCK_INIT;
        _delegates = [NSMutableSet set];
        _deviceController = controller;
        _nodeID = nodeID;
        _state = MTRDeviceStateUnknown;
    }

    return self;
}

- (instancetype)initWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller
{
    if (self = [super init]) {
        _lock = OS_UNFAIR_LOCK_INIT;
        _timeSyncLock = OS_UNFAIR_LOCK_INIT;
        _descriptionLock = OS_UNFAIR_LOCK_INIT;
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

        // If there is a data store, make sure we have an observer to monitor system clock changes, so
        // NSDate-based write coalescing could be reset and not get into a bad state.
        if (_persistedClusterData) {
            mtr_weakify(self);
            _systemTimeChangeObserverToken = [[NSNotificationCenter defaultCenter] addObserverForName:NSSystemClockDidChangeNotification object:nil queue:nil usingBlock:^(NSNotification * _Nonnull notification) {
                mtr_strongify(self);
                std::lock_guard lock(self->_lock);
                [self _resetStorageBehaviorState];
            }];
        }

        _delegates = [NSMutableSet set];

        MTR_LOG_DEBUG("%@ init with hex nodeID 0x%016llX", self, _nodeID.unsignedLongLongValue);
    }
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:_systemTimeChangeObserverToken];

    // TODO: retain cycle and clean up https://github.com/project-chip/connectedhomeip/issues/34267
    MTR_LOG("MTRDevice dealloc: %p", self);
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
    mtr_weakify(self);
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (nextUpdateInSeconds * NSEC_PER_SEC)), self.queue, ^{
        MTR_LOG_DEBUG("%@ Timer expired, start Device Time Update", self);
        mtr_strongify(self);
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

#pragma mark Delegate handling

- (void)setDelegate:(id<MTRDeviceDelegate>)delegate queue:(dispatch_queue_t)queue
{
    MTR_LOG("%@ setDelegate %@", self, delegate);
    [self _addDelegate:delegate queue:queue interestedPathsForAttributes:nil interestedPathsForEvents:nil];
}

- (void)addDelegate:(id<MTRDeviceDelegate>)delegate queue:(dispatch_queue_t)queue
{
    MTR_LOG("%@ addDelegate %@", self, delegate);
    [self _addDelegate:delegate queue:queue interestedPathsForAttributes:nil interestedPathsForEvents:nil];
}

- (void)addDelegate:(id<MTRDeviceDelegate>)delegate queue:(dispatch_queue_t)queue interestedPathsForAttributes:(NSArray * _Nullable)interestedPathsForAttributes interestedPathsForEvents:(NSArray * _Nullable)interestedPathsForEvents
{
    MTR_LOG("%@ addDelegate %@ with interested attribute paths %@ event paths %@", self, delegate, interestedPathsForAttributes, interestedPathsForEvents);
    [self _addDelegate:delegate queue:queue interestedPathsForAttributes:interestedPathsForAttributes interestedPathsForEvents:interestedPathsForEvents];
}

- (void)_addDelegate:(id<MTRDeviceDelegate>)delegate queue:(dispatch_queue_t)queue interestedPathsForAttributes:(NSArray * _Nullable)interestedPathsForAttributes interestedPathsForEvents:(NSArray * _Nullable)interestedPathsForEvents
{
    std::lock_guard lock(_lock);

    // Replace delegate info with the same delegate object, and opportunistically remove defunct delegate references
    NSMutableSet<MTRDeviceDelegateInfo *> * delegatesToRemove = [NSMutableSet set];
    for (MTRDeviceDelegateInfo * delegateInfo in _delegates) {
        id<MTRDeviceDelegate> strongDelegate = delegateInfo.delegate;
        if (!strongDelegate) {
            [delegatesToRemove addObject:delegateInfo];
            MTR_LOG("%@ removing delegate info for nil delegate %p", self, delegateInfo.delegatePointerValue);
        } else if (strongDelegate == delegate) {
            [delegatesToRemove addObject:delegateInfo];
            MTR_LOG("%@ replacing delegate info for %p", self, delegate);
        }
    }
    if (delegatesToRemove.count) {
        NSUInteger oldDelegatesCount = _delegates.count;
        [_delegates minusSet:delegatesToRemove];
        MTR_LOG("%@ addDelegate: removed %lu", self, static_cast<unsigned long>(_delegates.count - oldDelegatesCount));
    }

    MTRDeviceDelegateInfo * newDelegateInfo = [[MTRDeviceDelegateInfo alloc] initWithDelegate:delegate queue:queue interestedPathsForAttributes:interestedPathsForAttributes interestedPathsForEvents:interestedPathsForEvents];
    [_delegates addObject:newDelegateInfo];
    MTR_LOG("%@ added delegate info %@", self, newDelegateInfo);

    // Call hook to allow subclasses to act on delegate addition.
    [self _delegateAdded];
}

- (void)_delegateAdded
{
    // Nothing to do; this is a hook for subclasses.  If that ever changes for
    // some reason, subclasses need to start calling this hook on their super.
}

- (void)removeDelegate:(id<MTRDeviceDelegate>)delegate
{
    MTR_LOG("%@ removeDelegate %@", self, delegate);

    std::lock_guard lock(_lock);

    NSMutableSet<MTRDeviceDelegateInfo *> * delegatesToRemove = [NSMutableSet set];
    [self _iterateDelegatesWithBlock:^(MTRDeviceDelegateInfo * delegateInfo) {
        id<MTRDeviceDelegate> strongDelegate = delegateInfo.delegate;
        if (!strongDelegate) {
            [delegatesToRemove addObject:delegateInfo];
            MTR_LOG("%@ removing delegate info for nil delegate %p", self, delegateInfo.delegatePointerValue);
        } else if (strongDelegate == delegate) {
            [delegatesToRemove addObject:delegateInfo];
            MTR_LOG("%@ removing delegate info %@ for %p", self, delegateInfo, delegate);
        }
    }];
    if (delegatesToRemove.count) {
        NSUInteger oldDelegatesCount = _delegates.count;
        [_delegates minusSet:delegatesToRemove];
        MTR_LOG("%@ removeDelegate: removed %lu", self, static_cast<unsigned long>(_delegates.count - oldDelegatesCount));
    }
}

- (void)invalidate
{
    std::lock_guard lock(_lock);

    [_delegates removeAllObjects];
}

- (void)nodeMayBeAdvertisingOperational
{
    assertChipStackLockedByCurrentThread();

    MTR_LOG("%@ saw new operational advertisement", self);
}

- (BOOL)_delegateExists
{
    os_unfair_lock_assert_owner(&self->_lock);
    return [self _iterateDelegatesWithBlock:nil];
}

- (BOOL)_iterateDelegatesWithBlock:(void(NS_NOESCAPE ^ _Nullable)(MTRDeviceDelegateInfo * delegateInfo))block
{
    os_unfair_lock_assert_owner(&self->_lock);

    if (!_delegates.count) {
        MTR_LOG_DEBUG("%@ no delegates to iterate", self);
        return NO;
    }

    // Opportunistically remove defunct delegate references on every iteration
    NSMutableSet * delegatesToRemove = nil;
    for (MTRDeviceDelegateInfo * delegateInfo in _delegates) {
        id<MTRDeviceDelegate> strongDelegate = delegateInfo.delegate;
        if (strongDelegate) {
            if (block) {
                @autoreleasepool {
                    block(delegateInfo);
                }
            }
            (void) strongDelegate; // ensure it stays alive
        } else {
            if (!delegatesToRemove) {
                delegatesToRemove = [NSMutableSet set];
            }
            [delegatesToRemove addObject:delegateInfo];
        }
    }

    if (delegatesToRemove.count) {
        [_delegates minusSet:delegatesToRemove];
        MTR_LOG("%@ _iterateDelegatesWithBlock: removed %lu remaining %lu", self, static_cast<unsigned long>(delegatesToRemove.count), (unsigned long) static_cast<unsigned long>(_delegates.count));
    }

    return (_delegates.count > 0);
}

- (BOOL)_callDelegatesWithBlock:(void (^)(id<MTRDeviceDelegate> delegate))block
{
    os_unfair_lock_assert_owner(&self->_lock);

    __block NSUInteger delegatesCalled = 0;
    [self _iterateDelegatesWithBlock:^(MTRDeviceDelegateInfo * delegateInfo) {
        if ([delegateInfo callDelegateWithBlock:block]) {
            delegatesCalled++;
        }
    }];

    return (delegatesCalled > 0);
}

- (BOOL)_lockAndCallDelegatesWithBlock:(void (^)(id<MTRDeviceDelegate> delegate))block
{
    std::lock_guard lock(self->_lock);
    return [self _callDelegatesWithBlock:block];
}

#ifdef DEBUG
// Only used for unit test purposes - normal delegate should not expect or handle being called back synchronously
- (void)_callFirstDelegateSynchronouslyWithBlock:(void (^)(id<MTRDeviceDelegate> delegate))block
{
    os_unfair_lock_assert_owner(&self->_lock);

    for (MTRDeviceDelegateInfo * delegateInfo in _delegates) {
        if ([delegateInfo callDelegateSynchronouslyWithBlock:block]) {
            MTR_LOG("%@ _callFirstDelegateSynchronouslyWithBlock: successfully called %@", self, delegateInfo);
            return;
        }
    }
}
#endif

#ifdef DEBUG
- (MTRInternalDeviceState)_getInternalState
{
    std::lock_guard lock(self->_lock);
    return _internalDeviceState;
}
#endif

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

    // Sanity check
    if (![self _dataStoreExists]) {
        MTR_LOG_ERROR("%@ storage behavior: no data store in _persistClusterData!", self);
        return;
    }

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

    std::lock_guard lock(_descriptionLock);
    _mostRecentReportTimeForDescription = [mostRecentReportTimes lastObject];
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

#ifdef DEBUG
- (void)unitTestInjectEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport
{
    NSAssert(NO, @"Unit test injection of reports needs to be handled by subclasses");
}

- (void)unitTestInjectAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport fromSubscription:(BOOL)isFromSubscription
{
    NSAssert(NO, @"Unit test injection of reports needs to be handled by subclasses");
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
        MTRDeviceClusterData * data = [_deviceController.controllerDataStore getStoredClusterDataForNodeID:_nodeID endpointID:clusterPath.endpoint clusterID:clusterPath.cluster];
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

    NSAssert(_deviceController.controllerDataStore != nil,
        @"How can _persistedClusters have an entry if we have no persistence?");
    NSAssert(_persistedClusterData != nil,
        @"How can _persistedClusterData not exist if we have persisted clusters?");

    // Page in the stored value for the data.
    MTRDeviceClusterData * data = [_deviceController.controllerDataStore getStoredClusterDataForNodeID:_nodeID endpointID:clusterPath.endpoint clusterID:clusterPath.cluster];
    MTR_LOG("%@ cluster path %@ cache miss - load from storage success %@", self, clusterPath, YES_NO(data));
    if (data != nil) {
        [_persistedClusterData setObject:data forKey:clusterPath];
    } else {
        // If clusterPath is in _persistedClusters and the data store returns nil for it, then the in-memory cache is now not dependable, and subscription should be reset and reestablished to reload cache from device

        // First make sure _persistedClusters is consistent with storage, so repeated calls don't immediately re-trigger this
        [self _reconcilePersistedClustersWithStorage];
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

#ifdef DEBUG
- (void)unitTestResetSubscription
{
}
#endif

#ifdef DEBUG
- (NSUInteger)unitTestAttributesReportedSinceLastCheck
{
    return 0;
}

- (NSUInteger)unitTestNonnullDelegateCount
{
    std::lock_guard lock(self->_lock);

    NSUInteger nonnullDelegateCount = 0;
    for (MTRDeviceDelegateInfo * delegateInfo in _delegates) {
        if (delegateInfo.delegate) {
            nonnullDelegateCount++;
        }
    }

    return nonnullDelegateCount;
}
#endif

#pragma mark Device Interactions

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWithEndpointID:(NSNumber *)endpointID
                                                              clusterID:(NSNumber *)clusterID
                                                            attributeID:(NSNumber *)attributeID
                                                                 params:(MTRReadParams * _Nullable)params
{
#define MTRDeviceErrorStr "MTRDevice readAttributeWithEndpointID:clusterID:attributeID:params: must be handled by subclasses"
    MTR_LOG_ERROR(MTRDeviceErrorStr);
#ifdef DEBUG
    NSAssert(NO, @MTRDeviceErrorStr);
#endif // DEBUG
#undef MTRDeviceErrorStr
    return nil;
}

- (void)writeAttributeWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                         attributeID:(NSNumber *)attributeID
                               value:(id)value
               expectedValueInterval:(NSNumber *)expectedValueInterval
                   timedWriteTimeout:(NSNumber * _Nullable)timeout
{
#define MTRDeviceErrorStr "MTRDevice writeAttributeWithEndpointID:clusterID:attributeID:value:expectedValueInterval:timedWriteTimeout: must be handled by subclasses"
    MTR_LOG_ERROR(MTRDeviceErrorStr);
#ifdef DEBUG
    NSAssert(NO, @MTRDeviceErrorStr);
#endif // DEBUG
#undef MTRDeviceErrorStr
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
        mtr_weakify(self);
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (waitTime * NSEC_PER_SEC)), self.queue, ^{
            mtr_strongify(self);
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

    [self _updateAttributeDependentDescriptionData];

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

#pragma mark - Description handling

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

    auto * partsListPath = [MTRAttributePath attributePathWithEndpointID:@(kRootEndpointId)
                                                               clusterID:@(MTRClusterIDTypeDescriptorID)
                                                             attributeID:@(MTRAttributeIDTypeClusterDescriptorAttributePartsListID)];
    auto * partsList = [self _cachedAttributeValueForPath:partsListPath];
    NSMutableArray<NSNumber *> * endpointsOnDevice = [self arrayOfNumbersFromAttributeValue:partsList];
    if (!endpointsOnDevice) {
        endpointsOnDevice = [[NSMutableArray<NSNumber *> alloc] init];
    }
    // Add Root node!
    [endpointsOnDevice addObject:@(0)];
    return endpointsOnDevice;
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

@end

/* BEGIN DRAGONS: Note methods here cannot be renamed, and are used by private callers, do not rename, remove or modify behavior here */

@implementation MTRDevice (MatterPrivateForInternalDragonsDoNotFeed)

- (BOOL)_deviceHasActiveSubscription
{
    return NO;
}

- (void)_deviceMayBeReachable
{
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
