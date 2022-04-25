/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef CHIP_DEVICE_H
#define CHIP_DEVICE_H

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Handler for read attribute response, write attribute response, invoke command response and reports.
 *
 * Handler will receive either values or error. Either one of the parameters will be nil.
 *
 * @param values  Received values are an NSArray object with response-value element as described below.
 *
 *                A response-value is an NSDictionary object with the following key values:
 *
 *                kCHIPAttributePathKey : CHIPAttributePath object. Included for attribute value.
 *                kCHIPCommandPathKey : CHIPCommandPath object. Included for command response.
 *                kCHIPErrorKey : NSError object. Included to indicate an error.
 *                kCHIPDataKey: Data-value NSDictionary object.
 *                              Included when there is data and when there is no error.
 *                              The data-value is described below.
 *
 *                A data-value is an NSDictionary object with the following key values:
 *
 *                kCHIPTypeKey : data type. kCHIPSignedIntegerValueType, kCHIPUnsignedIntegerValueType, kCHIPBooleanValueType,
 *                               kCHIPUTF8StringValueType, kCHIPOctetStringValueType, kCHIPFloatValueType, kCHIPDoubleValueType,
 *                               kCHIPNullValueType, kCHIPStructureValueType or kCHIPArrayValueType.
 *
 *                kCHIPValueKey : data value. Per each data type, data value shall be the following object:
 *
 *                          kCHIPSignedIntegerValueType: NSNumber object.
 *                          kCHIPUnsignedIntegerValueType: NSNumber object.
 *                          kCHIPBooleanValueType: NSNumber object.
 *                          kCHIPUTF8StringValueType: NSString object.
 *                          kCHIPOctetStringValueType: NSData object.
 *                          kCHIPFloatValueType: NSNumber object.
 *                          kCHIPDoubleValueType: NSNumber object.
 *                          kCHIPNullValueType: "value" key will not be included.
 *                          kCHIPStructureValueType: structure-value NSArray object.
 *                                                   See below for the definition of structure-value.
 *                          kCHIPArrayValueType: Array-value NSArray object. See below for the definition of array-value.
 *
 *                A structure-value is an NSArray object with NSDictionary objects as its elements. Each dictionary element will
 *                contain the following key values.
 *
 *                kCHIPContextTagKey : NSNumber object as context tag.
 *                kCHIPDataKey : Data-value NSDictionary object.
 *
 *                An array-value is an NSArray object with NSDictionary objects as its elements. Each dictionary element will
 *                contain the following key values.
 *
 *                kCHIPDataKey : Data-value NSDictionary object.
 */
typedef void (^CHIPDeviceResponseHandler)(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error);

extern NSString * const kCHIPAttributePathKey;
extern NSString * const kCHIPCommandPathKey;
extern NSString * const kCHIPDataKey;
extern NSString * const kCHIPErrorKey;
extern NSString * const kCHIPTypeKey;
extern NSString * const kCHIPValueKey;
extern NSString * const kCHIPContextTagKey;
extern NSString * const kCHIPSignedIntegerValueType;
extern NSString * const kCHIPUnsignedIntegerValueType;
extern NSString * const kCHIPBooleanValueType;
extern NSString * const kCHIPUTF8StringValueType;
extern NSString * const kCHIPOctetStringValueType;
extern NSString * const kCHIPFloatValueType;
extern NSString * const kCHIPDoubleValueType;
extern NSString * const kCHIPNullValueType;
extern NSString * const kCHIPStructureValueType;
extern NSString * const kCHIPArrayValueType;

@class CHIPAttributeCacheContainer;
@class CHIPReadParams;
@class CHIPSubscribeParams;

@interface CHIPDevice : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Subscribe to receive attribute reports for everything (all endpoints, all
 * clusters, all attributes, all events) on the device.
 *
 * A non-nil attribute cache container will cache attribute values, retrievable
 * through the designated attribute cache container.
 *
 * reportHandler will be called any time a data update is available (with a
 * non-nil "value" and nil "error"), or any time there is an error for the
 * entire subscription (with a nil "value" and non-nil "error").  If it's called
 * with an error, that will terminate the subscription.
 *
 * The array passed to reportHandler will contain CHIPAttributeReport
 * instances.  Errors for specific paths, not the whole subscription, will be
 * reported via those objects.
 *
 * reportHandler is not supported over XPC at the moment.
 *
 * subscriptionEstablished block, if not nil, will be called once the
 * subscription is established.  This will be _after_ the first (priming) call
 * to reportHandler.  Note that if the CHIPSubscribeParams are set to
 * automatically resubscribe this can end up being called more than once.
 *
 * TODO: Remove this once the replacement below is adopted
 */
