/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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
#import <Matter/MTRDefines.h>

@class MTRSetupPayload;
@class MTRDeviceController;

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
 *                MTREventNumberKey : NSNumber-wrapped uint64_t value. Monotonically increasing, and consecutive event reports
 *                                    should have consecutive numbers unless device reboots, or if events are lost.
 *                                    Only present when both MTREventPathKey and MTRDataKey are present.
 *                MTREventPriorityKey : NSNumber-wrapped MTREventPriority value.
 *                                      Only present when both MTREventPathKey and MTRDataKey are present.
 *                MTREventTimeTypeKey : NSNumber-wrapped MTREventTimeType value.
 *                                      Only present when both MTREventPathKey and MTRDataKey are present.
 *                MTREventSystemUpTimeKey : NSNumber-wrapped NSTimeInterval value.
 *                                          Only present when MTREventTimeTypeKey is MTREventTimeTypeSystemUpTime.
 *                MTREventTimestampDateKey : NSDate object.
 *                                           Only present when MTREventTimeTypeKey is MTREventTimeTypeTimestampDate.
 *
 *                Only one of MTREventTimestampDateKey and MTREventSystemUpTimeKey will be present, depending on the value for
 *                MTREventTimeTypeKey.
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
 * Handler for openCommissioningWindowWithSetupPasscode.
 */
API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2))
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
extern NSString * const MTREventNumberKey API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
extern NSString * const MTREventPriorityKey API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
extern NSString * const MTREventTimeTypeKey API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
extern NSString * const MTREventSystemUpTimeKey API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
extern NSString * const MTREventTimestampDateKey API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));

@class MTRClusterStateCacheContainer;
@class MTRAttributeCacheContainer;
@class MTRReadParams;
@class MTRSubscribeParams;

typedef NS_ENUM(uint8_t, MTRTransportType) {
    MTRTransportTypeUndefined = 0,
    MTRTransportTypeUDP,
    MTRTransportTypeBLE,
    MTRTransportTypeTCP,
} API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * A path indicating an attribute being requested (for read or subscribe).
 *
 * nil is used to represent wildcards.
 */
MTR_NEWLY_AVAILABLE
@interface MTRAttributeRequestPath : NSObject <NSCopying>
@property (nonatomic, readonly, copy, nullable) NSNumber * endpoint;
@property (nonatomic, readonly, copy, nullable) NSNumber * cluster;
@property (nonatomic, readonly, copy, nullable) NSNumber * attribute;

+ (MTRAttributeRequestPath *)requestPathWithEndpointID:(NSNumber * _Nullable)endpointID
                                             clusterID:(NSNumber * _Nullable)clusterID
                                           attributeID:(NSNumber * _Nullable)attributeID MTR_NEWLY_AVAILABLE;
@end

/**
 * A path indicating an event being requested (for read or subscribe).
 *
 * nil is used to represent wildcards.
 */
MTR_NEWLY_AVAILABLE
@interface MTREventRequestPath : NSObject <NSCopying>
@property (nonatomic, readonly, copy, nullable) NSNumber * endpoint;
@property (nonatomic, readonly, copy, nullable) NSNumber * cluster;
@property (nonatomic, readonly, copy, nullable) NSNumber * event;

+ (MTREventRequestPath *)requestPathWithEndpointID:(NSNumber * _Nullable)endpointID
                                         clusterID:(NSNumber * _Nullable)clusterID
                                           eventID:(NSNumber * _Nullable)eventID MTR_NEWLY_AVAILABLE;
@end

@interface MTRBaseDevice : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Create a device object with the given node id and controller.  This
 * will always succeed, even if there is no such node id on the controller's
 * fabric, but attempts to actually use the MTRBaseDevice will fail
 * (asynchronously) in that case.
 */
+ (MTRBaseDevice *)deviceWithNodeID:(NSNumber *)nodeID
                         controller:(MTRDeviceController *)controller
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * The transport used by the current session with this device, or
 * `MTRTransportTypeUndefined` if no session is currently active.
 */
