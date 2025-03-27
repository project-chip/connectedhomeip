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

#import "MTRAttributeValueWaiter_Internal.h"
#import "MTRBaseClusters.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRCluster.h"
#import "MTRClusterConstants.h"
#import "MTRConversion.h"
#import "MTRDefines_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "MTRDeviceDataValidation.h"
#import "MTRDevice_Internal.h"
#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRMetricKeys.h"
#import "MTRMetricsCollector.h"
#import "MTRTimeUtils.h"
#import "MTRUnfairLock.h"
#import "MTRUtilities.h"
#import "zap-generated/MTRCommandPayloads_Internal.h"

#import "lib/core/CHIPError.h"

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

#pragma mark - MTRDevice

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

MTR_DIRECT_MEMBERS
@interface MTRDevice ()
// nil until the first time we need it.  Access guarded by our lock.
@property (nonatomic, readwrite, nullable) NSHashTable<MTRAttributeValueWaiter *> * attributeValueWaiters;
@end

@implementation MTRDevice

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
    // TODO: retain cycle and clean up https://github.com/project-chip/connectedhomeip/issues/34267
    MTR_LOG("MTRDevice dealloc: %p", self);

    [_deviceController deviceDeallocated];

    // Locking because _cancelAllAttributeValueWaiters has os_unfair_lock_assert_owner(&_lock)
    std::lock_guard lock(_lock);
    [self _cancelAllAttributeValueWaiters];
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
    [self _delegateAdded:delegate];
}

- (void)_delegateAdded:(id<MTRDeviceDelegate>)delegate
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

    // Call hook to allow subclasses to act on delegate addition.
    [self _delegateRemoved:delegate];
}

- (void)_delegateRemoved:(id<MTRDeviceDelegate>)delegate
{
    os_unfair_lock_assert_owner(&self->_lock);

    // Nothing to do for now. At the moment this is a hook for subclasses.
}

- (void)invalidate
{
    std::lock_guard lock(_lock);

    [_delegates removeAllObjects];
    [self _cancelAllAttributeValueWaiters];
}