- (void)subscribeWithQueue:(dispatch_queue_t)queue
                minInterval:(uint16_t)minInterval
                maxInterval:(uint16_t)maxInterval
                     params:(nullable CHIPSubscribeParams *)params
             cacheContainer:(CHIPAttributeCacheContainer * _Nullable)attributeCacheContainer
              reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
    subscriptionEstablished:(nullable void (^)(void))subscriptionEstablishedHandler;

/**
 * Subscribe to receive attribute reports for everything (all endpoints, all
 * clusters, all attributes, all events) on the device.
 *
 * A non-nil attribute cache container will cache attribute values, retrievable
 * through the designated attribute cache container.
 *
 * attributeReportHandler will be called any time a data update is available (with a
 * non-nil "value")
 *
 * The array passed to attributeReportHandler will contain CHIPAttributeReport
 * instances.  Errors for specific paths, not the whole subscription, will be
 * reported via those objects.
 *
 * eventReportHandler will be called any time an event is reported (with a
 * non-nil "value")
 *
 * The array passed to eventReportHandler will contain CHIPEventReport
 * instances.  Errors for specific paths, not the whole subscription, will be
 * reported via those objects.
 *
 * errorHandler will be called any time there is an error for the
 * entire subscription (with a non-nil "error"), and terminate the subscription.
 *
 * Both report handlers are not supported over XPC at the moment.
 *
 * subscriptionEstablished block, if not nil, will be called once the
 * subscription is established.  This will be _after_ the first (priming) call
 * to both report handlers.  Note that if the CHIPSubscribeParams are set to
 * automatically resubscribe this can end up being called more than once.
 */
- (void)subscribeWithQueue:(dispatch_queue_t)queue
                minInterval:(uint16_t)minInterval
                maxInterval:(uint16_t)maxInterval
                     params:(nullable CHIPSubscribeParams *)params
             cacheContainer:(CHIPAttributeCacheContainer * _Nullable)attributeCacheContainer
     attributeReportHandler:(void (^)(NSArray * value))attributeReportHandler
         eventReportHandler:(void (^)(NSArray * value))eventReportHandler
               errorHandler:(void (^)(NSError * error))errorHandler
    subscriptionEstablished:(nullable void (^)(void))subscriptionEstablishedHandler;

/**
 * Read attribute in a designated attribute path
 */
- (void)readAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                          clusterId:(NSNumber * _Nullable)clusterId
                        attributeId:(NSNumber * _Nullable)attributeId
                             params:(CHIPReadParams * _Nullable)params
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(CHIPDeviceResponseHandler)completion;

/**
 * Write to attribute in a designated attribute path
 *
 * @param value       A data-value NSDictionary object as described in
 *                    CHIPDeviceResponseHandler.
 *
 * @param timeoutMs   timeout in milliseconds for timed write, or nil.
 *
 * @param completion  response handler will receive either values or error.
 *
 *                    Received values are an NSArray object with response-value element as described in
 *                    readAttributeWithEndpointId:clusterId:attributeId:clientQueue:completion:.
 */
- (void)writeAttributeWithEndpointId:(NSNumber *)endpointId
                           clusterId:(NSNumber *)clusterId
                         attributeId:(NSNumber *)attributeId
                               value:(id)value
                   timedWriteTimeout:(NSNumber * _Nullable)timeoutMs
                         clientQueue:(dispatch_queue_t)clientQueue
                          completion:(CHIPDeviceResponseHandler)completion;

/**
 * Invoke a command with a designated command path
 *
 * @param commandFields   command fields object. The object must be a data-value NSDictionary object
 *                      as described in the CHIPDeviceResponseHandler.
 *                      The attribute must be a Structure, i.e.,
 *                      the NSDictionary kCHIPTypeKey key must have the value kCHIPStructureValueType.
 *
 * @param timeoutMs   timeout in milliseconds for timed invoke, or nil.
 *
 * @param completion  response handler will receive either values or error.
 */