@property (readonly) MTRTransportType sessionTransportType API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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
 * The array passed to eventReportHandler will contain MTREventReport
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
       resubscriptionScheduled:(MTRDeviceResubscriptionScheduledHandler _Nullable)resubscriptionScheduled
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Reads attributes from the device.
 *
 * Nil values for endpointID, clusterID, attributeID indicate wildcards
 * (e.g. nil attributeID means "read all the attributes from the endpoint(s) and
 * cluster(s) that match endpointID/clusterID").
 *
 * If all of endpointID, clusterID, attributeID are non-nil, a single
 * attribute will be read.
 *
 * If all of endpointID, clusterID, attributeID are nil, all attributes on the
 * device will be read.
 *
 * A non-nil attributeID along with a nil clusterID will only succeed if the
 * attribute ID is for a global attribute that applies to all clusters.
 *
 * The completion will be called with an error if the entire read interaction fails.
 * Otherwise it will be called with values, which may be empty (e.g. if no paths
 * matched the wildcard) or may include per-path errors if particular paths
 * failed.
 */
- (void)readAttributesWithEndpointID:(NSNumber * _Nullable)endpointID
                           clusterID:(NSNumber * _Nullable)clusterID
                         attributeID:(NSNumber * _Nullable)attributeID
                              params:(MTRReadParams * _Nullable)params
                               queue:(dispatch_queue_t)queue
                          completion:(MTRDeviceResponseHandler)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Reads multiple attribute or event paths from the device.
 *
 * Nil is treated as an empty array for attributePaths and eventPaths.
 *
 * Lists of attribute and event paths to read can be provided via attributePaths and eventPaths.
 *
 * The completion will be called with an error if the entire read interaction fails. Otherwise it
 * will be called with an array of values. This array may be empty (e.g. if no paths matched the
 * wildcard paths passed in, or if empty lists of paths were passed in) or may include per-path
 * errors if particular paths failed.
 *
 * If the sum of the lengths of attributePaths and eventPaths exceeds 9, the read may fail due to the device not supporting that
 * many read paths.
 */
- (void)readAttributePaths:(NSArray<MTRAttributeRequestPath *> * _Nullable)attributePaths
                eventPaths:(NSArray<MTREventRequestPath *> * _Nullable)eventPaths
                    params:(MTRReadParams * _Nullable)params
                     queue:(dispatch_queue_t)queue
                completion:(MTRDeviceResponseHandler)completion MTR_NEWLY_AVAILABLE;

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
 *                    A path-specific error status will get turned into an error
 *                    passed to the completion, so values will only be passed in
 *                    when the write succeeds.  In that case, values will have
 *                    the format documented in the definition of
 *                    MTRDeviceResponseHandler and will be an array with a single element
 *                    which is a dictionary that has a MTRAttributePathKey entry in it, whose value
 *                    is the attribute path that was successfully written to.
 */
- (void)writeAttributeWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                         attributeID:(NSNumber *)attributeID
                               value:(id)value
                   timedWriteTimeout:(NSNumber * _Nullable)timeoutMs
                               queue:(dispatch_queue_t)queue
                          completion:(MTRDeviceResponseHandler)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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
 * @param completion  response handler will receive either values or error.  A
 *                    path-specific error status from the command invocation
 *                    will result in an error being passed to the completion, so
 *                    values will only be passed in when the command succeeds.
 */
- (void)invokeCommandWithEndpointID:(NSNumber *)endpointID
                          clusterID:(NSNumber *)clusterID
                          commandID:(NSNumber *)commandID
                      commandFields:(id)commandFields
                 timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
                              queue:(dispatch_queue_t)queue
                         completion:(MTRDeviceResponseHandler)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Subscribes to the specified attributes on the device.
 *
 * Nil values for endpointID, clusterID, attributeID indicate wildcards
 * (e.g. nil attributeID means "subscribe to all the attributes from the
 * endpoint(s) and cluster(s) that match endpointID/clusterID").
 *
 * If all of endpointID, clusterID, attributeID are non-nil, a single attribute
 * will be subscribed to.
 *
 * If all of endpointID, clusterID, attributeID are nil, all attributes on the
 * device will be subscribed to.
 *
 * A non-nil attributeID along with a nil clusterID will only succeed if the
 * attribute ID is for a global attribute that applies to all clusters.
 *
 * The reportHandler will be called with an error if the subscription fails
 * entirely.
 *
 * The reportHandler will be called with arrays of response-value dictionaries
 * (which may be data or errors) as path-specific data is received.
 *
 * subscriptionEstablished will be called when the subscription is first
 * successfully established (after the initial set of data reports has been
 * delivered to reportHandler).  If params allow automatic resubscription, it
 * will be called any time resubscription succeeds.
 */
- (void)subscribeToAttributesWithEndpointID:(NSNumber * _Nullable)endpointID
                                  clusterID:(NSNumber * _Nullable)clusterID
                                attributeID:(NSNumber * _Nullable)attributeID
                                     params:(MTRSubscribeParams * _Nullable)params
                                      queue:(dispatch_queue_t)queue
                              reportHandler:(MTRDeviceResponseHandler)reportHandler
                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Subscribes to multiple attribute or event paths.
 *
 * Nil is treated as an empty array for attributePaths and eventPaths.
 *
 * Lists of attribute and event paths to subscribe to can be provided via attributePaths and eventPaths.
 *
 * The reportHandler will be called with an error if the subscription fails
 * entirely (including when both attributePaths and eventPaths are empty).
 *
 * The reportHandler will be called with arrays of response-value dictionaries
 * (which may be data or errors) as path-specific data is received.
 *
 * subscriptionEstablished will be called when the subscription is first
 * successfully established (after the initial set of data reports has been
 * delivered to reportHandler).  If params allow automatic resubscription, it
 * will be called any time resubscription succeeds.
 *
 * resubscriptionScheduled will be called if subscription drop is detected and
 * params allow automatic resubscription.
 *
 * If the sum of the lengths of attributePaths and eventPaths exceeds 3, the subscribe may fail due to the device not supporting
 * that many paths for a subscription.
 */
- (void)subscribeToAttributePaths:(NSArray<MTRAttributeRequestPath *> * _Nullable)attributePaths
                       eventPaths:(NSArray<MTREventRequestPath *> * _Nullable)eventPaths
                           params:(MTRSubscribeParams * _Nullable)params
                            queue:(dispatch_queue_t)queue
                    reportHandler:(MTRDeviceResponseHandler)reportHandler
          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
          resubscriptionScheduled:(MTRDeviceResubscriptionScheduledHandler _Nullable)resubscriptionScheduled MTR_NEWLY_AVAILABLE;

/**
 * Deregister all local report handlers for a remote device
 *
 * This method is applicable only for a remote device. For a local device, the stack has to be shutdown to stop report handlers.
 * There could be multiple clients accessing a node through a remote controller object and hence it is not appropriate
 * for one of those clients to shut down the entire stack to stop receiving reports.
 */
- (void)deregisterReportHandlersWithQueue:(dispatch_queue_t)queue
                               completion:(dispatch_block_t)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Open a commissioning window on the device.
 *
 * On success, completion will be called on queue with the MTRSetupPayload that
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
                                      completion:(MTRDeviceOpenCommissioningWindowHandler)completion
    API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2));