- (BOOL)delegateExists
{
    std::lock_guard lock(_lock);
    return [self _delegateExists];
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

- (NSDictionary<MTRAttributePath *, NSDictionary<NSString *, id> *> *)descriptorClusters
{
    @autoreleasepool {
        // For now, we have a temp array that we should make sure dies as soon
        // as possible.
        //
        // TODO: We should have a version of readAttributePaths that returns a
        // dictionary in the format we want here.
        auto path = [MTRAttributeRequestPath requestPathWithEndpointID:nil
                                                             clusterID:@(MTRClusterIDTypeDescriptorID)
                                                           attributeID:nil];
        auto * data = [self readAttributePaths:@[ path ]];

        auto * retval = [NSMutableDictionary dictionaryWithCapacity:data.count];
        for (NSDictionary * item in data) {
            // We double-check that the things in our dictionaries are the right
            // thing, because XPC has no way to check that for us.
            if (MTR_SAFE_CAST(item[MTRAttributePathKey], MTRAttributePath) && MTR_SAFE_CAST(item[MTRDataKey], NSDictionary)) {
                retval[item[MTRAttributePathKey]] = item[MTRDataKey];
            }
        }

        return retval;
    }
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

- (void)invokeCommands:(NSArray<NSArray<MTRCommandWithRequiredResponse *> *> *)commands
                 queue:(dispatch_queue_t)queue
            completion:(MTRDeviceResponseHandler)completion
{
    MTR_ABSTRACT_METHOD();
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
    });
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

- (nullable NSNumber *)estimatedSubscriptionLatency
{
    MTR_ABSTRACT_METHOD();
    return nil;
}

#pragma mark - Cache management

- (NSArray<NSDictionary<NSString *, id> *> *)getAllAttributesReport
{
    MTR_ABSTRACT_METHOD();
    return nil;
}

- (BOOL)deviceCachePrimed
{
    MTR_ABSTRACT_METHOD();
    return NO;
}

#pragma mark - Suspend/resume management

- (void)controllerSuspended
{
    // Nothing to do for now.
}

- (void)controllerResumed
{
    // Nothing to do for now.
}

#pragma mark - Value comparisons

- (BOOL)_attributeDataValue:(MTRDeviceDataValueDictionary)one isEqualToDataValue:(MTRDeviceDataValueDictionary)theOther
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

// _attributeDataValue:satisfiesExpectedDataValue: checks whether the newly
// received attribute data value satisfies the expectation we have.
//
// For now, a value is considered to satisfy the expectation if it's equal to
// the expected value, though we allow the fields of structs to be in a
// different order than expected: while in theory the spec does require a
// specific ordering for struct fields, in practice we should not force certain
// API consumers to deal with knowing what that ordering is.
//
// Things to consider for future:
//
// 1) Should a value that has _extra_ fields in a struct compared to the expected
//    value be considered as satisfying the expectation?  Arguably, yes.
//
// 2) Should lists actually enforce order (as now), or should they allow
//    reordering entries?
//
// 3) For fabric-scoped lists, should we have a way to check for just "our
//    fabric's" entries?
- (BOOL)_attributeDataValue:(MTRDeviceDataValueDictionary)observed satisfiesValueExpectation:(MTRDeviceDataValueDictionary)expected
{
    // Sanity check for nil cases (which really should not happen!)
    if (!observed && !expected) {
        MTR_LOG_ERROR("%@ observed to expected attribute data-value comparison does not expect comparing two nil dictionaries", self);
        return YES;
    }

    if (!observed || !expected) {
        // Again, not expected here.  But clearly the expectation is not really
        // satisfied, in some sense.
        MTR_LOG_ERROR("@ observed to expected attribute data-value comparison does not expect a nil %s", observed ? "expected" : "observed");
        return NO;
    }

    if (![observed[MTRTypeKey] isEqual:expected[MTRTypeKey]]) {
        // Different types, does not satisfy expectation.
        return NO;
    }

    if ([MTRArrayValueType isEqual:expected[MTRTypeKey]]) {
        // For array-values, check that sizes are same and entries satisfy expectations.
        if (![observed[MTRValueKey] isKindOfClass:NSArray.class] || ![expected[MTRValueKey] isKindOfClass:NSArray.class]) {
            // Malformed data, just claim expectation is not satisfied.
            MTR_LOG_ERROR("%@ at least one of observed and expected value is not an NSArrray: %@, %@", self, observed, expected);
            return NO;
        }

        NSArray<NSDictionary<NSString *, MTRDeviceDataValueDictionary> *> * observedArray = observed[MTRValueKey];
        NSArray<NSDictionary<NSString *, MTRDeviceDataValueDictionary> *> * expectedArray = expected[MTRValueKey];

        if (observedArray.count != expectedArray.count) {
            return NO;
        }

        for (NSUInteger i = 0; i < observedArray.count; ++i) {
            NSDictionary<NSString *, MTRDeviceDataValueDictionary> * observedEntry = observedArray[i];
            NSDictionary<NSString *, MTRDeviceDataValueDictionary> * expectedEntry = expectedArray[i];

            if (![observedEntry isKindOfClass:NSDictionary.class] || ![expectedEntry isKindOfClass:NSDictionary.class]) {
                MTR_LOG_ERROR("%@ expected or observed array-value contains entries that are not NSDictionary: %@, %@", self, observedEntry, expectedEntry);
                return NO;
            }

            MTRDeviceDataValueDictionary observedDataValue = observedEntry[MTRDataKey];
            if (!MTR_SAFE_CAST(observedDataValue, NSDictionary)) {
                MTR_LOG_ERROR("%@ observed data-value is not an NSDictionary: %@", self, observedDataValue);
                return NO;
            }

            MTRDeviceDataValueDictionary expectedDataValue = expectedEntry[MTRDataKey];
            if (!MTR_SAFE_CAST(expectedDataValue, NSDictionary)) {
                MTR_LOG_ERROR("%@ expected data-value is not an NSDictionary: %@", self, expectedDataValue);
                return NO;
            }

            if (![self _attributeDataValue:observedDataValue satisfiesValueExpectation:expectedDataValue]) {
                return NO;
            }
        }

        return YES;
    }

    if (![MTRStructureValueType isEqual:expected[MTRTypeKey]]) {
        // For everything except arrays and structs, expectation is satisfied
        // exactly when the values are equal.
        return [self _attributeDataValue:observed isEqualToDataValue:expected];
    }

    // Now we have two structure-values.  Make sure they have the same number of fields
    // in them.
    if (![observed[MTRValueKey] isKindOfClass:NSArray.class] || ![expected[MTRValueKey] isKindOfClass:NSArray.class]) {
        // Malformed data, just claim not equivalent.
        MTR_LOG_ERROR("%@ at least one of observed and expected value is not an NSArrray: %@, %@", self, observed, expected);
        return NO;
    }

    NSArray<NSDictionary<NSString *, id> *> * observedArray = observed[MTRValueKey];
    NSArray<NSDictionary<NSString *, id> *> * expectedArray = expected[MTRValueKey];

    if (observedArray.count != expectedArray.count) {
        return NO;
    }

    for (NSDictionary<NSString *, id> * expectedField in expectedArray) {
        if (!MTR_SAFE_CAST(expectedField, NSDictionary) || !MTR_SAFE_CAST(expectedField[MTRContextTagKey], NSNumber) || !MTR_SAFE_CAST(expectedField[MTRDataKey], NSDictionary)) {
            MTR_LOG_ERROR("%@ expected structure-value contains invalid field %@", self, expectedField);
            return NO;
        }

        NSNumber * expectedContextTag = expectedField[MTRContextTagKey];

        // Make sure it's present in the other array.  In practice, these are
        // pretty small arrays, so the O(N^2) behavior here is ok.
        BOOL found = NO;
        for (NSDictionary<NSString *, id> * observedField in observedArray) {
            if (!MTR_SAFE_CAST(observedField, NSDictionary) || !MTR_SAFE_CAST(observedField[MTRContextTagKey], NSNumber) || !MTR_SAFE_CAST(observedField[MTRDataKey], NSDictionary)) {
                MTR_LOG_ERROR("%@ observed structure-value contains invalid field %@", self, observedField);
                return NO;
            }

            NSNumber * observedContextTag = observedField[MTRContextTagKey];
            if ([expectedContextTag isEqual:observedContextTag]) {
                found = YES;

                // Compare the data.
                if (![self _attributeDataValue:observedField[MTRDataKey] satisfiesValueExpectation:expectedField[MTRDataKey]]) {
                    return NO;
                }

                // Found a match for the context tag, stop looking.
                break;
            }
        }

        if (!found) {
            // Context tag present in expected but not observed.
            return NO;
        }
    }

    // All entries in the first field array matched entries in the second field
    // array.  Since the lengths are equal, the two arrays must match, as long
    // as all the context tags listed are distinct.  If someone produces invalid
    // TLV with the same context tag set in it multiple times, this method could
    // claim two structure-values are equivalent when the first has two fields
    // with context tag N and the second has a field with context tag N and
    // another field with context tag M.  That should be ok, in practice, but if
    // we discover it's not we will need a better algorithm here.  It's not
    // clear what "equivalent" should mean for such malformed TLV, expecially if
    // the same context tag maps to different values in one of the structs.
    return YES;
}

#pragma mark - Handling of waits for attribute values

- (MTRAttributeValueWaiter *)waitForAttributeValues:(NSDictionary<MTRAttributePath *, MTRDeviceDataValueDictionary> *)values timeout:(NSTimeInterval)timeout queue:(dispatch_queue_t)queue completion:(void (^)(NSError * _Nullable error))completion
{
    // Check whether the values coming in make sense.
    for (MTRAttributePath * path in values) {
        MTRVerifyArgumentOrDie(MTRDataValueDictionaryIsWellFormed(values[path]),
            ([NSString stringWithFormat:@"waitForAttributeValues handed invalid data-value %@ for path %@", path, values[path]]));
    }

    // Check whether we have all these values already.
    NSMutableArray<MTRAttributeRequestPath *> * requestPaths = [NSMutableArray arrayWithCapacity:values.count];
    for (MTRAttributePath * path in values) {
        [requestPaths addObject:[MTRAttributeRequestPath requestPathWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute]];
    }

    NSArray<MTRDeviceResponseValueDictionary> * currentValues = [self readAttributePaths:requestPaths];

    auto * attributeWaiter = [[MTRAttributeValueWaiter alloc] initWithDevice:self values:values queue:queue completion:completion];

    for (MTRDeviceResponseValueDictionary currentValue in currentValues) {
        // Pretend as if this got reported, for purposes of the attribute
        // waiter.
        [attributeWaiter _attributeValue:currentValue[MTRDataKey] reportedForPath:currentValue[MTRAttributePathKey] byDevice:self];
    }

    if (attributeWaiter.allValuesSatisfied) {
        MTR_LOG("%@ waitForAttributeValues no need to wait, values already match: %@", self, values);
        // We haven't added this waiter to self.attributeValueWaiters yet, so
        // no need to remove it before notifying.
        [attributeWaiter _notifyWithError:nil];
        return attributeWaiter;
    }

    // Otherwise, wait for one of our termination conditions.
    {
        std::lock_guard lock(_lock);
        if (!self.attributeValueWaiters) {
            self.attributeValueWaiters = [NSHashTable weakObjectsHashTable];
        }
        [self.attributeValueWaiters addObject:attributeWaiter];
    }

    MTR_LOG("%@ waitForAttributeValues will wait up to %f seconds for %@", self, timeout, values);
    [attributeWaiter _startTimerWithTimeout:timeout];
    return attributeWaiter;
}

- (void)_attributeValue:(MTRDeviceDataValueDictionary)value reportedForPath:(MTRAttributePath *)path
{
    os_unfair_lock_assert_owner(&_lock);

    // Check whether anyone was waiting for this attribute.
    NSMutableArray * satisfiedWaiters;
    for (MTRAttributeValueWaiter * attributeValueWaiter in self.attributeValueWaiters) {
        if ([attributeValueWaiter _attributeValue:value reportedForPath:path byDevice:self] && attributeValueWaiter.allValuesSatisfied) {
            if (!satisfiedWaiters) {
                satisfiedWaiters = [NSMutableArray array];
            }
            [satisfiedWaiters addObject:attributeValueWaiter];
        }
    }

    for (MTRAttributeValueWaiter * attributeValueWaiter in satisfiedWaiters) {
        [self.attributeValueWaiters removeObject:attributeValueWaiter];
        [attributeValueWaiter _notifyWithError:nil];
    }
}

- (void)_forgetAttributeWaiter:(MTRAttributeValueWaiter *)attributeValueWaiter
{
    std::lock_guard lock(_lock);
    [self.attributeValueWaiters removeObject:attributeValueWaiter];
}

- (void)_cancelAllAttributeValueWaiters
{
    os_unfair_lock_assert_owner(&_lock);

    auto * attributeValueWaiters = self.attributeValueWaiters;
    self.attributeValueWaiters = nil;
    for (MTRAttributeValueWaiter * attributeValueWaiter in attributeValueWaiters) {
        [attributeValueWaiter _notifyCancellation];
    }
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
