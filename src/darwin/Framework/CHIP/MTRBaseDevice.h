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

#import <Matter/MTRCluster.h>

@class MTRSetupPayload;

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
 *                MTREventPathKey : MTREventPath object. Included for event value.
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

/**
 * Handler for -subscribeWithQueue: attribute and event reports
 *
 * @param values This array contains MTRAttributeReport objects for attribute reports, and MTREventReport objects for event reports
 */
typedef void (^MTRDeviceReportHandler)(NSArray * values);
typedef void (^MTRDeviceErrorHandler)(NSError * error);

/**
 * Handler for subscribeWithQueue: resubscription scheduling notifications.
 * This will be called when subscription loss is detected.
 *
 * @param error An error indicating the reason the subscription has been lost.
 * @param resubscriptionDelay A delay, in milliseconds, before the next
 *        automatic resubscription will be attempted.
 */
typedef void (^MTRDeviceResubscriptionScheduledHandler)(NSError * error, NSNumber * resubscriptionDelay);

/**
 * Handler for openCommissioningWindow.
 */
typedef void (^MTRDeviceOpenCommissioningWindowHandler)(MTRSetupPayload * _Nullable payload, NSError * _Nullable error);

extern NSString * const MTRAttributePathKey;
extern NSString * const MTRCommandPathKey;
extern NSString * const MTREventPathKey;
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

@class MTRClusterStateCacheContainer;
@class MTRReadParams;
@class MTRSubscribeParams;
@class MTRDeviceController;

@interface MTRBaseDevice : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Create a device object with the given node id and controller.  This
 * will always succeed, even if there is no such node id on the controller's
 * fabric, but attempts to actually use the MTRBaseDevice will fail
 * (asynchronously) in that case.
 */
+ (instancetype)deviceWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller;

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
 * errorHandler will be called any time there is an error for the entire
 * subscription (with a non-nil "error"), and terminate the subscription.  This
 * will generally not be invoked if auto-resubscription is enabled, unless there
 * is a fatal error during a resubscription attempt.
 *
 * Both report handlers are not supported over XPC at the moment.
 *
 * The subscriptionEstablished block, if not nil, will be called once the
 * subscription is established.  This will be _after_ the first (priming) call
 * to both report handlers.  Note that if the MTRSubscribeParams are set to
 * automatically resubscribe this can end up being called more than once.
 *
 * The resubscriptionScheduled block, if not nil, will be called if
 * auto-resubscription is enabled, subscription loss is detected, and a
 * resubscription is scheduled.  This can be called multiple times in a row
 * without an intervening subscriptionEstablished call if the resubscription
 * attempts fail.
 */
- (void)subscribeWithQueue:(dispatch_queue_t)queue
                        params:(MTRSubscribeParams *)params
    clusterStateCacheContainer:(MTRClusterStateCacheContainer * _Nullable)clusterStateCacheContainer
        attributeReportHandler:(MTRDeviceReportHandler _Nullable)attributeReportHandler
            eventReportHandler:(MTRDeviceReportHandler _Nullable)eventReportHandler
                  errorHandler:(MTRDeviceErrorHandler)errorHandler
       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
       resubscriptionScheduled:(MTRDeviceResubscriptionScheduledHandler _Nullable)resubscriptionScheduled;

/**
 * Reads the given attribute path from the device.
 *
 * nil values for endpointID, clusterID, attributeID indicate wildcards
 * (e.g. nil attributeID means "read all the attributes from the endpoint(s) and
 * cluster(s) that match endpointID/clusterID").
 *
 * A non-nil attributeID along with a nil clusterID will only succeed if the
 * attribute ID is for a global attribute that applies to all clusters.
 */
- (void)readAttributePathWithEndpointID:(NSNumber * _Nullable)endpointID
                              clusterID:(NSNumber * _Nullable)clusterID
                            attributeID:(NSNumber * _Nullable)attributeID
                                 params:(MTRReadParams * _Nullable)params
                                  queue:(dispatch_queue_t)queue
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
 *                    Received values are documented in the definition of
 *                    MTRDeviceResponseHandler.
 */
- (void)writeAttributeWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                         attributeID:(NSNumber *)attributeID
                               value:(id)value
                   timedWriteTimeout:(NSNumber * _Nullable)timeoutMs
                               queue:(dispatch_queue_t)queue
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
- (void)invokeCommandWithEndpointID:(NSNumber *)endpointID
                          clusterID:(NSNumber *)clusterID
                          commandID:(NSNumber *)commandID
                      commandFields:(id)commandFields
                 timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
                              queue:(dispatch_queue_t)queue
                         completion:(MTRDeviceResponseHandler)completion;