/**
 * Open a commissioning window on the device, using a random setup passcode.
 *
 * On success, completion will be called on queue with the MTRSetupPayload that
 * can be used to commission the device.
 *
 * @param discriminator The discriminator to use for the commissionable
 *                      advertisement.
 * @param duration      Duration, in seconds, during which the commissioning
 *                      window will be open.
 */
- (void)openCommissioningWindowWithDiscriminator:(NSNumber *)discriminator
                                        duration:(NSNumber *)duration
                                           queue:(dispatch_queue_t)queue
                                      completion:(MTRDeviceOpenCommissioningWindowHandler)completion MTR_NEWLY_AVAILABLE;

/**
 * Reads events from the device.
 *
 * Nil values for endpointID, clusterID, eventID indicate wildcards
 * (e.g. nil eventID means "read all the events from the endpoint(s) and
 * cluster(s) that match endpointID/clusterID").
 *
 * If all of endpointID, clusterID, eventID are non-nil, all the matching instances of a single
 * event will be read.
 *
 * If all of endpointID, clusterID, eventID are nil, all events on the
 * device will be read.
 *
 * The completion will be called with an error if the entire read interaction fails.
 * Otherwise it will be called with values, which may be empty (e.g. if no paths
 * matched the wildcard) or may include per-path errors if particular paths
 * failed.
 */

