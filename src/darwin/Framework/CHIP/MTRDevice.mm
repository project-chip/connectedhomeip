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

#pragma mark - SubscriptionCallback class declaration
using namespace chip;
using namespace chip::app;
using namespace chip::Protocols::InteractionModel;

typedef void (^DataReportCallback)(NSArray * value);
typedef void (^ErrorCallback)(NSError * error);
typedef void (^ResubscriptionCallback)(void);

namespace {

class SubscriptionCallback final : public ClusterStateCache::Callback {
public:
    SubscriptionCallback(dispatch_queue_t queue, DataReportCallback attributeReportCallback, DataReportCallback eventReportCallback,
        ErrorCallback errorCallback, SubscriptionEstablishedHandler subscriptionEstablishedHandler,
        ResubscriptionCallback resubscriptionCallback, void (^onDoneHandler)(void))
        : mQueue(queue)
        , mAttributeReportCallback(attributeReportCallback)
        , mEventReportCallback(eventReportCallback)
        , mErrorCallback(errorCallback)
        , mSubscriptionEstablishedHandler(subscriptionEstablishedHandler)
        , mResubscriptionCallback(resubscriptionCallback)
        , mBufferedReadAdapter(*this)
        , mOnDoneHandler(onDoneHandler)
    {
    }

    ~SubscriptionCallback()
    {
        // Ensure we release the ReadClient before we tear down anything else,
        // so it can call our OnDeallocatePaths properly.
        mReadClient = nullptr;
    }

    BufferedReadCallback & GetBufferedCallback() { return mBufferedReadAdapter; }

    // We need to exist to get a ReadClient, so can't take this as a constructor argument.
    void AdoptReadClient(std::unique_ptr<ReadClient> aReadClient) { mReadClient = std::move(aReadClient); }
    void AdoptAttributeCache(std::unique_ptr<ClusterStateCache> aAttributeCache) { mAttributeCache = std::move(aAttributeCache); }

private:
    void OnReportBegin() override;

    void OnReportEnd() override;

    void OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus) override;

    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override;

    void OnError(CHIP_ERROR aError) override;

    void OnDone(ReadClient * aReadClient) override;

    void OnDeallocatePaths(ReadPrepareParams && aReadPrepareParams) override;

    void OnSubscriptionEstablished(SubscriptionId aSubscriptionId) override;

    CHIP_ERROR OnResubscriptionNeeded(ReadClient * apReadClient, CHIP_ERROR aTerminationCause) override;

    void ReportData();
    void ReportError(CHIP_ERROR err);
    void ReportError(const StatusIB & status);
    void ReportError(NSError * _Nullable err);

private:
    dispatch_queue_t mQueue;
    DataReportCallback _Nullable mAttributeReportCallback = nil;
    DataReportCallback _Nullable mEventReportCallback = nil;
    // We set mErrorCallback to nil when queueing error reports, so we
    // make sure to only report one error.
    ErrorCallback _Nullable mErrorCallback = nil;
    SubscriptionEstablishedHandler _Nullable mSubscriptionEstablishedHandler;
    ResubscriptionCallback mResubscriptionCallback;
    BufferedReadCallback mBufferedReadAdapter;
    NSMutableArray * _Nullable mAttributeReports = nil;
    NSMutableArray * _Nullable mEventReports = nil;

