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

#import "MTRAsyncCallbackWorkQueue_Internal.h"
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

namespace {

class SubscriptionCallback final : public ClusterStateCache::Callback {
public:
    SubscriptionCallback(dispatch_queue_t queue, DataReportCallback attributeReportCallback, DataReportCallback eventReportCallback,
        ErrorCallback errorCallback, SubscriptionEstablishedHandler _Nullable subscriptionEstablishedHandler)
        : mQueue(queue)
        , mAttributeReportCallback(attributeReportCallback)
        , mEventReportCallback(eventReportCallback)
        , mErrorCallback(errorCallback)
        , mSubscriptionEstablishedHandler(subscriptionEstablishedHandler)
        , mBufferedReadAdapter(*this)
    {
    }

    SubscriptionCallback(dispatch_queue_t queue, DataReportCallback attributeReportCallback, DataReportCallback eventReportCallback,
        ErrorCallback errorCallback, SubscriptionEstablishedHandler _Nullable subscriptionEstablishedHandler,
        void (^onDoneHandler)(void))
        : mQueue(queue)
        , mAttributeReportCallback(attributeReportCallback)
        , mEventReportCallback(eventReportCallback)
        , mErrorCallback(errorCallback)
        , mSubscriptionEstablishedHandler(subscriptionEstablishedHandler)
        , mBufferedReadAdapter(*this)
        , mOnDoneHandler(onDoneHandler)
    {
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
@property (nonatomic, copy) MTRDevice * blahhey;
@property (nonatomic, copy) id<MTRDeviceSubscriptionDelegate> blah;
@property (nonatomic, readonly) os_unfair_lock lock;
@property (nonatomic) dispatch_queue_t queue;
@property (nonatomic) NSMutableSet<MTRWeakReference<id<MTRDeviceSubscriptionDelegate>> *> * subscribers;

@property (nonatomic) BOOL subscriptionActive;

// Read cache is attributePath => NSDictionary of value.
// See MTRDeviceResponseHandler definition for value dictionary details.
@property (nonatomic) NSMutableDictionary<MTRAttributePath *, NSDictionary *> * readCache;

// Expected value cache is attributePath => MTRPair of [NSDate of expiration time, NSDictionary of value]
// See MTRDeviceResponseHandler definition for value dictionary details.
@property (nonatomic) NSMutableDictionary<MTRAttributePath *, MTRPair<NSDate *, NSDictionary *> *> * expectedValueCache;

@property (nonatomic) MTRAsyncCallbackWorkQueue * asyncCallbackWorkQueue;
@end

@implementation MTRDevice

- (instancetype)initWithDeviceID:(uint64_t)deviceID
                deviceController:(MTRDeviceController *)deviceController
                           queue:(dispatch_queue_t)queue
{
    if (self = [super init]) {
        _lock = OS_UNFAIR_LOCK_INIT;
        _deviceID = deviceID;
        _deviceController = deviceController;
        _queue = queue;
        _readCache = [NSMutableDictionary dictionary];
        _expectedValueCache = [NSMutableDictionary dictionary];
        _asyncCallbackWorkQueue = [[MTRAsyncCallbackWorkQueue alloc] initWithContext:self queue:queue];
    }
    return self;
}

+ (instancetype)deviceWithDeviceID:(uint64_t)deviceID deviceController:(MTRDeviceController *)deviceController
{
    return [deviceController deviceForDeviceID:deviceID];
}

- (void)connectAndPerformAsync:(MTRDevicePerformAsyncBlock)asyncBlock
{
    [_deviceController getBaseDevice:_deviceID
                               queue:_queue
                   completionHandler:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                       asyncBlock(device);
                   }];
}

#pragma mark Subscription
- (void)performBlockForDelegates:(void (^)(id<MTRDeviceSubscriptionDelegate>))block
{
    os_unfair_lock_lock(&self->_lock);
    NSSet * delegatesToCall = [_subscribers copy];
    os_unfair_lock_unlock(&self->_lock);

    NSMutableSet * lostDelegates = [NSMutableSet set];
    for (MTRWeakReference<id<MTRDeviceSubscriptionDelegate>> * delegateWeakReference in delegatesToCall) {
        id<MTRDeviceSubscriptionDelegate> delegate = delegateWeakReference.strongObject;
        if (!delegate) {
            [lostDelegates addObject:delegateWeakReference];
        }

        block(delegate);
    }

    os_unfair_lock_lock(&self->_lock);
    [_subscribers minusSet:lostDelegates];
    os_unfair_lock_unlock(&self->_lock);

    // Options to consider when no more subscribers are alive:
    //   A) tear down subscription
    //   B) keep subscription until error, and wait for client to resubscribe
    //   C) keep subscription / resubscribe on error, until EOL MTRDevice
}

- (void)_handleSubscriptionEstablished
{
    [self performBlockForDelegates:^(id<MTRDeviceSubscriptionDelegate> delegate) {
        [delegate subscriptionEstablished];
    }];
}

- (void)_handleSubscriptionError:(NSError *)error
{
    _subscriptionActive = NO;

    [self performBlockForDelegates:^(id<MTRDeviceSubscriptionDelegate> delegate) {
        [delegate subscriptionEndedWithError:error];
    }];
    [_subscribers removeAllObjects];
}

- (void)_reportAttributes:(NSArray<NSDictionary<NSString *, id> *> *)attributes
{
    if (attributes.count) {
        [self performBlockForDelegates:^(id<MTRDeviceSubscriptionDelegate> delegate) {
            [delegate subscriptionReceivedAttributeReport:attributes];
        }];
    }
}

- (void)_handleAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport
{
    NSArray * attributesToReport = [self _getAttributesToReportWithReportedValues:attributeReport];

    [self _reportAttributes:attributesToReport];
}

- (void)_handleEventReport:(NSArray *)eventReport
{
    [self performBlockForDelegates:^(id<MTRDeviceSubscriptionDelegate> delegate) {
        [delegate subscriptionReceivedEventReport:eventReport];
    }];
}

- (void)subscribeWithMinInterval:(uint16_t)minInterval maxInterval:(uint16_t)maxInterval params:(MTRSubscribeParams *)params
{
    // for now just subscribe once
    if (_subscriptionActive) {
        return;
    }

    _subscriptionActive = YES;

    // Copy params before going async.
    params = [params copy];

    [_deviceController getSessionForNode:_deviceID
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
                           readParams.mKeepSubscriptions = [params.keepPreviousSubscriptions boolValue];

                           std::unique_ptr<SubscriptionCallback> callback;
                           std::unique_ptr<ReadClient> readClient;
                           std::unique_ptr<ClusterStateCache> attributeCache;
                           callback = std::make_unique<SubscriptionCallback>(
                               self.queue,
                               ^(NSArray * value) {
                                   [self _handleAttributeReport:value];
                               },
                               ^(NSArray * value) {
                                   [self _handleEventReport:value];
                               },
                               ^(NSError * error) {
                                   [self _handleSubscriptionError:error];
                               },
                               ^(void) {
                                   [self _handleSubscriptionEstablished];
                               });
                           readClient = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), exchangeManager,
                               callback->GetBufferedCallback(), ReadClient::InteractionType::Subscribe);

