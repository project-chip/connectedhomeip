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

#import <os/lock.h>

#import "MTRAsyncCallbackWorkQueue.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRBaseSubscriptionCallback.h"
#import "MTRCluster.h"
#import "MTRDeviceController_Internal.h"
#import "MTRDevice_Internal.h"
#import "MTRError_Internal.h"
#import "MTREventTLVValueDecoder_Internal.h"
#import "MTRLogging_Internal.h"

#include "lib/core/CHIPError.h"
#include "lib/core/DataModelTypes.h"
#include <app/ConcreteAttributePath.h>

#include <app/AttributePathParams.h>
#include <app/BufferedReadCallback.h>
#include <app/ClusterStateCache.h>
#include <app/InteractionModelEngine.h>
#include <platform/PlatformManager.h>

typedef void (^MTRDeviceAttributeReportHandler)(NSArray * _Nonnull);

// Consider moving utility classes to their own file
#pragma mark - Utility Classes
@interface MTRPair<FirstObjectType, SecondObjectType> : NSObject
+ (instancetype)pairWithFirst:(FirstObjectType)first second:(SecondObjectType)second;
@property (nonatomic, readonly) FirstObjectType first;
@property (nonatomic, readonly) SecondObjectType second;
@end

@implementation MTRPair
- (instancetype)initWithFirst:(id)first second:(id)second
{
    if (self = [super init]) {
        _first = first;
        _second = second;
    }
    return self;
}
+ (instancetype)pairWithFirst:(id)first second:(id)second
{
    return [[MTRPair alloc] initWithFirst:first second:second];
}
@end

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
    return [[MTRWeakReference alloc] initWithObject:object];
}
- (id)strongObject
{
    return _object;
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

#pragma mark - SubscriptionCallback class declaration
using namespace chip;
using namespace chip::app;
using namespace chip::Protocols::InteractionModel;

namespace {

class SubscriptionCallback final : public MTRBaseSubscriptionCallback {
public:
    SubscriptionCallback(DataReportCallback attributeReportCallback, DataReportCallback eventReportCallback,
        ErrorCallback errorCallback, MTRDeviceResubscriptionScheduledHandler resubscriptionCallback,
        SubscriptionEstablishedHandler subscriptionEstablishedHandler, OnDoneHandler onDoneHandler)
        : MTRBaseSubscriptionCallback(attributeReportCallback, eventReportCallback, errorCallback, resubscriptionCallback,
            subscriptionEstablishedHandler, onDoneHandler)
    {
    }

private:
    void OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus) override;

    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override;
};

} // anonymous namespace

#pragma mark - MTRDevice
@interface MTRDevice ()
@property (nonatomic, readonly) os_unfair_lock lock; // protects the caches and device state
@property (nonatomic) chip::FabricIndex fabricIndex;
@property (nonatomic) MTRWeakReference<id<MTRDeviceDelegate>> * weakDelegate;
@property (nonatomic) dispatch_queue_t delegateQueue;
@property (nonatomic) NSArray<NSDictionary<NSString *, id> *> * unreportedEvents;

@property (nonatomic) BOOL subscriptionActive;

#define MTRDEVICE_SUBSCRIPTION_ATTEMPT_MIN_WAIT_SECONDS (1)
#define MTRDEVICE_SUBSCRIPTION_ATTEMPT_MAX_WAIT_SECONDS (3600)
@property (nonatomic) uint32_t lastSubscriptionAttemptWait;
@property (nonatomic) BOOL reattemptingSubscription;

// Read cache is attributePath => NSDictionary of value.
// See MTRDeviceResponseHandler definition for value dictionary details.
@property (nonatomic) NSMutableDictionary<MTRAttributePath *, NSDictionary *> * readCache;

// Expected value cache is attributePath => MTRPair of [NSDate of expiration time, NSDictionary of value]
// See MTRDeviceResponseHandler definition for value dictionary details.
@property (nonatomic) NSMutableDictionary<MTRAttributePath *, MTRPair<NSDate *, NSDictionary *> *> * expectedValueCache;

@property (nonatomic) BOOL expirationCheckScheduled;
@end

