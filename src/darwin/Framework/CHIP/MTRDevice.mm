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
#import "MTRLogging.h"

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
@property (nonatomic) dispatch_queue_t queue;
@property (nonatomic) MTRWeakReference<id<MTRDeviceDelegate>> * weakDelegate;
@property (nonatomic) dispatch_queue_t delegateQueue;
@property (nonatomic) NSArray<NSDictionary<NSString *, id> *> * unreportedEvents;

@property (nonatomic) BOOL subscriptionActive;

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
        _deviceController = controller;
        _queue = dispatch_queue_create("com.apple.matter.framework.xpc.workqueue", DISPATCH_QUEUE_SERIAL);
        ;
        _readCache = [NSMutableDictionary dictionary];
        _expectedValueCache = [NSMutableDictionary dictionary];
        _asyncCallbackWorkQueue = [[MTRAsyncCallbackWorkQueue alloc] initWithContext:self queue:_queue];
        _state = MTRDeviceStateUnknown;
    }
    return self;
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
    os_unfair_lock_lock(&self->_lock);

    _weakDelegate = [MTRWeakReference weakReferenceWithObject:delegate];
    _delegateQueue = queue;
    [self setupSubscription];

    os_unfair_lock_unlock(&self->_lock);
}

- (void)_handleSubscriptionEstablished
{
    os_unfair_lock_lock(&self->_lock);

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
    // TODO: logic to reattempt subscription with exponential back off
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

- (void)setupSubscription
{
    // for now just subscribe once
    if (_subscriptionActive) {
        return;
    }

    _subscriptionActive = YES;

    [_deviceController getSessionForNode:_nodeID.unsignedLongLongValue
                              completion:^(chip::Messaging::ExchangeManager * _Nullable exchangeManager,
                                  const chip::Optional<chip::SessionHandle> & session, NSError * _Nullable error) {
                                  if (error != nil) {
                                      dispatch_async(self.queue, ^{
                                          [self _handleSubscriptionError:error];
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
                                  attributePath.release();
                                  eventPath.release();

                                  std::unique_ptr<SubscriptionCallback> callback;
                                  std::unique_ptr<ReadClient> readClient;
                                  std::unique_ptr<ClusterStateCache> attributeCache;
                                  callback = std::make_unique<SubscriptionCallback>(
                                      ^(NSArray * value) {
                                          dispatch_async(self.queue, ^{
                                              // OnAttributeData (after OnReportEnd)
                                              [self _handleAttributeReport:value];
                                          });
                                      },
                                      ^(NSArray * value) {
                                          dispatch_async(self.queue, ^{
                                              // OnEventReport (after OnReportEnd)
                                              [self _handleEventReport:value];
                                          });
                                      },
                                      ^(NSError * error) {
                                          dispatch_async(self.queue, ^{
                                              // OnError
                                              [self _handleSubscriptionError:error];
                                          });
                                      },
                                      ^(NSError * error, NSNumber * resubscriptionDelay) {
                                          dispatch_async(self.queue, ^{
                                              // OnResubscriptionNeeded
                                              [self _handleResubscriptionNeeded];
                                          });
                                      },
                                      ^(void) {
                                          dispatch_async(self.queue, ^{
                                              // OnSubscriptionEstablished
                                              [self _handleSubscriptionEstablished];
                                          });
                                      },
                                      ^(void) {
                                          dispatch_async(self.queue, ^{
                                              // OnDone
                                              [self _handleSubscriptionReset];
                                          });
                                      });
                                  readClient = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), exchangeManager,
                                      callback->GetBufferedCallback(), ReadClient::InteractionType::Subscribe);

                                  // SendAutoResubscribeRequest cleans up the params, even on failure.
                                  CHIP_ERROR err = readClient->SendAutoResubscribeRequest(std::move(readParams));

                                  if (err != CHIP_NO_ERROR) {
                                      dispatch_async(self.queue, ^{
                                          [self _handleSubscriptionError:[MTRError errorForCHIPErrorCode:err]];
                                      });

                                      return;
                                  }

                                  // Callback and ReadClient will be deleted when OnDone is called or an error is
                                  // encountered.
                                  callback->AdoptReadClient(std::move(readClient));
                                  callback.release();
                              }];
}

#pragma mark Device Interactions
- (NSDictionary<NSString *, id> *)readAttributeWithEndpointID:(NSNumber *)endpointID
                                                    clusterID:(NSNumber *)clusterID
                                                  attributeID:(NSNumber *)attributeID
                                                       params:(MTRReadParams *)params
{
    // Create work item, set ready handler to perform task, then enqueue the work
    MTRAsyncCallbackQueueWorkItem * workItem = [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:_queue];
    MTRAsyncCallbackReadyHandler readyHandler = ^(MTRDevice * device, NSUInteger retryCount) {
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
                                      [self _handleAttributeReport:values];
                                  }

                                  // TODO: better retry logic
                                  if (error && (retryCount < 2)) {
                                      [workItem retryWork];
                                  } else {
                                      [workItem endWork];
                                  }
                              }];
    };
    workItem.readyHandler = readyHandler;
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
    if (timeout) {
        timeout = MTRClampedNumber(timeout, @(1), @(UINT16_MAX));
    }
    expectedValueInterval = MTRClampedNumber(expectedValueInterval, @(1), @(UINT32_MAX));
    MTRAsyncCallbackQueueWorkItem * workItem = [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:_queue];
    MTRAsyncCallbackReadyHandler readyHandler = ^(MTRDevice * device, NSUInteger retryCount) {
        MTRBaseDevice * baseDevice = [self newBaseDevice];
        [baseDevice
            writeAttributeWithEndpointID:endpointID
                               clusterID:clusterID
                             attributeID:attributeID
                                   value:value
                       timedWriteTimeout:timeout
                                   queue:self.queue
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  if (values) {
                                      [self _handleAttributeReport:values];
                                  }
                                  [workItem endWork];
                              }];
    };
    workItem.readyHandler = readyHandler;
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
    if (timeout) {
        timeout = MTRClampedNumber(timeout, @(1), @(UINT16_MAX));
    }
    if (!expectedValueInterval || ([expectedValueInterval compare:@(0)] == NSOrderedAscending)) {
        expectedValues = nil;
    } else {
        expectedValueInterval = MTRClampedNumber(expectedValueInterval, @(1), @(UINT32_MAX));
    }
    MTRAsyncCallbackQueueWorkItem * workItem = [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:_queue];
    MTRAsyncCallbackReadyHandler readyHandler = ^(MTRDevice * device, NSUInteger retryCount) {
        MTRBaseDevice * baseDevice = [self newBaseDevice];
        [baseDevice
            invokeCommandWithEndpointID:endpointID
                              clusterID:clusterID
                              commandID:commandID
                          commandFields:commandFields
                     timedInvokeTimeout:timeout
                                  queue:self.queue
                             completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                 dispatch_async(queue, ^{
                                     completion(values, error);
                                 });
                                 [workItem endWork];
                             }];
    };
    workItem.readyHandler = readyHandler;
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
    for (MTRPair<MTRAttributePath *, NSDictionary *> * attributeInfo in attributeInfoToRemove) {
        // compare with known value and mark for report if different
        MTRAttributePath * attributePath = attributeInfo.first;
        NSDictionary * attributeDataValue = attributeInfo.second;
        NSDictionary * cachedAttributeDataValue = _readCache[attributePath];
        if (cachedAttributeDataValue
            && ![self _attributeDataValue:attributeDataValue isEqualToDataValue:cachedAttributeDataValue]) {
            [attributesToReport addObject:@{ MTRAttributePathKey : attributePath, MTRDataKey : cachedAttributeDataValue }];
        }

        _expectedValueCache[attributePath] = nil;
    }
    [self _reportAttributes:attributesToReport];