                           CHIP_ERROR err;
                           if (params != nil && params.autoResubscribe != nil && ![params.autoResubscribe boolValue]) {
                               err = readClient->SendRequest(readParams);
                           } else {
                               // SendAutoResubscribeRequest cleans up the params, even on failure.
                               attributePath.release();
                               eventPath.release();
                               err = readClient->SendAutoResubscribeRequest(std::move(readParams));
                           }

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

// TODO: support more than one subscriber / subscription delegate
//   * reconcile second+ subscriptions with different min and max intervals
//     (save min/max intervals for each subscriber and resubscribe when they change)
//   * events should be cached and replayed for second+ subscribers
- (void)subscribeWithDelegate:(id<MTRDeviceSubscriptionDelegate>)delegate
                        queue:(dispatch_queue_t)queue
                  minInterval:(uint16_t)minInterval
                  maxInterval:(uint16_t)maxInterval
                       params:(MTRSubscribeParams *)params
{
    // Save a weak reference to delegate, so if/when the object goes away, they are automatically unsubscribed
    MTRWeakReference<id<MTRDeviceSubscriptionDelegate>> * weakDelegate = [MTRWeakReference weakReferenceWithObject:delegate];
    [_subscribers addObject:weakDelegate];

    // perform the actual subscription
    [self subscribeWithMinInterval:minInterval maxInterval:maxInterval params:params];
}

#pragma mark Device Interactions
- (NSDictionary<NSString *, id> *)readAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                                                    clusterId:(NSNumber * _Nullable)clusterId
                                                  attributeId:(NSNumber * _Nullable)attributeId
                                                       params:(MTRReadParams * _Nullable)params
{
    // Create work item, set ready handler to perform task, then enqueue the work
    MTRAsyncCallbackQueueWorkItem * workItem = [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:_queue];
    MTRAsyncCallbackReadyHandler readyHandler = ^(MTRDevice * device, NSUInteger retryCount) {
        [self connectAndPerformAsync:^(MTRBaseDevice * baseDevice) {
            [baseDevice readAttributeWithEndpointId:endpointId
                                          clusterId:clusterId
                                        attributeId:attributeId
                                             params:params
                                        clientQueue:self.queue
                                         completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values,
                                             NSError * _Nullable error) {
                                             if (values) {
                                                 // Since the format is the same data-value dictionary, this looks like an attribute
                                                 // report
                                                 [self _handleAttributeReport:values];
                                             }
                                             // TODO: retry on error
                                             [workItem endWork];
                                         }];
        }];
    };
    workItem.readyHandler = readyHandler;
    [_asyncCallbackWorkQueue enqueueWorkItem:workItem];

