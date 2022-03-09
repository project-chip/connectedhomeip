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

typedef void (^CHIPDeviceResponseHandler)(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error);

extern NSString * const kCHIPTypeKey;
extern NSString * const kCHIPValueKey;
extern NSString * const kCHIPTagKey;
extern NSString * const kCHIPSignedIntegerValueTypeKey;
extern NSString * const kCHIPUnsignedIntegerValueTypeKey;
extern NSString * const kCHIPBooleanValueTypeKey;
extern NSString * const kCHIPUTF8StringValueTypeKey;
extern NSString * const kCHIPOctetStringValueTypeKey;
extern NSString * const kCHIPFloatValueTypeKey;
extern NSString * const kCHIPDoubleValueTypeKey;
extern NSString * const kCHIPNullValueTypeKey;
extern NSString * const kCHIPStructureValueTypeKey;
extern NSString * const kCHIPArrayValueTypeKey;
extern NSString * const kCHIPListValueTypeKey;
extern NSString * const kCHIPEndpointIdKey;
extern NSString * const kCHIPClusterIdKey;
extern NSString * const kCHIPAttributeIdKey;
extern NSString * const kCHIPCommandIdKey;
extern NSString * const kCHIPDataKey;
extern NSString * const kCHIPStatusKey;

@interface CHIPDevice : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Subscribe to receive attribute reports for everything (all endpoints, all
 * clusters, all attributes, all events) on the device.
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
 * subscriptionEstablished block, if not nil, will be called once the
 * subscription is established.  This will be _after_ the first (priming) call
 * to reportHandler.
 *
 * TODO: The "all events" part does not work yet.
 */
- (void)subscribeWithQueue:(dispatch_queue_t)queue
                minInterval:(uint16_t)minInterval
                maxInterval:(uint16_t)maxInterval
              reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
    subscriptionEstablished:(nullable void (^)(void))subscriptionEstablishedHandler;

/**
 * Read attribute in a designated attribute path
 *
 * @param completion  response handler will receive either value or error. value will be an NSArray object with NSDictionary
 * elements. Each NSDictionary will have "endpointId", "clusterId", "attributeId", "status" and "data" keys. "endpointId",
 * "clusterId", "attributeId" and "status" will be mapped to NSNumber objects. "status" with 0 value indicates success and non-zero
 * value indicates failure. "data" key is present only when "status" value is 0. "data" key will be mapped to an NSDictionary
 * object, representing attribute value of the path. NSDictionary representing attribute value will contain "type" and "value" keys.
 *                        "type" will be mapped to "SignedInteger", "UnsignedInteger", "UTF8String", "OctetString", "Float",
 * "Double", "Boolean", "Null", "Structure", "Array" or "List. "value" will be mapped to an NSNumber, NSString, nil or NSArray
 * instance. When "type" is "OctetStriing", "value" will be an NSData object. When "type" is "Structure", "Array" or "List", "value"
 * will be NSArray with NSDictionary elements. Each NSDictionary element will have "tag" and "value" keys. "tag" will be mapped to
 * an NSNumber value. "value" will be mapped to an NSDictionary instance representing any attribute value recursively.
 */
- (void)readAttributeWithEndpointId:(NSUInteger)endpointId
                          clusterId:(NSUInteger)clusterId
                        attributeId:(NSUInteger)attributeId
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(CHIPDeviceResponseHandler)completion;

/**
 * Write to attribute in a designated attribute path
 *
 * @param completion  response handler will receive either value or error. value will be an NSArray object with NSDictionary
 * elements. Each NSDictionary will have "endpointId", "clusterId", "attributeId" and "status" keys. "endpointId", "clusterId",
 * "attributeId" and "status" will be mapped to NSNumber objects. "status" with 0 value indicates success and non-zero value
 * indicates failure.
 */
- (void)writeAttributeWithEndpointId:(NSUInteger)endpointId
                           clusterId:(NSUInteger)clusterId
                         attributeId:(NSUInteger)attributeId
                               value:(id)value
                         clientQueue:(dispatch_queue_t)clientQueue
                          completion:(CHIPDeviceResponseHandler)completion;

/**
 * Invoke a command with a designated command path
 *
 * @param commandFields   command fields object. The object must be an NSDictionary object representing attribute value
 *                      as described in the readAttributeWithEndpointId:clusterId:attributeId:clientQueue:responseHandler: method.
 *                      The attribute must be a Structure, i.e., the NSDictionary "type" key must have the value "Structure".
 *
 * @param completion  response handler will receive either value or error. value will be an NSArray object with NSDictionary
 * elements. Each NSDictionary will have "endpointId", "clusterId", "commandId", "status" and "responseData" keys. "endpointId",
 * "clusterId", "attributeId" and "status" will be mapped to NSNumber objects. "status" with 0 value indicates success and non-zero
 * value indicates failure. "responseData" key will be included only when "status" key has 0 value and there is response data for
 * the command. "responseData" key value will be an NSDictionary object representing attribute value as described in the
 * readAttributeWithEndpointId:clusterId:attributeId:clientQueue:responseHandler: method.
 */
- (void)invokeCommandWithEndpointId:(NSUInteger)endpointId
                          clusterId:(NSUInteger)clusterId
                          commandId:(NSUInteger)commandId
                      commandFields:(id)commandFields
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(CHIPDeviceResponseHandler)completion;

/**
 * Subscribe an attribute in a designated attribute path
 *
 * @param reportHandler   handler for the reports. Note that only the report handler by the last call to this method per the same
 * attribute path will receive reports. Report handler will receive either value or error. value will be an NSDictionary object. The
 * NSDictionary object will have "endpointId", "clusterId", "attributeId" and "value" keys. "endpointId", "clusterId" and
 * "attributeId" will be mapped to NSNumber objects. "value" key value will be an NSDictionary object representing attribute value
 *                      as described in the readAttributeWithEndpointId:clusterId:attributeId:clientQueue:responseHandler: method.
 */
- (void)subscribeAttributeWithEndpointId:(NSUInteger)endpointId
                               clusterId:(NSUInteger)clusterId
                             attributeId:(NSUInteger)attributeId
                             minInterval:(NSUInteger)minInterval
                             maxInterval:(NSUInteger)maxInterval
                             clientQueue:(dispatch_queue_t)clientQueue
                           reportHandler:(void (^)(NSDictionary<NSString *, id> * _Nullable value,
                                             NSError * _Nullable error))reportHandler
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

NS_ASSUME_NONNULL_END

#endif /* CHIP_DEVICE_H */