- (void)readEventsWithEndpointID:(NSNumber * _Nullable)endpointID
                       clusterID:(NSNumber * _Nullable)clusterID
                         eventID:(NSNumber * _Nullable)eventID
                          params:(MTRReadParams * _Nullable)params
                           queue:(dispatch_queue_t)queue
                      completion:(MTRDeviceResponseHandler)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Subscribes to the specified events on the device.
 *
 * Nil values for endpointID, clusterID, eventID indicate wildcards
 * (e.g. nil eventID means "subscribe to all the events from the
 * endpoint(s) and cluster(s) that match endpointID/clusterID").
 *
 * If all of endpointID, clusterID, eventID are non-nil, a single event
 * will be subscribed to.
 *
 * If all of endpointID, clusterID, eventID are nil, all events on the
 * device will be subscribed to.
 *
 * The reportHandler will be called with an error if the subscription fails
 * entirely.
 *
 * The reportHandler will be called with arrays of response-value dictionaries
 * (which may be data or errors) as path-specific data is received.
 *
 * subscriptionEstablished will be called when the subscription is first
 * successfully established (after the initial set of data reports has been
 * delivered to reportHandler).  If params allow automatic resubscription, it
 * will be called any time resubscription succeeds.
 */
- (void)subscribeToEventsWithEndpointID:(NSNumber * _Nullable)endpointID
                              clusterID:(NSNumber * _Nullable)clusterID
                                eventID:(NSNumber * _Nullable)eventID
                                 params:(MTRSubscribeParams * _Nullable)params
                                  queue:(dispatch_queue_t)queue
                          reportHandler:(MTRDeviceResponseHandler)reportHandler
                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

/**
 * A path indicating a specific cluster on a device (i.e. without any
 * wildcards).
 */
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRClusterPath : NSObject <NSCopying>
@property (nonatomic, readonly, copy) NSNumber * endpoint;
@property (nonatomic, readonly, copy) NSNumber * cluster;

+ (MTRClusterPath *)clusterPathWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

/**
 * A path indicating a specific attribute on a device (i.e. without any
 * wildcards).
 */
@interface MTRAttributePath : MTRClusterPath
@property (nonatomic, readonly, copy) NSNumber * attribute;

+ (MTRAttributePath *)attributePathWithEndpointID:(NSNumber *)endpointID
                                        clusterID:(NSNumber *)clusterID
                                      attributeID:(NSNumber *)attributeID
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

/**
 * A path indicating a specific event that can be emitted on a device
 * (i.e. without any wildcards).  There can be multiple instances of actual
 * events for a given event path.
 */
@interface MTREventPath : MTRClusterPath
@property (nonatomic, readonly, copy) NSNumber * event;

+ (MTREventPath *)eventPathWithEndpointID:(NSNumber *)endpointID
                                clusterID:(NSNumber *)clusterID
                                  eventID:(NSNumber *)eventID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

/**
 * A path indicating a specific command on a device (i.e. without any
 * wildcards).
 */
@interface MTRCommandPath : MTRClusterPath
@property (nonatomic, readonly, copy) NSNumber * command;

+ (MTRCommandPath *)commandPathWithEndpointID:(NSNumber *)endpointID
                                    clusterID:(NSNumber *)clusterID
                                    commandID:(NSNumber *)commandID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
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

typedef NS_ENUM(NSUInteger, MTREventTimeType) {
    MTREventTimeTypeSystemUpTime = 0,
    MTREventTimeTypeTimestampDate
} API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));

typedef NS_ENUM(NSUInteger, MTREventPriority) {
    MTREventPriorityDebug = 0,
    MTREventPriorityInfo = 1,
    MTREventPriorityCritical = 2
} API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));

@interface MTREventReport : NSObject
@property (nonatomic, readonly, copy) MTREventPath * path;
@property (nonatomic, readonly, copy) NSNumber * eventNumber; // EventNumber type (uint64_t)
@property (nonatomic, readonly, copy) NSNumber * priority; // PriorityLevel type (MTREventPriority)

// Either systemUpTime or timestampDate will be valid depending on eventTimeType
@property (nonatomic, readonly) MTREventTimeType eventTimeType API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
@property (nonatomic, readonly) NSTimeInterval systemUpTime API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
@property (nonatomic, readonly, copy, nullable)
    NSDate * timestampDate API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));

// An instance of one of the event payload interfaces.
@property (nonatomic, readonly, copy) id value;

// If this specific path resulted in an error, the error (in the
// MTRInteractionErrorDomain or MTRErrorDomain) that corresponds to this
// path.
@property (nonatomic, readonly, copy, nullable) NSError * error;
@end

@interface MTRBaseDevice (Deprecated)

/**
 * Deprecated MTRBaseDevice APIs.
 */