    // Return current known / expected value right away
    MTRAttributePath * attributePath = [MTRAttributePath attributePathWithEndpointId:endpointId
                                                                           clusterId:clusterId
                                                                         attributeId:attributeId];
    NSDictionary<NSString *, id> * attributeValueToReturn = [self _attributeValueDictionaryForAttributePath:attributePath];

    return attributeValueToReturn;
}

- (void)writeAttributeWithEndpointId:(NSNumber *)endpointId
                           clusterId:(NSNumber *)clusterId
                         attributeId:(NSNumber *)attributeId
                               value:(id)value
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                   timedWriteTimeout:(NSNumber * _Nullable)timeoutMs
{
    // Start the asynchronous operation
    [self connectAndPerformAsync:^(MTRBaseDevice * baseDevice) {
        [baseDevice
            writeAttributeWithEndpointId:endpointId
                               clusterId:clusterId
                             attributeId:attributeId
                                   value:value
                       timedWriteTimeout:timeoutMs
                             clientQueue:self.queue
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  if (values) {
                                      [self _handleAttributeReport:values];
                                  }
                              }];
    }];

    // Commit change into expected value cache
    MTRAttributePath * attributePath = [MTRAttributePath attributePathWithEndpointId:endpointId
                                                                           clusterId:clusterId
                                                                         attributeId:attributeId];
    NSDictionary * newExpectedValueDictionary = @{ MTRAttributePathKey : attributePath, MTRDataKey : value };

    [self setExpectedValues:@[ newExpectedValueDictionary ] expectedValueInterval:expectedValueIntervalMs];
}

- (void)invokeCommandWithEndpointId:(NSNumber *)endpointId
                          clusterId:(NSNumber *)clusterId
                          commandId:(NSNumber *)commandId
                      commandFields:(id)commandFields
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                 timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion
{
    // Perform this operation
    [self connectAndPerformAsync:^(MTRBaseDevice * baseDevice) {
        [baseDevice
            invokeCommandWithEndpointId:endpointId
                              clusterId:clusterId
                              commandId:commandId
                          commandFields:commandFields
                     timedInvokeTimeout:timeoutMs
                            clientQueue:self.queue
                             completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                 dispatch_async(clientQueue, ^{
                                     completion(values, error);
                                 });
                             }];
    }];

    [self setExpectedValues:expectedValues expectedValueInterval:expectedValueIntervalMs];
}

#pragma mark - Cache management

// (assume lock is held)
- (void)_checkExpiredExpectedValues
{
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
            // not yet expired - return result
            return expectedValue.second;
        }
    }

    // Then check read cache
    NSDictionary<NSString *, id> * cachedAttributeValue = _readCache[attributePath];
    if (cachedAttributeValue) {
        return cachedAttributeValue;
    } else {
        // TODO: when not found in cache, generated default values should be used
        MTR_LOG_INFO(
            "_attributeValueDictionaryForAttributePath: could not find cached attribute values for attribute %@", attributePath);
    }

    return nil;
}