@implementation MTRDevice

- (instancetype)initWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller
{
    if (self = [super init]) {
        _lock = OS_UNFAIR_LOCK_INIT;
        _nodeID = [nodeID copy];
        _fabricIndex = controller.fabricIndex;
        _deviceController = controller;
        _queue = dispatch_queue_create("com.apple.matter.framework.device.workqueue", DISPATCH_QUEUE_SERIAL);
        _readCache = [NSMutableDictionary dictionary];
        _expectedValueCache = [NSMutableDictionary dictionary];
        _asyncCallbackWorkQueue = [[MTRAsyncCallbackWorkQueue alloc] initWithContext:self queue:_queue];
        _state = MTRDeviceStateUnknown;
        MTR_LOG_INFO("%@ init with hex nodeID 0x%016llX", self, _nodeID.unsignedLongLongValue);
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRDevice: %p>[fabric: %u, nodeID: %@]", self, _fabricIndex, _nodeID];
}

+ (instancetype)deviceWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller
{
    return [controller deviceForNodeID:nodeID];
}

#pragma mark Subscription and delegate handling

// subscription intervals are in seconds
#define MTR_DEVICE_SUBSCRIPTION_MAX_INTERVAL_MIN (2)
#define MTR_DEVICE_SUBSCRIPTION_MAX_INTERVAL_MAX (60)

- (void)setDelegate:(id<MTRDeviceDelegate>)delegate queue:(dispatch_queue_t)queue
{
    MTR_LOG_INFO("%@ setDelegate %@", self, delegate);
    os_unfair_lock_lock(&self->_lock);

    _weakDelegate = [MTRWeakReference weakReferenceWithObject:delegate];
    _delegateQueue = queue;
    [self _setupSubscription];

    os_unfair_lock_unlock(&self->_lock);
}

- (void)invalidate
{
    MTR_LOG_INFO("%@ invalidate", self);
    os_unfair_lock_lock(&self->_lock);

    _weakDelegate = nil;

    os_unfair_lock_unlock(&self->_lock);
}

- (void)_handleSubscriptionEstablished
{
    os_unfair_lock_lock(&self->_lock);

    // reset subscription attempt wait time when subscription succeeds
    _lastSubscriptionAttemptWait = 0;

    _state = MTRDeviceStateReachable;
    id<MTRDeviceDelegate> delegate = _weakDelegate.strongObject;
    if (delegate) {
        dispatch_async(_delegateQueue, ^{
            [delegate device:self stateChanged:MTRDeviceStateReachable];
        });
    }

    os_unfair_lock_unlock(&self->_lock);
}

- (void)_handleSubscriptionError:(NSError *)error
{
    os_unfair_lock_lock(&self->_lock);

    _subscriptionActive = NO;
    _unreportedEvents = nil;

    id<MTRDeviceDelegate> delegate = _weakDelegate.strongObject;
    if (delegate) {
        dispatch_async(_delegateQueue, ^{
            [delegate device:self stateChanged:MTRDeviceStateUnreachable];
        });
    }

    os_unfair_lock_unlock(&self->_lock);
}

- (void)_handleResubscriptionNeeded
{
    os_unfair_lock_lock(&self->_lock);

    _state = MTRDeviceStateUnknown;

    id<MTRDeviceDelegate> delegate = _weakDelegate.strongObject;
    if (delegate) {
        dispatch_async(_delegateQueue, ^{
            [delegate device:self stateChanged:MTRDeviceStateUnknown];
        });
    }

    os_unfair_lock_unlock(&self->_lock);
}

- (void)_handleSubscriptionReset
{
    os_unfair_lock_lock(&self->_lock);
    // if there is no delegate then also do not retry
    id<MTRDeviceDelegate> delegate = _weakDelegate.strongObject;
    if (!delegate) {
        MTR_LOG_INFO("%@ no delegate - do not reattempt subscription", self);
        os_unfair_lock_unlock(&self->_lock);
        return;
    }

    // don't schedule multiple retries
    if (self.reattemptingSubscription) {
        MTR_LOG_INFO("%@ already reattempting subscription", self);
        os_unfair_lock_unlock(&self->_lock);
        return;
    }

    self.reattemptingSubscription = YES;

    if (_lastSubscriptionAttemptWait < MTRDEVICE_SUBSCRIPTION_ATTEMPT_MIN_WAIT_SECONDS) {
        _lastSubscriptionAttemptWait = MTRDEVICE_SUBSCRIPTION_ATTEMPT_MIN_WAIT_SECONDS;
    } else {
        _lastSubscriptionAttemptWait *= 2;
        if (_lastSubscriptionAttemptWait > MTRDEVICE_SUBSCRIPTION_ATTEMPT_MAX_WAIT_SECONDS) {
            _lastSubscriptionAttemptWait = MTRDEVICE_SUBSCRIPTION_ATTEMPT_MAX_WAIT_SECONDS;
        }
    }

    MTR_LOG_INFO("%@ scheduling to reattempt subscription in %u seconds", self, _lastSubscriptionAttemptWait);
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(_lastSubscriptionAttemptWait * NSEC_PER_SEC)), self.queue, ^{
        os_unfair_lock_lock(&self->_lock);
        MTR_LOG_INFO("%@ reattempting subscription", self);
        self.reattemptingSubscription = NO;
        [self _setupSubscription];
        os_unfair_lock_unlock(&self->_lock);
    });

    os_unfair_lock_unlock(&self->_lock);
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