/**
 * Subscribes to the given attribute path on the device.
 *
 * nil values for endpointID, clusterID, attributeID indicate wildcards
 * (e.g. nil attributeID means "read all the attributes from the endpoint(s) and
 * cluster(s) that match endpointID/clusterID").
 *
 * A non-nil attributeID along with a nil clusterID will only succeed if the
 * attribute ID is for a global attribute that applies to all clusters.
 */
- (void)subscribeAttributePathWithEndpointID:(NSNumber * _Nullable)endpointID
                                   clusterID:(NSNumber * _Nullable)clusterID
                                 attributeID:(NSNumber * _Nullable)attributeID
                                      params:(MTRSubscribeParams *)params
                                       queue:(dispatch_queue_t)queue
                               reportHandler:(MTRDeviceResponseHandler)reportHandler
                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished;

/**
 * Deregister all local report handlers for a remote device
 *
 * This method is applicable only for a remote device. For a local device, the stack has to be shutdown to stop report handlers.
 * There could be multiple clients accessing a node through a remote controller object and hence it is not appropriate
 * for one of those clients to shut down the entire stack to stop receiving reports.
 */
- (void)deregisterReportHandlersWithQueue:(dispatch_queue_t)queue completion:(dispatch_block_t)completion;

/**
 * Open a commissioning window on the device.
 *
 * On success, completion will be called with the MTRSetupPayload that
 * can be used to commission the device.
 *
 * @param setupPasscode The setup passcode to use for the commissioning window.
 *                      See MTRSetupPayload's generateRandomSetupPasscode for
 *                      generating a valid random passcode.
 * @param discriminator The discriminator to use for the commissionable
 *                      advertisement.
 * @param duration      Duration, in seconds, during which the commissioning
 *                      window will be open.
 */
- (void)openCommissioningWindowWithSetupPasscode:(NSNumber *)setupPasscode
                                   discriminator:(NSNumber *)discriminator
                                        duration:(NSNumber *)duration
                                           queue:(dispatch_queue_t)queue
                                      completion:(MTRDeviceOpenCommissioningWindowHandler)completion;

@end

/**
 * A path indicating a specific cluster on a device (i.e. without any
 * wildcards).
 */
@interface MTRClusterPath : NSObject <NSCopying>
@property (nonatomic, readonly, copy) NSNumber * endpoint;
@property (nonatomic, readonly, copy) NSNumber * cluster;

+ (instancetype)clusterPathWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

/**
 * A path indicating a specific attribute on a device (i.e. without any
 * wildcards).
 */
@interface MTRAttributePath : MTRClusterPath <NSCopying>
@property (nonatomic, readonly, copy) NSNumber * attribute;

+ (instancetype)attributePathWithEndpointID:(NSNumber *)endpointID
                                  clusterID:(NSNumber *)clusterID
                                attributeID:(NSNumber *)attributeID;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

/**
 * A path indicating a specific event that can be emitted on a device
 * (i.e. without any wildcards).  There can be multiple instances of actual
 * events for a given event path.
 */
@interface MTREventPath : MTRClusterPath <NSCopying>
@property (nonatomic, readonly, copy) NSNumber * event;

+ (instancetype)eventPathWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID eventID:(NSNumber *)eventID;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

/**
 * A path indicating a specific command on a device (i.e. without any
 * wildcards).
 */
@interface MTRCommandPath : MTRClusterPath <NSCopying>
@property (nonatomic, readonly, copy) NSNumber * command;

+ (instancetype)commandPathWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID commandID:(NSNumber *)commandID;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

@interface MTRAttributeReport : NSObject
@property (nonatomic, readonly, copy) MTRAttributePath * path;
// value is nullable because nullable attributes can have nil as value.
@property (nonatomic, readonly, copy, nullable) id value;
// If this specific path resulted in an error, the error (in the
// MTRInteractionErrorDomain or MTRErrorDomain) that corresponds to this
// path.
@property (nonatomic, readonly, copy, nullable) NSError * error;
@end

@interface MTREventReport : NSObject
@property (nonatomic, readonly, copy) MTREventPath * path;
@property (nonatomic, readonly, copy) NSNumber * eventNumber; // EventNumber type (uint64_t)
@property (nonatomic, readonly, copy) NSNumber * priority; // PriorityLevel type (uint8_t)
@property (nonatomic, readonly, copy) NSNumber * timestamp; // Timestamp type (uint64_t)
// An instance of one of the event payload interfaces.
@property (nonatomic, readonly, copy) id value;

// If this specific path resulted in an error, the error (in the
// MTRInteractionErrorDomain or MTRErrorDomain) that corresponds to this
// path.
@property (nonatomic, readonly, copy, nullable) NSError * error;
@end

NS_ASSUME_NONNULL_END