- (BOOL)_attributeDataValue:(NSDictionary *)one isEqualToDataValue:(NSDictionary *)theOther
{
    // Attribute data-value dictionary should be all standard containers which
    // means isEqual: comparisons all the way down, making a deep comparison.
    return [one isEqualToDictionary:theOther];
}

- (NSArray *)_getAttributesToReportWithReportedValues:(NSArray<NSDictionary<NSString *, id> *> *)reportedAttributeValues
{
    NSMutableArray * attributesToReport = [NSMutableArray array];

    os_unfair_lock_lock(&self->_lock);
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
    os_unfair_lock_unlock(&self->_lock);

    return attributesToReport;
}

- (NSArray *)_getAttributesToReportWithNewExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedAttributeValues
                                          expirationTime:(NSDate *)expirationTime
{
    NSMutableArray * attributesToReport = [NSMutableArray array];

    // First update cache
    os_unfair_lock_lock(&self->_lock);
    for (NSDictionary<NSString *, id> * attributeReponseValue in expectedAttributeValues) {
        MTRAttributePath * attributePath = attributeReponseValue[MTRAttributePathKey];
        NSDictionary * attributeDataValue = attributeReponseValue[MTRDataKey];

        // check if value is different than cache, and report if needed
        BOOL shouldReportAttribute = NO;

        MTRPair<NSDate *, NSDictionary *> * expectedValue = _expectedValueCache[attributePath];
        if (expectedValue) {
            // first check write cache and purge / update
            if (![self _attributeDataValue:attributeDataValue isEqualToDataValue:expectedValue.second]) {
                shouldReportAttribute = YES;
            }
            _expectedValueCache[attributePath] = [MTRPair pairWithFirst:expirationTime second:expectedValue];
        } else {
            // if new expected value then compare with read cache and report if needed
            if (![self _attributeDataValue:attributeDataValue isEqualToDataValue:_readCache[attributePath]]) {
                shouldReportAttribute = YES;
            }
        }

        if (shouldReportAttribute) {
            [attributesToReport addObject:attributeReponseValue];
        }
    }
    os_unfair_lock_unlock(&self->_lock);

    return attributesToReport;
}

- (void)setExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)values
    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    // since NSTimeInterval is in seconds, convert ms into seconds in double
    NSTimeInterval expectedValueInterval = expectedValueIntervalMs.doubleValue / 1000;
    NSDate * expirationTime = [NSDate dateWithTimeIntervalSinceNow:expectedValueInterval];

    NSArray * attributesToReport = [self _getAttributesToReportWithNewExpectedValues:values expirationTime:expirationTime];

    [self _reportAttributes:attributesToReport];
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
    id _Nullable value = nil;
    NSError * _Nullable error = nil;
    if (apStatus != nullptr) {
        error = [MTRError errorForIMStatus:*apStatus];
    } else if (apData == nullptr) {
        error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
    } else {
        CHIP_ERROR err;
        value = MTRDecodeEventPayload(aEventHeader.mPath, *apData, &err);
        if (err == CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB) {
            // We don't know this event; just skip it.
            return;
        }

        if (err != CHIP_NO_ERROR) {
            value = nil;
            error = [MTRError errorForCHIPErrorCode:err];
        }
    }

    if (mEventReports == nil) {
        // Never got a OnReportBegin?  Not much to do other than tear things down.
        ReportError(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    [mEventReports addObject:[[MTREventReport alloc] initWithPath:aEventHeader.mPath
                                                      eventNumber:@(aEventHeader.mEventNumber)
                                                         priority:@((uint8_t) aEventHeader.mPriorityLevel)
                                                        timestamp:@(aEventHeader.mTimestamp.mValue)
                                                            value:value
                                                            error:error]];
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

    id _Nullable value = MTRDecodeDataValueDictionaryFromCHIPTLV(apData);

    if (value) {
        [mAttributeReports addObject:@ { MTRAttributePathKey : [[MTRAttributePath alloc] initWithPath:aPath], MTRDataKey : value }];
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