- (void)_handleAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport
{
    os_unfair_lock_lock(&self->_lock);

    // _getAttributesToReportWithReportedValues will log attribute paths reported
    [self _reportAttributes:[self _getAttributesToReportWithReportedValues:attributeReport]];

    os_unfair_lock_unlock(&self->_lock);
}

- (void)_handleEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport
{
    os_unfair_lock_lock(&self->_lock);

    // first combine with previous unreported events, if they exist
    if (_unreportedEvents) {
        eventReport = [_unreportedEvents arrayByAddingObjectsFromArray:eventReport];
        _unreportedEvents = nil;
    }

    id<MTRDeviceDelegate> delegate = _weakDelegate.strongObject;
    if (delegate) {
        dispatch_async(_delegateQueue, ^{
            [delegate device:self receivedEventReport:eventReport];
        });
    } else {
        // save unreported events
        _unreportedEvents = eventReport;
    }

    os_unfair_lock_unlock(&self->_lock);
}

// assume lock is held
- (void)_setupSubscription
{
    os_unfair_lock_assert_owner(&self->_lock);

    // for now just subscribe once
    if (_subscriptionActive) {
        return;
    }

    _subscriptionActive = YES;

    [_deviceController getSessionForNode:_nodeID.unsignedLongLongValue
                              completion:^(chip::Messaging::ExchangeManager * _Nullable exchangeManager,
                                  const chip::Optional<chip::SessionHandle> & session, NSError * _Nullable error) {
                                  if (error != nil) {
                                      MTR_LOG_INFO("%@ getSessionForNode error %@", self, error);
                                      dispatch_async(self.queue, ^{
                                          [self _handleSubscriptionError:error];
                                          [self _handleSubscriptionReset];
                                      });
                                      return;
                                  }

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
                                  if (idleSleepInterval.count() < MTR_DEVICE_SUBSCRIPTION_MAX_INTERVAL_MIN) {
                                      idleSleepInterval = System::Clock::Seconds32(MTR_DEVICE_SUBSCRIPTION_MAX_INTERVAL_MIN);
                                  }
                                  if (idleSleepInterval.count() > MTR_DEVICE_SUBSCRIPTION_MAX_INTERVAL_MAX) {
                                      idleSleepInterval = System::Clock::Seconds32(MTR_DEVICE_SUBSCRIPTION_MAX_INTERVAL_MAX);
                                  }
                                  readParams.mMaxIntervalCeilingSeconds = static_cast<uint16_t>(idleSleepInterval.count());

                                  readParams.mpAttributePathParamsList = attributePath.get();
                                  readParams.mAttributePathParamsListSize = 1;
                                  readParams.mpEventPathParamsList = eventPath.get();
                                  readParams.mEventPathParamsListSize = 1;
                                  readParams.mKeepSubscriptions = true;
                                  readParams.mIsFabricFiltered = false;
                                  attributePath.release();
                                  eventPath.release();

                                  auto callback = std::make_unique<SubscriptionCallback>(
                                      ^(NSArray * value) {
                                          MTR_LOG_INFO("%@ got attribute report %@", self, value);
                                          dispatch_async(self.queue, ^{
                                              // OnAttributeData (after OnReportEnd)
                                              [self _handleAttributeReport:value];
                                          });
                                      },
                                      ^(NSArray * value) {
                                          MTR_LOG_INFO("%@ got event report %@", self, value);
                                          dispatch_async(self.queue, ^{
                                              // OnEventReport (after OnReportEnd)
                                              [self _handleEventReport:value];
                                          });
                                      },
                                      ^(NSError * error) {
                                          MTR_LOG_INFO("%@ got subscription error %@", self, error);
                                          dispatch_async(self.queue, ^{
                                              // OnError
                                              [self _handleSubscriptionError:error];
                                          });
                                      },
                                      ^(NSError * error, NSNumber * resubscriptionDelay) {
                                          MTR_LOG_INFO("%@ got resubscription error %@ delay %@", self, error, resubscriptionDelay);
                                          dispatch_async(self.queue, ^{
                                              // OnResubscriptionNeeded
                                              [self _handleResubscriptionNeeded];
                                          });
                                      },
                                      ^(void) {
                                          MTR_LOG_INFO("%@ got subscription established", self);
                                          dispatch_async(self.queue, ^{
                                              // OnSubscriptionEstablished
                                              [self _handleSubscriptionEstablished];
                                          });
                                      },
                                      ^(void) {
                                          MTR_LOG_INFO("%@ got subscription done", self);
                                          dispatch_async(self.queue, ^{
                                              // OnDone
                                              [self _handleSubscriptionReset];
                                          });
                                      });

                                  // Set up a cluster state cache.  We really just want this for the
                                  // logic it has for tracking data versions and event numbers so we
                                  // minimize the amount of data we request on resubscribes; we
                                  // don't care about the data it stores.  Ideally we could use the
                                  // dataversion-management logic without needing to store the data
                                  // separately from the data store we already have, or we would
                                  // stop storing our data separately.
                                  auto clusterStateCache = std::make_unique<ClusterStateCache>(*callback.get());
                                  auto readClient
                                      = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), exchangeManager,
                                          clusterStateCache->GetBufferedCallback(), ReadClient::InteractionType::Subscribe);

                                  // SendAutoResubscribeRequest cleans up the params, even on failure.
                                  CHIP_ERROR err = readClient->SendAutoResubscribeRequest(std::move(readParams));

                                  if (err != CHIP_NO_ERROR) {
                                      NSError * error = [MTRError errorForCHIPErrorCode:err];
                                      MTR_LOG_INFO("%@ SendAutoResubscribeRequest error %@", self, error);
                                      dispatch_async(self.queue, ^{
                                          [self _handleSubscriptionError:error];
                                      });

                                      return;
                                  }

                                  // Callback and ClusterStateCache and ReadClient will be deleted
                                  // when OnDone is called or an error is encountered.
                                  callback->AdoptReadClient(std::move(readClient));
                                  callback->AdoptClusterStateCache(std::move(clusterStateCache));
                                  callback.release();
                              }];
}

