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
#import "MTRDeviceController_Internal.h"
#import "MTRDevice_Internal.h"
#import "MTRLogging.h"

#include <app/ConcreteAttributePath.h>

typedef void (^MTRDeviceAttributeReportHandler)(NSArray * _Nonnull);

@interface MTRDevice ()
@property (nonatomic, readonly) os_unfair_lock lock;
@property (nonatomic, strong) dispatch_queue_t queue;

// Read cache is attributePath => NSDictionary of value.
// See MTRDeviceResponseHandler definition for value dictionary details.
@property (nonatomic, strong) NSMutableDictionary<MTRAttributePath *, NSDictionary *> * readCache;

// Expected value cache is attributePath => NSArray of [NSDate of expiration time, NSDictionary of value]
// See MTRDeviceResponseHandler definition for value dictionary details.
@property (nonatomic, strong) NSMutableDictionary<MTRAttributePath *, NSArray *> * expectedValueCache;

// This is a copy of attribute report handlers
//   Dictionary of attributePath => NSArray of [MTRDeviceReportHandler, dispatch_quue_t]
//   AttributePath with all invalid IDs is a valid key for wildcard subscription
//
// TODO: Note this currently supports only 1 subscription per attribute path and 1 wild card subscription. Should look into
// changeing subscription to include subscriber info, so they can be invalidated when subscriber goes away. (delegate, or return a
// subscription token)
@property (nonatomic, strong) NSMutableDictionary<MTRAttributePath *, NSArray *> * attributeReportHandlers;
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
        _attributeReportHandlers = [NSMutableDictionary dictionary];
    }
    return self;
}

+ (instancetype)deviceWithDeviceID:(uint64_t)deviceID deviceController:(MTRDeviceController *)deviceController
{
    return [deviceController deviceForDeviceID:deviceID];
}

- (void)setExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)values
    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    os_unfair_lock_lock(&self->_lock);
    for (NSDictionary<NSString *, id> * valueDictionary in values) {
        MTRAttributePath * attributePath = valueDictionary[MTRAttributePathKey];
        if (!attributePath) {
            continue;
        }
        NSDictionary * attributeValueDictionary = valueDictionary[MTRDataKey];
        if (!attributeValueDictionary) {
            continue;
        }

        // since NSTimeInterval is in seconds, convert ms into seconds in double
        NSTimeInterval expectedValueInterval = expectedValueIntervalMs.doubleValue / 1000;
        NSDate * expirationTime = [NSDate dateWithTimeIntervalSinceNow:expectedValueInterval];

        _expectedValueCache[attributePath] = @[ expirationTime, attributeValueDictionary ];
    }
    os_unfair_lock_unlock(&self->_lock);
}

- (void)subscribeWithQueue:(dispatch_queue_t)queue
                minInterval:(uint16_t)minInterval
                maxInterval:(uint16_t)maxInterval
                     params:(nullable MTRSubscribeParams *)params
             cacheContainer:(MTRAttributeCacheContainer * _Nullable)attributeCacheContainer
     attributeReportHandler:(nullable MTRDeviceReportHandler)attributeReportHandler
         eventReportHandler:(nullable MTRDeviceReportHandler)eventReportHandler
               errorHandler:(void (^)(NSError * error))errorHandler
    subscriptionEstablished:(nullable void (^)(void))subscriptionEstablishedHandler
{
    // save the attribute subscription handler
    MTRAttributePath * wildCardAttributePath = [MTRAttributePath attributePathWithEndpointId:@(chip::kInvalidEndpointId)
                                                                                   clusterId:@(chip::kInvalidClusterId)
                                                                                 attributeId:@(chip::kInvalidAttributeId)];
    os_unfair_lock_lock(&self->_lock);
    _attributeReportHandlers[wildCardAttributePath] = @[ attributeReportHandler, queue ];
    os_unfair_lock_unlock(&self->_lock);

    [_deviceController getBaseDevice:_deviceID
                               queue:_queue
                   completionHandler:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                       [device subscribeWithQueue:self->_queue
                           minInterval:minInterval
                           maxInterval:maxInterval
                           params:params
                           cacheContainer:attributeCacheContainer
                           attributeReportHandler:^(NSArray * _Nonnull values) {
                               // Feed the read cache
                               for (MTRAttributeReport * attributeReport in values) {
                                   NSDictionary * valueDictionary =
                                       [self _attributeValueDictionaryForAttributeReport:attributeReport];
                                   self->_readCache[attributeReport.path] = valueDictionary;
                               }

                               // now forward report
                               attributeReportHandler(values);
                           }
                           eventReportHandler:eventReportHandler
                           errorHandler:^(NSError * _Nonnull error) {
                               // Tear down this subscription internally
                               os_unfair_lock_lock(&self->_lock);
                               self->_attributeReportHandlers[wildCardAttributePath] = nil;
                               os_unfair_lock_unlock(&self->_lock);

                               errorHandler(error);
                           }
                           subscriptionEstablished:subscriptionEstablishedHandler];
                   }];
}

- (NSDictionary<NSString *, id> *)readAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                                                    clusterId:(NSNumber * _Nullable)clusterId
                                                  attributeId:(NSNumber * _Nullable)attributeId
                                                       params:(MTRReadParams * _Nullable)params
{
    // Get the value dictionary ready first
    MTRAttributePath * attributePath = [MTRAttributePath attributePathWithEndpointId:endpointId
                                                                           clusterId:clusterId
                                                                         attributeId:attributeId];
    NSDictionary<NSString *, id> * attributeValueToReturn = [self _attributeValueDictionaryForAttributePath:attributePath];

    // Queue this operation before returning the value

    return attributeValueToReturn;
}