// Have a reasonable minimum wait time for expiration timers
#define MTR_DEVICE_EXPIRATION_CHECK_TIMER_MINIMUM_WAIT_TIME (0.1)

    if (nextExpirationDate && _expectedValueCache.count && !self.expirationCheckScheduled) {
        NSTimeInterval waitTime = [nextExpirationDate timeIntervalSinceDate:now];
        if (waitTime < MTR_DEVICE_EXPIRATION_CHECK_TIMER_MINIMUM_WAIT_TIME) {
            waitTime = MTR_DEVICE_EXPIRATION_CHECK_TIMER_MINIMUM_WAIT_TIME;
        }
        MTRWeakReference<MTRDevice *> * weakSelf = [MTRWeakReference weakReferenceWithObject:self];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(waitTime * NSEC_PER_SEC)), _queue, ^{
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
        MTR_LOG_INFO(
            "_attributeValueDictionaryForAttributePath: could not find cached attribute values for attribute %@", attributePath);
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
            _expectedValueCache[attributePath] = nil;
            _readCache[attributePath] = nil;
        } else {
            // check write cache and purge if needed
            MTRPair<NSDate *, NSDictionary *> * expectedValue = _expectedValueCache[attributePath];
            if (expectedValue) {
                if (![self _attributeDataValue:attributeDataValue isEqualToDataValue:expectedValue.second]) {
                    shouldReportAttribute = YES;
                }
                _expectedValueCache[attributePath] = nil;
            }

            // then compare and update read cache
            if (![self _attributeDataValue:attributeDataValue isEqualToDataValue:_readCache[attributePath]]) {
                _readCache[attributePath] = attributeDataValue;
                shouldReportAttribute = YES;
            }
        }

        if (shouldReportAttribute) {
            [attributesToReport addObject:attributeReponseValue];
        }
    }

    return attributesToReport;
}

// assume lock is held
- (NSArray *)_getAttributesToReportWithNewExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedAttributeValues
                                          expirationTime:(NSDate *)expirationTime
{
    os_unfair_lock_assert_owner(&self->_lock);

    NSMutableArray * attributesToReport = [NSMutableArray array];
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
        }
    }

    return attributesToReport;
}

- (void)setExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)values expectedValueInterval:(NSNumber *)expectedValueInterval
{
    // since NSTimeInterval is in seconds, convert ms into seconds in double
    NSDate * expirationTime = [NSDate dateWithTimeIntervalSinceNow:expectedValueInterval.doubleValue / 1000];

    os_unfair_lock_lock(&self->_lock);
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