#pragma mark Device Interactions
- (NSDictionary<NSString *, id> *)readAttributeWithEndpointID:(NSNumber *)endpointID
                                                    clusterID:(NSNumber *)clusterID
                                                  attributeID:(NSNumber *)attributeID
                                                       params:(MTRReadParams *)params
{
    NSString * logPrefix = [NSString stringWithFormat:@"%@ read %@ %@ %@", self, endpointID, clusterID, attributeID];
    // Create work item, set ready handler to perform task, then enqueue the work
    MTRAsyncCallbackQueueWorkItem * workItem = [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:self.queue];
    MTRAsyncCallbackReadyHandler readyHandler = ^(MTRDevice * device, NSUInteger retryCount) {
        MTR_LOG_INFO("%@ dequeueWorkItem %@", logPrefix, self->_asyncCallbackWorkQueue);
        MTRBaseDevice * baseDevice = [self newBaseDevice];
        [baseDevice
            readAttributesWithEndpointID:endpointID
                               clusterID:clusterID
                             attributeID:attributeID
                                  params:params
                                   queue:self.queue
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  if (values) {
                                      // Since the format is the same data-value dictionary, this looks like an attribute
                                      // report
                                      MTR_LOG_INFO("%@ completion values %@", logPrefix, values);
                                      [self _handleAttributeReport:values];
                                  }

                                  // TODO: better retry logic
                                  if (error && (retryCount < 2)) {
                                      MTR_LOG_INFO(
                                          "%@ completion error %@ retryWork %lu", logPrefix, error, (unsigned long) retryCount);
                                      [workItem retryWork];
                                  } else {
                                      MTR_LOG_INFO("%@ completion error %@ endWork", logPrefix, error);
                                      [workItem endWork];
                                  }
                              }];
    };
    workItem.readyHandler = readyHandler;
    MTR_LOG_INFO("%@ enqueueWorkItem %@", logPrefix, _asyncCallbackWorkQueue);
    [_asyncCallbackWorkQueue enqueueWorkItem:workItem];

    // Return current known / expected value right away
    MTRAttributePath * attributePath = [MTRAttributePath attributePathWithEndpointID:endpointID
                                                                           clusterID:clusterID
                                                                         attributeID:attributeID];
    NSDictionary<NSString *, id> * attributeValueToReturn = [self _attributeValueDictionaryForAttributePath:attributePath];

    return attributeValueToReturn;
}