- (void)writeAttributeWithEndpointId:(NSNumber *)endpointId
                           clusterId:(NSNumber *)clusterId
                         attributeId:(NSNumber *)attributeId
                               value:(id)value
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                   timedWriteTimeout:(NSNumber * _Nullable)timeoutMs
{
    // Commit change into expected value cache
    MTRAttributePath * attributePath = [MTRAttributePath attributePathWithEndpointId:endpointId
                                                                           clusterId:clusterId
                                                                         attributeId:attributeId];
    NSDictionary * newExpectedValueDictionary = @{ MTRAttributePathKey : attributePath, MTRDataKey : value };

    // since NSTimeInterval is in seconds, convert ms into seconds in double
    NSTimeInterval expectedValueInterval = expectedValueIntervalMs.doubleValue / 1000;
    NSDate * expirationTime = [NSDate dateWithTimeIntervalSinceNow:expectedValueInterval];

    // always overwrite with the latest value
    // TODO: implement timer to scan and purge expired expected values, and report from read cache if value differs
    _expectedValueCache[attributePath] = @[ expirationTime, newExpectedValueDictionary ];

    // check if the value is the same as the read cache, and report if different
    if (![newExpectedValueDictionary isEqualToDictionary:_readCache[attributePath]]) {
        [self _reportAttributes:@[ attributePath ]];
    }

    // Queue this operation
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

    // Queue this operation
}

#pragma mark - Helper methods

// Get attribute value dictionary for an attribute path from the right cache
- (NSDictionary *)_attributeValueDictionaryForAttributePath:(MTRAttributePath *)attributePath
{
    // First check expected value cache

    // Then check read cache

    // TODO: when not found in cache, generated default values should be used
    MTR_LOG_INFO(
        "_attributeValueDictionaryForAttributePath: could not find cached attribute values for attribute %@", attributePath);

    return nil;
}

// Convert attribute value dictionary (as defined with MTRDeviceResponseHandler) into MTRAttributeReport
- (MTRAttributeReport *)_attributeReportForAttributeValueDictionary:(NSDictionary *)attributeValueDictionary
{
    // Turn value dictionary into an attribute report object

    return nil;
}

// Convert MTRAttributeReport into attribute value dictionary as defined with MTRDeviceResponseHandler
- (NSDictionary *)_attributeValueDictionaryForAttributeReport:(MTRAttributeReport *)attributeReport
{
    // Turn attribute report into the value dictionary

    return nil;
}

// Method to convert the value dictionary (as defined with MTRDeviceResponseHandler) into MTRAttributeReport
- (MTRAttributeReport *)_attributeReportForAttributePath:(MTRAttributePath *)attributePath
{
    NSDictionary * valueDictionary = [self _attributeValueDictionaryForAttributePath:attributePath];

    return [self _attributeReportForAttributeValueDictionary:valueDictionary];
}

// Method to take a list of attribute paths and call saved subscription attribute report handlers
- (void)_reportAttributes:(NSArray<MTRAttributePath *> *)attributePaths
{
    // check if need to build report for wild card subscription
    MTRAttributePath * wildCardAttributePath = [MTRAttributePath attributePathWithEndpointId:@(chip::kInvalidEndpointId)
                                                                                   clusterId:@(chip::kInvalidClusterId)
                                                                                 attributeId:@(chip::kInvalidAttributeId)];
    NSArray * wildCardReportHandlerInfo = _attributeReportHandlers[wildCardAttributePath];
    NSMutableArray * wildCardAttributeReports = nil;
    if (wildCardReportHandlerInfo) {
        wildCardAttributeReports = [NSMutableArray array];
    }

    // first report individual
    for (MTRAttributePath * attributePath in attributePaths) {
        // ignore wild card path from input
        if ([attributePath.endpoint isEqualToNumber:@(chip::kInvalidEndpointId)] ||
            [attributePath.cluster isEqualToNumber:@(chip::kInvalidClusterId)] ||
            [attributePath.attribute isEqualToNumber:@(chip::kInvalidAttributeId)]) {
            MTR_LOG_INFO("_reportAttributes asked to report invalid attribute path %@", attributePath);
            continue;
        }

        NSArray * attributeReportHandlerInfo = _attributeReportHandlers[attributePath];

        // if no subscription exists for this attribute, skip
        if (!attributeReportHandlerInfo && !wildCardReportHandlerInfo) {
            continue;
        }

        MTRDeviceReportHandler reportHandler = [attributeReportHandlerInfo objectAtIndex:0];
        dispatch_queue_t reportHandlerQueue = [attributeReportHandlerInfo objectAtIndex:1];

        MTRAttributeReport * attributeReport = [self _attributeReportForAttributePath:attributePath];

        // if no reports exist, skip
        if (!attributeReport) {
            continue;
        }

        dispatch_async(reportHandlerQueue, ^{
            reportHandler(@[ attributeReport ]);
        });

        // build wild card subscription report if needed
        if (wildCardReportHandlerInfo) {
            [wildCardAttributeReports addObject:attributeReport];
        }
    }

    // report the complete set of values for wild card subscription, if any
    if (wildCardReportHandlerInfo && wildCardAttributeReports.count) {
        MTRDeviceReportHandler reportHandler = [wildCardReportHandlerInfo objectAtIndex:0];
        dispatch_queue_t reportHandlerQueue = [wildCardReportHandlerInfo objectAtIndex:1];
        dispatch_async(reportHandlerQueue, ^{
            reportHandler(wildCardAttributeReports);
        });
    }
}

- (void)connectAndPerformAsync:(MTRDevicePerformAsyncBlock)asyncBlock
{
    [_deviceController getBaseDevice:_deviceID
                               queue:_queue
                   completionHandler:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                       asyncBlock([device internalDevice]);
                   }];
}

@end
