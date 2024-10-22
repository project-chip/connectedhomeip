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

#import "MTRBaseClusters.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRCluster.h"
#import "MTRClusterConstants.h"
#import "MTRConversion.h"
#import "MTRDefines_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "MTRDeviceDataValueDictionary.h"
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
