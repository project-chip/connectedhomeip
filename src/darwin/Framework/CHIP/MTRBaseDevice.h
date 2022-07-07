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
 *                MTRAttributePathKey : MTRAttributePath object. Included for attribute value.
 *                MTRCommandPathKey : MTRCommandPath object. Included for command response.
 *                MTRErrorKey : NSError object. Included to indicate an error.
 *                MTRDataKey: Data-value NSDictionary object.
 *                              Included when there is data and when there is no error.
 *                              The data-value is described below.
 *
 *                A data-value is an NSDictionary object with the following key values:
 *
 *                MTRTypeKey : data type. MTRSignedIntegerValueType, MTRUnsignedIntegerValueType, MTRBooleanValueType,
 *                               MTRUTF8StringValueType, MTROctetStringValueType, MTRFloatValueType, MTRDoubleValueType,
 *                               MTRNullValueType, MTRStructureValueType or MTRArrayValueType.
 *
 *                MTRValueKey : data value. Per each data type, data value shall be the following object:
 *
 *                          MTRSignedIntegerValueType: NSNumber object.
 *                          MTRUnsignedIntegerValueType: NSNumber object.
 *                          MTRBooleanValueType: NSNumber object.
 *                          MTRUTF8StringValueType: NSString object.
 *                          MTROctetStringValueType: NSData object.
 *                          MTRFloatValueType: NSNumber object.
 *                          MTRDoubleValueType: NSNumber object.
 *                          MTRNullValueType: "value" key will not be included.
 *                          MTRStructureValueType: structure-value NSArray object.
 *                                                   See below for the definition of structure-value.
 *                          MTRArrayValueType: Array-value NSArray object. See below for the definition of array-value.
 *
 *                A structure-value is an NSArray object with NSDictionary objects as its elements. Each dictionary element will
 *                contain the following key values.
 *
 *                MTRContextTagKey : NSNumber object as context tag.
 *                MTRDataKey : Data-value NSDictionary object.
 *
 *                An array-value is an NSArray object with NSDictionary objects as its elements. Each dictionary element will
 *                contain the following key values.
 *
 *                MTRDataKey : Data-value NSDictionary object.
 */
typedef void (^MTRDeviceResponseHandler)(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error);

extern NSString * const MTRAttributePathKey;
extern NSString * const MTRCommandPathKey;
extern NSString * const MTRDataKey;
extern NSString * const MTRErrorKey;
extern NSString * const MTRTypeKey;
extern NSString * const MTRValueKey;
extern NSString * const MTRContextTagKey;
extern NSString * const MTRSignedIntegerValueType;
extern NSString * const MTRUnsignedIntegerValueType;
extern NSString * const MTRBooleanValueType;
extern NSString * const MTRUTF8StringValueType;
extern NSString * const MTROctetStringValueType;
extern NSString * const MTRFloatValueType;
extern NSString * const MTRDoubleValueType;
extern NSString * const MTRNullValueType;
extern NSString * const MTRStructureValueType;
extern NSString * const MTRArrayValueType;

@class MTRAttributeCacheContainer;
@class MTRReadParams;
@class MTRSubscribeParams;

@interface MTRBaseDevice : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

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
 * The array passed to attributeReportHandler will contain MTRAttributeReport
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
 * to both report handlers.  Note that if the MTRSubscribeParams are set to
 * automatically resubscribe this can end up being called more than once.
 */
- (void)subscribeWithQueue:(dispatch_queue_t)queue
                minInterval:(uint16_t)minInterval
                maxInterval:(uint16_t)maxInterval
                     params:(nullable MTRSubscribeParams *)params
             cacheContainer:(MTRAttributeCacheContainer * _Nullable)attributeCacheContainer
     attributeReportHandler:(nullable void (^)(NSArray * value))attributeReportHandler
         eventReportHandler:(nullable void (^)(NSArray * value))eventReportHandler
               errorHandler:(void (^)(NSError * error))errorHandler
    subscriptionEstablished:(nullable void (^)(void))subscriptionEstablishedHandler;