- (void)subscribeWithQueue:(dispatch_queue_t)queue
                minInterval:(uint16_t)minInterval
                maxInterval:(uint16_t)maxInterval
                     params:(MTRSubscribeParams * _Nullable)params
             cacheContainer:(MTRAttributeCacheContainer * _Nullable)attributeCacheContainer
     attributeReportHandler:(MTRDeviceReportHandler _Nullable)attributeReportHandler
         eventReportHandler:(MTRDeviceReportHandler _Nullable)eventReportHandler
               errorHandler:(MTRDeviceErrorHandler)errorHandler
    subscriptionEstablished:(dispatch_block_t _Nullable)subscriptionEstablishedHandler
    resubscriptionScheduled:(MTRDeviceResubscriptionScheduledHandler _Nullable)resubscriptionScheduledHandler
    MTR_DEPRECATED("Please use "
                   "subscribeWithQueue:params:clusterStateCacheContainer:attributeReportHandler:eventReportHandler:errorHandler:"
                   "subscriptionEstablished:resubscriptionScheduled:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                          clusterId:(NSNumber * _Nullable)clusterId
                        attributeId:(NSNumber * _Nullable)attributeId
                             params:(MTRReadParams * _Nullable)params
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion
    MTR_DEPRECATED("Please use readAttributesWithEndpointID:clusterID:attributeID:params:queue:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)writeAttributeWithEndpointId:(NSNumber *)endpointId
                           clusterId:(NSNumber *)clusterId
                         attributeId:(NSNumber *)attributeId
                               value:(id)value
                   timedWriteTimeout:(NSNumber * _Nullable)timeoutMs
                         clientQueue:(dispatch_queue_t)clientQueue
                          completion:(MTRDeviceResponseHandler)completion
    MTR_DEPRECATED("Please use writeAttributeWithEndpointID:clusterID:attributeID:value:timedWriteTimeout:queue:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)invokeCommandWithEndpointId:(NSNumber *)endpointId
                          clusterId:(NSNumber *)clusterId
                          commandId:(NSNumber *)commandId
                      commandFields:(id)commandFields
                 timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion
    MTR_DEPRECATED("Please use invokeCommandWithEndpointID:clusterID:commandID:commandFields:timedInvokeTimeout:queue:completion",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)subscribeAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                               clusterId:(NSNumber * _Nullable)clusterId
                             attributeId:(NSNumber * _Nullable)attributeId
                             minInterval:(NSNumber *)minInterval
                             maxInterval:(NSNumber *)maxInterval
                                  params:(MTRSubscribeParams * _Nullable)params
                             clientQueue:(dispatch_queue_t)clientQueue
                           reportHandler:(MTRDeviceResponseHandler)reportHandler
                 subscriptionEstablished:(dispatch_block_t _Nullable)subscriptionEstablishedHandler
    MTR_DEPRECATED("Please use "
                   "subscribeToAttributesWithEndpointID:clusterID:attributeID:params:queue:"
                   "reportHandler:subscriptionEstablished:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)deregisterReportHandlersWithClientQueue:(dispatch_queue_t)queue
                                     completion:(dispatch_block_t)completion
    MTR_DEPRECATED("Pease use deregisterReportHandlersWithQueue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
        tvos(16.1, 16.4));

@end

@interface MTRAttributePath (Deprecated)

+ (instancetype)attributePathWithEndpointId:(NSNumber *)endpointId
                                  clusterId:(NSNumber *)clusterId
                                attributeId:(NSNumber *)attributeId
    MTR_DEPRECATED("Please use attributePathWithEndpointID:clusterID:attributeID:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTREventPath (Deprecated)

+ (instancetype)eventPathWithEndpointId:(NSNumber *)endpointId
                              clusterId:(NSNumber *)clusterId
                                eventId:(NSNumber *)eventId
    MTR_DEPRECATED("Please use eventPathWithEndpointID:clusterID:eventID:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
        tvos(16.1, 16.4));

@end

@interface MTRCommandPath (Deprecated)

+ (instancetype)commandPathWithEndpointId:(NSNumber *)endpointId
                                clusterId:(NSNumber *)clusterId
                                commandId:(NSNumber *)commandId
    MTR_DEPRECATED("Please use commandPathWithEndpointID:clusterID:commandID:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTREventReport (Deprecated)
@property (nonatomic, readonly, copy) NSNumber * timestamp MTR_DEPRECATED(
    "Please use timestampDate and systemUpTime", ios(16.1, 16.5), macos(13.0, 13.4), watchos(9.1, 9.5), tvos(16.1, 16.5));
@end

NS_ASSUME_NONNULL_END