- (void)writeAttributeWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                         attributeID:(NSNumber *)attributeID
                               value:(id)value
               expectedValueInterval:(NSNumber *)expectedValueInterval
                   timedWriteTimeout:(NSNumber * _Nullable)timeout
{
    NSString * logPrefix = [NSString stringWithFormat:@"%@ write %@ %@ %@", self, endpointID, clusterID, attributeID];
    if (timeout) {
        timeout = MTRClampedNumber(timeout, @(1), @(UINT16_MAX));
    }
    expectedValueInterval = MTRClampedNumber(expectedValueInterval, @(1), @(UINT32_MAX));
    MTRAsyncCallbackQueueWorkItem * workItem = [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:self.queue];
    MTRAsyncCallbackReadyHandler readyHandler = ^(MTRDevice * device, NSUInteger retryCount) {
        MTR_LOG_INFO("%@ dequeueWorkItem %@", logPrefix, self->_asyncCallbackWorkQueue);
        MTRBaseDevice * baseDevice = [self newBaseDevice];
        [baseDevice
            writeAttributeWithEndpointID:endpointID
                               clusterID:clusterID
                             attributeID:attributeID
                                   value:value
                       timedWriteTimeout:timeout
                                   queue:self.queue
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  MTR_LOG_INFO("%@ completion error %@ endWork", logPrefix, error);
                                  [workItem endWork];
                              }];
    };
    workItem.readyHandler = readyHandler;
    MTR_LOG_INFO("%@ enqueueWorkItem %@", logPrefix, _asyncCallbackWorkQueue);
    [_asyncCallbackWorkQueue enqueueWorkItem:workItem];

    // Commit change into expected value cache
    MTRAttributePath * attributePath = [MTRAttributePath attributePathWithEndpointID:endpointID
                                                                           clusterID:clusterID
                                                                         attributeID:attributeID];
    NSDictionary * newExpectedValueDictionary = @{ MTRAttributePathKey : attributePath, MTRDataKey : value };

    [self setExpectedValues:@[ newExpectedValueDictionary ] expectedValueInterval:expectedValueInterval];
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
    NSString * logPrefix = [NSString stringWithFormat:@"%@ command %@ %@ %@", self, endpointID, clusterID, commandID];
    if (timeout) {
        timeout = MTRClampedNumber(timeout, @(1), @(UINT16_MAX));
    }
    if (!expectedValueInterval || ([expectedValueInterval compare:@(0)] == NSOrderedAscending)) {
        expectedValues = nil;
    } else {
        expectedValueInterval = MTRClampedNumber(expectedValueInterval, @(1), @(UINT32_MAX));
    }
    MTRAsyncCallbackQueueWorkItem * workItem = [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:self.queue];
    MTRAsyncCallbackReadyHandler readyHandler = ^(MTRDevice * device, NSUInteger retryCount) {
        MTR_LOG_INFO("%@ dequeueWorkItem %@", logPrefix, self->_asyncCallbackWorkQueue);
        MTRBaseDevice * baseDevice = [self newBaseDevice];
        [baseDevice
            invokeCommandWithEndpointID:endpointID
                              clusterID:clusterID
                              commandID:commandID
                          commandFields:commandFields
                     timedInvokeTimeout:timeout
                                  queue:self.queue
                             completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                 MTR_LOG_INFO("%@ completion values %@ error %@ endWork", logPrefix, values, error);
                                 dispatch_async(queue, ^{
                                     completion(values, error);
                                 });
                                 [workItem endWork];
                             }];
    };
    workItem.readyHandler = readyHandler;
    MTR_LOG_INFO("%@ enqueueWorkItem %@", logPrefix, _asyncCallbackWorkQueue);
    [_asyncCallbackWorkQueue enqueueWorkItem:workItem];

    if (expectedValues) {
        [self setExpectedValues:expectedValues expectedValueInterval:expectedValueInterval];
    }
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