- (void)invokeCommandWithEndpointId:(NSNumber *)endpointId
                          clusterId:(NSNumber *)clusterId
                          commandId:(NSNumber *)commandId
                      commandFields:(id)commandFields
                 timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(CHIPDeviceResponseHandler)completion;

/**
 * Subscribe an attribute in a designated attribute path
 */
- (void)subscribeAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                               clusterId:(NSNumber * _Nullable)clusterId
                             attributeId:(NSNumber * _Nullable)attributeId
                             minInterval:(NSNumber *)minInterval
                             maxInterval:(NSNumber *)maxInterval
                                  params:(CHIPSubscribeParams * _Nullable)params
                             clientQueue:(dispatch_queue_t)clientQueue
                           reportHandler:(CHIPDeviceResponseHandler)reportHandler
                 subscriptionEstablished:(nullable void (^)(void))subscriptionEstablishedHandler;

/**
 * Deregister all local report handlers for a remote device
 *
 * This method is applicable only for a remote device. For a local device, the stack has to be shutdown to stop report handlers.
 * There could be multiple clients accessing a node through a remote controller object and hence it is not appropriate
 * for one of those clients to shut down the entire stack to stop receiving reports.
 */
- (void)deregisterReportHandlersWithClientQueue:(dispatch_queue_t)clientQueue completion:(void (^)(void))completion;

@end

@interface CHIPAttributePath : NSObject
@property (nonatomic, readonly, strong, nonnull) NSNumber * endpoint;
@property (nonatomic, readonly, strong, nonnull) NSNumber * cluster;
@property (nonatomic, readonly, strong, nonnull) NSNumber * attribute;

+ (instancetype)attributePathWithEndpointId:(NSNumber *)endpoint
                                  clusterId:(NSNumber *)clusterId
                                attributeId:(NSNumber *)attributeId;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

@interface CHIPEventPath : NSObject
@property (nonatomic, readonly, strong, nonnull) NSNumber * endpoint;
@property (nonatomic, readonly, strong, nonnull) NSNumber * cluster;
@property (nonatomic, readonly, strong, nonnull) NSNumber * event;

+ (instancetype)eventPathWithEndpointId:(NSNumber *)endpoint clusterId:(NSNumber *)clusterId eventId:(NSNumber *)eventId;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

@interface CHIPCommandPath : NSObject
@property (nonatomic, readonly, strong, nonnull) NSNumber * endpoint;
@property (nonatomic, readonly, strong, nonnull) NSNumber * cluster;
@property (nonatomic, readonly, strong, nonnull) NSNumber * command;

+ (instancetype)commandPathWithEndpointId:(NSNumber *)endpoint clusterId:(NSNumber *)clusterId commandId:(NSNumber *)commandId;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

@interface CHIPAttributeReport : NSObject
@property (nonatomic, readonly, strong, nonnull) CHIPAttributePath * path;
// value is nullable because nullable attributes can have nil as value.
@property (nonatomic, readonly, strong, nullable) id value;
// If this specific path resulted in an error, the error (in the
// MatterInteractionErrorDomain or CHIPErrorDomain) that corresponds to this
// path.
@property (nonatomic, readonly, strong, nullable) NSError * error;
@end

@interface CHIPEventReport : NSObject
@property (nonatomic, readonly, strong, nonnull) CHIPEventPath * path;
@property (nonatomic, readonly, strong, nonnull) NSNumber * eventNumber; // chip::EventNumber type (uint64_t)
@property (nonatomic, readonly, strong, nonnull) NSNumber * priority; // chip::app::PriorityLevel type (uint8_t)
@property (nonatomic, readonly, strong, nonnull) NSNumber * timestamp; // chip::app::Timestamp.mValue type (uint64_t)
@property (nonatomic, readonly, strong, nullable) id value;
// If this specific path resulted in an error, the error (in the
// MatterInteractionErrorDomain or CHIPErrorDomain) that corresponds to this
// path.
@property (nonatomic, readonly, strong, nullable) NSError * error;
@end

NS_ASSUME_NONNULL_END

#endif /* CHIP_DEVICE_H */