/**
 * Read attribute in a designated attribute path
 */
- (void)readAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                          clusterId:(NSNumber * _Nullable)clusterId
                        attributeId:(NSNumber * _Nullable)attributeId
                             params:(MTRReadParams * _Nullable)params
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion;

/**
 * Write to attribute in a designated attribute path
 *
 * @param value       A data-value NSDictionary object as described in
 *                    MTRDeviceResponseHandler.
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
                          completion:(MTRDeviceResponseHandler)completion;

/**
 * Invoke a command with a designated command path
 *
 * @param commandFields   command fields object. The object must be a data-value NSDictionary object
 *                      as described in the MTRDeviceResponseHandler.
 *                      The attribute must be a Structure, i.e.,
 *                      the NSDictionary MTRTypeKey key must have the value MTRStructureValueType.
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
                         completion:(MTRDeviceResponseHandler)completion;

/**
 * Subscribe an attribute in a designated attribute path
 */
- (void)subscribeAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                               clusterId:(NSNumber * _Nullable)clusterId
                             attributeId:(NSNumber * _Nullable)attributeId
                             minInterval:(NSNumber *)minInterval
                             maxInterval:(NSNumber *)maxInterval
                                  params:(MTRSubscribeParams * _Nullable)params
                             clientQueue:(dispatch_queue_t)clientQueue
                           reportHandler:(MTRDeviceResponseHandler)reportHandler
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

@interface MTRAttributePath : NSObject
@property (nonatomic, readonly, strong, nonnull) NSNumber * endpoint;
@property (nonatomic, readonly, strong, nonnull) NSNumber * cluster;
@property (nonatomic, readonly, strong, nonnull) NSNumber * attribute;

+ (instancetype)attributePathWithEndpointId:(NSNumber *)endpoint
                                  clusterId:(NSNumber *)clusterId
                                attributeId:(NSNumber *)attributeId;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

@interface MTREventPath : NSObject
@property (nonatomic, readonly, strong, nonnull) NSNumber * endpoint;
@property (nonatomic, readonly, strong, nonnull) NSNumber * cluster;
@property (nonatomic, readonly, strong, nonnull) NSNumber * event;

+ (instancetype)eventPathWithEndpointId:(NSNumber *)endpoint clusterId:(NSNumber *)clusterId eventId:(NSNumber *)eventId;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

@interface MTRCommandPath : NSObject
@property (nonatomic, readonly, strong, nonnull) NSNumber * endpoint;
@property (nonatomic, readonly, strong, nonnull) NSNumber * cluster;
@property (nonatomic, readonly, strong, nonnull) NSNumber * command;

+ (instancetype)commandPathWithEndpointId:(NSNumber *)endpoint clusterId:(NSNumber *)clusterId commandId:(NSNumber *)commandId;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

@interface MTRAttributeReport : NSObject
@property (nonatomic, readonly, strong, nonnull) MTRAttributePath * path;
// value is nullable because nullable attributes can have nil as value.
@property (nonatomic, readonly, strong, nullable) id value;
// If this specific path resulted in an error, the error (in the
// MTRInteractionErrorDomain or MTRErrorDomain) that corresponds to this
// path.
@property (nonatomic, readonly, strong, nullable) NSError * error;
@end

@interface MTREventReport : NSObject
@property (nonatomic, readonly, strong, nonnull) MTREventPath * path;
@property (nonatomic, readonly, strong, nonnull) NSNumber * eventNumber; // chip::EventNumber type (uint64_t)
@property (nonatomic, readonly, strong, nonnull) NSNumber * priority; // chip::app::PriorityLevel type (uint8_t)
@property (nonatomic, readonly, strong, nonnull) NSNumber * timestamp; // chip::app::Timestamp.mValue type (uint64_t)
@property (nonatomic, readonly, strong, nullable) id value;
// If this specific path resulted in an error, the error (in the
// MTRInteractionErrorDomain or MTRErrorDomain) that corresponds to this
// path.
@property (nonatomic, readonly, strong, nullable) NSError * error;
@end

NS_ASSUME_NONNULL_END
