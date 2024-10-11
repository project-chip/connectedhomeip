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

@interface MTRDevice ()

@property (nonatomic) chip::FabricIndex fabricIndex;

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

// For now, implement an initWithNodeID in case some sub-class outside the
// framework called it (by manually declaring it, even though it's not public
// API).  Ideally we would not have this thing, since its signature does not
// match the initWithNodeID signatures of our subclasses.
- (instancetype)initWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller
{
    return [self initForSubclassesWithNodeID:nodeID controller:controller];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:_systemTimeChangeObserverToken];

    // TODO: retain cycle and clean up https://github.com/project-chip/connectedhomeip/issues/34267
    MTR_LOG("MTRDevice dealloc: %p", self);
}

+ (MTRDevice *)deviceWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller
{
    if (nodeID == nil || controller == nil) {
        // These are not nullable in our API, but clearly someone is not
        // actually turning on the relevant compiler checks (or is doing dynamic
        // dispatch with bad values).  While we promise to not return nil from
        // this method, if the caller is ignoring the nullability API contract,
        // there's not much we can do here.
        MTR_LOG_ERROR("Can't create device with nodeID: %@, controller: %@",
            nodeID, controller);
        return nil;
    }

    return [controller deviceForNodeID:nodeID];
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
    os_unfair_lock_assert_owner(&self->_lock);

    // Nothing to do for now. At the moment this is a hook for subclasses.
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
    MTR_LOG("%@ cluster path %@ cache miss - load from storage success %@", self, clusterPath, MTR_YES_NO(data));
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
    MTR_ABSTRACT_METHOD();
    return nil;
}

- (void)writeAttributeWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                         attributeID:(NSNumber *)attributeID
                               value:(id)value
               expectedValueInterval:(NSNumber *)expectedValueInterval
                   timedWriteTimeout:(NSNumber * _Nullable)timeout
{
    MTR_ABSTRACT_METHOD();
}

- (NSArray<NSDictionary<NSString *, id> *> *)readAttributePaths:(NSArray<MTRAttributeRequestPath *> *)attributePaths
{
    MTR_ABSTRACT_METHOD();
    return [NSArray array];
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

    if (![commandFields isKindOfClass:NSDictionary.class]) {
        MTR_LOG_ERROR("%@ invokeCommandWithEndpointID passed a commandFields (%@) that is not a data-value NSDictionary object",
            self, commandFields);
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT]);
        return;
    }

    MTRDeviceDataValueDictionary fieldsDataValue = commandFields;
    if (![MTRStructureValueType isEqual:fieldsDataValue[MTRTypeKey]]) {
        MTR_LOG_ERROR("%@ invokeCommandWithEndpointID passed a commandFields (%@) that is not a structure-typed data-value object",
            self, commandFields);
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT]);
        return;
    }

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
                       commandFields:(MTRDeviceDataValueDictionary)commandFields
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues
               expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval
                  timedInvokeTimeout:(NSNumber * _Nullable)timeout
         serverSideProcessingTimeout:(NSNumber * _Nullable)serverSideProcessingTimeout
                               queue:(dispatch_queue_t)queue
                          completion:(MTRDeviceResponseHandler)completion
{
    MTR_ABSTRACT_METHOD();
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
    MTR_ABSTRACT_METHOD();
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
    });
}

- (void)openCommissioningWindowWithDiscriminator:(NSNumber *)discriminator
                                        duration:(NSNumber *)duration
                                           queue:(dispatch_queue_t)queue
                                      completion:(MTRDeviceOpenCommissioningWindowHandler)completion
{
    MTR_ABSTRACT_METHOD();
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
    });
}

- (void)downloadLogOfType:(MTRDiagnosticLogType)type
                  timeout:(NSTimeInterval)timeout
                    queue:(dispatch_queue_t)queue
               completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
{
    MTR_ABSTRACT_METHOD();
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
    });
}

#pragma mark - Cache management

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

- (NSArray<NSDictionary<NSString *, id> *> *)getAllAttributesReport
{
    MTR_ABSTRACT_METHOD();
    return nil;
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

- (void)controllerSuspended
{
    // Nothing to do for now.
}

- (void)controllerResumed
{
    // Nothing to do for now.
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