#pragma mark - Cache management

// assume lock is held
- (void)_checkExpiredExpectedValues
{
    os_unfair_lock_assert_owner(&self->_lock);

    // find expired attributes, and calculate next timer fire date
    NSDate * now = [NSDate date];
    NSDate * nextExpirationDate = nil;
    NSMutableSet<MTRPair<MTRAttributePath *, NSDictionary *> *> * attributeInfoToRemove = [NSMutableSet set];
    for (MTRAttributePath * attributePath in _expectedValueCache) {
        MTRPair<NSDate *, NSDictionary *> * expectedValue = _expectedValueCache[attributePath];
        NSDate * attributeExpirationDate = expectedValue.first;
        if (expectedValue) {
            if ([now compare:attributeExpirationDate] == NSOrderedDescending) {
                // expired - save [path, values] pair to attributeToRemove
                [attributeInfoToRemove addObject:[MTRPair pairWithFirst:attributePath second:expectedValue.second]];
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
    for (MTRPair<MTRAttributePath *, NSDictionary *> * attributeInfo in attributeInfoToRemove) {
        // compare with known value and mark for report if different
        MTRAttributePath * attributePath = attributeInfo.first;
        NSDictionary * attributeDataValue = attributeInfo.second;
        NSDictionary * cachedAttributeDataValue = _readCache[attributePath];
        if (cachedAttributeDataValue
            && ![self _attributeDataValue:attributeDataValue isEqualToDataValue:cachedAttributeDataValue]) {
            [attributesToReport addObject:@{ MTRAttributePathKey : attributePath, MTRDataKey : cachedAttributeDataValue }];
            [attributePathsToReport addObject:attributePath];
        }

        _expectedValueCache[attributePath] = nil;
    }

    // log attribute paths
    MTR_LOG_INFO("%@ report from expired expected values %@", self, attributePathsToReport);
    [self _reportAttributes:attributesToReport];

// Have a reasonable minimum wait time for expiration timers
#define MTR_DEVICE_EXPIRATION_CHECK_TIMER_MINIMUM_WAIT_TIME (0.1)

    if (nextExpirationDate && _expectedValueCache.count && !self.expirationCheckScheduled) {
        NSTimeInterval waitTime = [nextExpirationDate timeIntervalSinceDate:now];
        if (waitTime < MTR_DEVICE_EXPIRATION_CHECK_TIMER_MINIMUM_WAIT_TIME) {
            waitTime = MTR_DEVICE_EXPIRATION_CHECK_TIMER_MINIMUM_WAIT_TIME;
        }
        MTRWeakReference<MTRDevice *> * weakSelf = [MTRWeakReference weakReferenceWithObject:self];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(waitTime * NSEC_PER_SEC)), self.queue, ^{
            MTRDevice * strongSelf = weakSelf.strongObject;
            [strongSelf _performScheduledExpirationCheck];
        });
    }
}

- (void)_performScheduledExpirationCheck
{
    os_unfair_lock_lock(&self->_lock);

    self.expirationCheckScheduled = NO;
    [self _checkExpiredExpectedValues];

    os_unfair_lock_unlock(&self->_lock);
}

// Get attribute value dictionary for an attribute path from the right cache
- (NSDictionary<NSString *, id> *)_attributeValueDictionaryForAttributePath:(MTRAttributePath *)attributePath
{
    os_unfair_lock_lock(&self->_lock);

    // First check expected value cache
    MTRPair<NSDate *, NSDictionary *> * expectedValue = _expectedValueCache[attributePath];
    if (expectedValue) {
        NSDate * now = [NSDate date];
        if ([now compare:expectedValue.first] == NSOrderedDescending) {
            // expired - purge and fall through
            _expectedValueCache[attributePath] = nil;
        } else {
            os_unfair_lock_unlock(&self->_lock);

            // not yet expired - return result
            return expectedValue.second;
        }
    }

    // Then check read cache
    NSDictionary<NSString *, id> * cachedAttributeValue = _readCache[attributePath];
    if (cachedAttributeValue) {
        os_unfair_lock_unlock(&self->_lock);

        return cachedAttributeValue;
    } else {
        // TODO: when not found in cache, generated default values should be used
        MTR_LOG_INFO("%@ _attributeValueDictionaryForAttributePath: could not find cached attribute values for attribute %@", self,
            attributePath);
    }

    os_unfair_lock_unlock(&self->_lock);

    return nil;
}

- (BOOL)_attributeDataValue:(NSDictionary *)one isEqualToDataValue:(NSDictionary *)theOther
{
    // Attribute data-value dictionary should be all standard containers which
    // means isEqual: comparisons all the way down, making a deep comparison.
    return [one isEqualToDictionary:theOther];
}

// assume lock is held
- (NSArray *)_getAttributesToReportWithReportedValues:(NSArray<NSDictionary<NSString *, id> *> *)reportedAttributeValues
{
    os_unfair_lock_assert_owner(&self->_lock);

    NSMutableArray * attributesToReport = [NSMutableArray array];
    NSMutableArray * attributePathsToReport = [NSMutableArray array];
    for (NSDictionary<NSString *, id> * attributeReponseValue in reportedAttributeValues) {
        MTRAttributePath * attributePath = attributeReponseValue[MTRAttributePathKey];
        NSDictionary * attributeDataValue = attributeReponseValue[MTRDataKey];
        NSError * attributeError = attributeReponseValue[MTRErrorKey];

        // sanity check either data value or error must exist
        if (!attributeDataValue && attributeError) {
            continue;
        }

        // check if value is different than cache, and report if needed
        BOOL shouldReportAttribute = NO;

        // if this is an error, report and purge cache
        if (attributeError) {
            shouldReportAttribute = YES;
            MTR_LOG_INFO("%@ report %@ error %@ purge expected value %@ read cache %@", self, attributePath, attributeError,
                _expectedValueCache[attributePath], _readCache[attributePath]);
            _expectedValueCache[attributePath] = nil;
            _readCache[attributePath] = nil;
        } else {
            // if expected values exists, purge and update read cache
            MTRPair<NSDate *, NSDictionary *> * expectedValue = _expectedValueCache[attributePath];
            if (expectedValue) {
                if (![self _attributeDataValue:attributeDataValue isEqualToDataValue:expectedValue.second]) {
                    shouldReportAttribute = YES;
                }
                _expectedValueCache[attributePath] = nil;
                _readCache[attributePath] = attributeDataValue;
            } else if (![self _attributeDataValue:attributeDataValue isEqualToDataValue:_readCache[attributePath]]) {
                // otherwise compare and update read cache
                _readCache[attributePath] = attributeDataValue;
                shouldReportAttribute = YES;
            }

            if (!shouldReportAttribute) {
                if (expectedValue) {
                    MTR_LOG_INFO("%@ report %@ value filtered - same as expected values", self, attributePath);
                } else {
                    MTR_LOG_INFO("%@ report %@ value filtered - same values as cache", self, attributePath);
                }
            }
        }

        if (shouldReportAttribute) {
            [attributesToReport addObject:attributeReponseValue];
            [attributePathsToReport addObject:attributePath];
        }
    }

    MTR_LOG_INFO("%@ report from reported values %@", self, attributePathsToReport);

    return attributesToReport;
}

// assume lock is held
- (NSArray *)_getAttributesToReportWithNewExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedAttributeValues
                                          expirationTime:(NSDate *)expirationTime
{
    os_unfair_lock_assert_owner(&self->_lock);

    NSMutableArray * attributesToReport = [NSMutableArray array];
    NSMutableArray * attributePathsToReport = [NSMutableArray array];
    for (NSDictionary<NSString *, id> * attributeReponseValue in expectedAttributeValues) {
        MTRAttributePath * attributePath = attributeReponseValue[MTRAttributePathKey];
        NSDictionary * attributeDataValue = attributeReponseValue[MTRDataKey];

        // check if value is different than cache, and report if needed
        BOOL shouldReportAttribute = NO;

        // first check write cache and purge / update
        MTRPair<NSDate *, NSDictionary *> * previousExpectedValue = _expectedValueCache[attributePath];
        if (previousExpectedValue) {
            if (![self _attributeDataValue:attributeDataValue isEqualToDataValue:previousExpectedValue.second]) {
                shouldReportAttribute = YES;
            }
        } else {
            // if new expected value then compare with read cache and report if needed
            if (![self _attributeDataValue:attributeDataValue isEqualToDataValue:_readCache[attributePath]]) {
                shouldReportAttribute = YES;
            }
        }
        _expectedValueCache[attributePath] = [MTRPair pairWithFirst:expirationTime second:attributeDataValue];

        if (shouldReportAttribute) {
            [attributesToReport addObject:attributeReponseValue];
            [attributePathsToReport addObject:attributePath];
        }
    }

    MTR_LOG_INFO("%@ report from new expected values %@", self, attributePathsToReport);

    return attributesToReport;
}

- (void)setExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)values expectedValueInterval:(NSNumber *)expectedValueInterval
{
    // since NSTimeInterval is in seconds, convert ms into seconds in double
    NSDate * expirationTime = [NSDate dateWithTimeIntervalSinceNow:expectedValueInterval.doubleValue / 1000];

    MTR_LOG_INFO(
        "Setting expected values %@ with expiration time %f seconds from now", values, [expirationTime timeIntervalSinceNow]);

    os_unfair_lock_lock(&self->_lock);

    // _getAttributesToReportWithNewExpectedValues will log attribute paths reported
    NSArray * attributesToReport = [self _getAttributesToReportWithNewExpectedValues:values expirationTime:expirationTime];
    [self _reportAttributes:attributesToReport];

    [self _checkExpiredExpectedValues];
    os_unfair_lock_unlock(&self->_lock);
}

- (MTRBaseDevice *)newBaseDevice
{
    return [[MTRBaseDevice alloc] initWithNodeID:self.nodeID controller:self.deviceController];
}

@end

@implementation MTRDevice (Deprecated)

+ (instancetype)deviceWithNodeID:(uint64_t)nodeID deviceController:(MTRDeviceController *)deviceController
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
        if (value) {
            [mEventReports addObject:@ { MTREventPathKey : eventPath, MTRDataKey : value }];
        }
    }
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
        id value = MTRDecodeDataValueDictionaryFromCHIPTLV(apData);
        if (value) {
            [mAttributeReports addObject:@ { MTRAttributePathKey : attributePath, MTRDataKey : value }];
        }
    }
}
} // anonymous namespace