    // Our lifetime management is a little complicated.  On error we
    // attempt to delete the ReadClient, but asynchronously.  While
    // that's pending, someone else (e.g. an error it runs into) could
    // delete it too.  And if someone else does attempt to delete it, we want to
    // make sure we delete ourselves as well.
    //
    // To handle this, enforce the following rules:
    //
    // 1) We guarantee that mErrorCallback is only invoked with an error once.
    // 2) We ensure that we delete ourselves and the passed in ReadClient only from OnDone or a queued-up
    //    error callback, but not both, by tracking whether we have a queued-up
    //    deletion.
    std::unique_ptr<ReadClient> mReadClient;
    std::unique_ptr<ClusterStateCache> mAttributeCache;
    bool mHaveQueuedDeletion = false;
    void (^mOnDoneHandler)(void) = nil;
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

@property (nonatomic) MTRAsyncCallbackWorkQueue * asyncCallbackWorkQueue;
@end

@implementation MTRDevice

- (instancetype)initWithNodeID:(uint64_t)nodeID deviceController:(MTRDeviceController *)deviceController
{
    if (self = [super init]) {
        _lock = OS_UNFAIR_LOCK_INIT;
        _nodeID = nodeID;
        _deviceController = deviceController;
        _queue = dispatch_queue_create("com.apple.matter.framework.xpc.workqueue", DISPATCH_QUEUE_SERIAL);
        ;
        _readCache = [NSMutableDictionary dictionary];
        _expectedValueCache = [NSMutableDictionary dictionary];
        _asyncCallbackWorkQueue = [[MTRAsyncCallbackWorkQueue alloc] initWithContext:self queue:_queue];
        _state = MTRDeviceStateUnknown;
    }
    return self;
}

+ (instancetype)deviceWithNodeID:(uint64_t)nodeID deviceController:(MTRDeviceController *)deviceController
{
    return [deviceController deviceForNodeID:nodeID];
}

#pragma mark Subscription and delegate handling

// subscription intervals are in seconds
#define MTR_DEVICE_SUBSCRIPTION_MAX_INTERVAL_DEFAULT (3600)

- (void)setDelegate:(id<MTRDeviceDelegate>)delegate queue:(dispatch_queue_t)queue
{
    os_unfair_lock_lock(&self->_lock);

    _weakDelegate = [MTRWeakReference weakReferenceWithObject:delegate];
    _delegateQueue = queue;
    [self subscribeWithMinInterval:0 maxInterval:MTR_DEVICE_SUBSCRIPTION_MAX_INTERVAL_DEFAULT];

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

- (void)subscribeWithMinInterval:(uint16_t)minInterval maxInterval:(uint16_t)maxInterval
{
    // for now just subscribe once
    if (_subscriptionActive) {
        return;
    }

    _subscriptionActive = YES;

    [_deviceController getSessionForNode:_nodeID
                       completionHandler:^(chip::Messaging::ExchangeManager * _Nullable exchangeManager,
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
                           ReadPrepareParams readParams(session.Value());
                           readParams.mMinIntervalFloorSeconds = minInterval;
                           readParams.mMaxIntervalCeilingSeconds = maxInterval;
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
                               self.queue,
                               ^(NSArray * value) {
                                   // OnAttributeData (after OnReportEnd)
                                   [self _handleAttributeReport:value];
                               },
                               ^(NSArray * value) {
                                   // OnEventReport (after OnReportEnd)
                                   [self _handleEventReport:value];
                               },
                               ^(NSError * error) {
                                   // OnError
                                   [self _handleSubscriptionError:error];
                               },
                               ^(void) {
                                   // OnSubscriptionEstablished
                                   [self _handleSubscriptionEstablished];
                               },
                               ^(void) {
                                   // OnResubscriptionNeeded
                                   [self _handleResubscriptionNeeded];
                               },
                               ^(void) {
                                   // OnDone
                                   [self _handleSubscriptionReset];
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
        MTRBaseDevice * baseDevice = [[MTRBaseDevice alloc] initWithNodeID:self.nodeID controller:self.deviceController];

        [baseDevice
            readAttributeWithEndpointId:endpointID
                              clusterId:clusterID
                            attributeId:attributeID
                                 params:params
                            clientQueue:self.queue
                             completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                 if (values) {
                                     // Since the format is the same data-value dictionary, this looks like an attribute
                                     // report
                                     [self _handleAttributeReport:values];
                                 }

                                 // TODO: better retry logic
                                 if (retryCount < 2) {
                                     [workItem retryWork];
                                 } else {
                                     [workItem endWork];
                                 }
                             }];
    };
    workItem.readyHandler = readyHandler;
    [_asyncCallbackWorkQueue enqueueWorkItem:workItem];

    // Return current known / expected value right away
    MTRAttributePath * attributePath = [MTRAttributePath attributePathWithEndpointId:endpointID
                                                                           clusterId:clusterID
                                                                         attributeId:attributeID];
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
    // Start the asynchronous operation
    MTRBaseDevice * baseDevice = [[MTRBaseDevice alloc] initWithNodeID:self.nodeID controller:self.deviceController];
    [baseDevice
        writeAttributeWithEndpointId:endpointID
                           clusterId:clusterID
                         attributeId:attributeID
                               value:value
                   timedWriteTimeout:timeout
                         clientQueue:self.queue
                          completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                              if (values) {
                                  [self _handleAttributeReport:values];
                              }
                          }];

    // Commit change into expected value cache
    MTRAttributePath * attributePath = [MTRAttributePath attributePathWithEndpointId:endpointID
                                                                           clusterId:clusterID
                                                                         attributeId:attributeID];
    NSDictionary * newExpectedValueDictionary = @{ MTRAttributePathKey : attributePath, MTRDataKey : value };

    [self setExpectedValues:@[ newExpectedValueDictionary ] expectedValueInterval:expectedValueInterval];
}

- (void)invokeCommandWithEndpointID:(NSNumber *)endpointID
                          clusterID:(NSNumber *)clusterID
                          commandID:(NSNumber *)commandID
                      commandFields:(id)commandFields
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
              expectedValueInterval:(NSNumber *)expectedValueInterval
                 timedInvokeTimeout:(NSNumber * _Nullable)timeout
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion
{
    // Perform this operation
    MTRBaseDevice * baseDevice = [[MTRBaseDevice alloc] initWithNodeID:self.nodeID controller:self.deviceController];
    [baseDevice
        invokeCommandWithEndpointId:endpointID
                          clusterId:clusterID
                          commandId:commandID
                      commandFields:commandFields
                 timedInvokeTimeout:timeout
                        clientQueue:self.queue
                         completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                             dispatch_async(clientQueue, ^{
                                 completion(values, error);
                             });
                         }];

    [self setExpectedValues:expectedValues expectedValueInterval:expectedValueInterval];
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
            [attributesToReport addObject:cachedAttributeDataValue];
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

@end

#pragma mark - SubscriptionCallback
namespace {
void SubscriptionCallback::OnReportBegin()
{
    mAttributeReports = [NSMutableArray new];
    mEventReports = [NSMutableArray new];
}

// Reports attribute and event data if any exists
void SubscriptionCallback::ReportData()
{
    __block NSArray * attributeReports = mAttributeReports;
    mAttributeReports = nil;
    __block NSArray * eventReports = mEventReports;
    mEventReports = nil;
    if (mAttributeReportCallback && attributeReports.count) {
        dispatch_async(mQueue, ^{
            mAttributeReportCallback(attributeReports);
        });
    }
    if (mEventReportCallback && eventReports.count) {
        dispatch_async(mQueue, ^{
            mEventReportCallback(eventReports);
        });
    }
}

void SubscriptionCallback::OnReportEnd() { ReportData(); }

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

void SubscriptionCallback::OnError(CHIP_ERROR aError)
{
    // If OnError is called after OnReportBegin, we should report the collected data
    ReportData();
    ReportError([MTRError errorForCHIPErrorCode:aError]);
}

void SubscriptionCallback::OnDone(ReadClient *)
{
    if (mOnDoneHandler) {
        mOnDoneHandler();
        mOnDoneHandler = nil;
    }
    if (!mHaveQueuedDeletion) {
        delete this;
        return; // Make sure we touch nothing else.
    }
}

void SubscriptionCallback::OnDeallocatePaths(ReadPrepareParams && aReadPrepareParams)
{
    VerifyOrDie((aReadPrepareParams.mAttributePathParamsListSize == 0 && aReadPrepareParams.mpAttributePathParamsList == nullptr)
        || (aReadPrepareParams.mAttributePathParamsListSize == 1 && aReadPrepareParams.mpAttributePathParamsList != nullptr));
    if (aReadPrepareParams.mpAttributePathParamsList) {
        delete aReadPrepareParams.mpAttributePathParamsList;
    }

    VerifyOrDie((aReadPrepareParams.mDataVersionFilterListSize == 0 && aReadPrepareParams.mpDataVersionFilterList == nullptr)
        || (aReadPrepareParams.mDataVersionFilterListSize == 1 && aReadPrepareParams.mpDataVersionFilterList != nullptr));
    if (aReadPrepareParams.mpDataVersionFilterList != nullptr) {
        delete aReadPrepareParams.mpDataVersionFilterList;
    }

    VerifyOrDie((aReadPrepareParams.mEventPathParamsListSize == 0 && aReadPrepareParams.mpEventPathParamsList == nullptr)
        || (aReadPrepareParams.mEventPathParamsListSize == 1 && aReadPrepareParams.mpEventPathParamsList != nullptr));
    if (aReadPrepareParams.mpEventPathParamsList) {
        delete aReadPrepareParams.mpEventPathParamsList;
    }
}

void SubscriptionCallback::OnSubscriptionEstablished(SubscriptionId aSubscriptionId)
{
    if (mSubscriptionEstablishedHandler) {
        dispatch_async(mQueue, mSubscriptionEstablishedHandler);
    }
}

CHIP_ERROR SubscriptionCallback::OnResubscriptionNeeded(ReadClient * apReadClient, CHIP_ERROR aTerminationCause)
{
    return apReadClient->DefaultResubscribePolicy(aTerminationCause);
}

void SubscriptionCallback::ReportError(CHIP_ERROR err) { ReportError([MTRError errorForCHIPErrorCode:err]); }

void SubscriptionCallback::ReportError(const StatusIB & status) { ReportError([MTRError errorForIMStatus:status]); }

void SubscriptionCallback::ReportError(NSError * _Nullable err)
{
    if (!err) {
        // Very strange... Someone tried to create a MTRError for a success status?
        return;
    }

    if (mHaveQueuedDeletion) {
        // Already have an error report pending which will delete us.
        return;
    }

    __block ErrorCallback callback = mErrorCallback;
    __block auto * myself = this;
    mErrorCallback = nil;
    mAttributeReportCallback = nil;
    mEventReportCallback = nil;
    __auto_type onDoneHandler = mOnDoneHandler;
    mOnDoneHandler = nil;
    dispatch_async(mQueue, ^{
        callback(err);
        if (onDoneHandler) {
            onDoneHandler();
        }

        // Deletion of our ReadClient (and hence of ourselves, since the
        // ReadClient has a pointer to us) needs to happen on the Matter work
        // queue.
        dispatch_async(DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
            delete myself;
        });
    });

    mHaveQueuedDeletion = true;
}
} // anonymous namespace
