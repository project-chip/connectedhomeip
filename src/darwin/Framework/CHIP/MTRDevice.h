/**
 *
 *    Copyright (c) 2022-2025 Project CHIP Authors
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
#import <Matter/MTRAttributeValueWaiter.h>
#import <Matter/MTRBaseClusters.h>
#import <Matter/MTRBaseDevice.h>
#import <Matter/MTRCommandWithRequiredResponse.h>
#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRDeviceController;

typedef NS_ENUM(NSUInteger, MTRDeviceState) {
    MTRDeviceStateUnknown = 0,
    MTRDeviceStateReachable = 1,
    MTRDeviceStateUnreachable = 2
};

@protocol MTRDeviceDelegate;

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDevice : NSObject
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Get an MTRDevice object representing a device with a specific node ID
 * associated with a specific controller.
 *
 * MTRDevice objects are stateful, and callers should hold on to the MTRDevice
 * while they are using it.
 */
+ (MTRDevice *)deviceWithNodeID:(NSNumber *)nodeID
                     controller:(MTRDeviceController *)controller MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * The current state of the device.
 *
 * The three states:
 *   MTRDeviceStateUnknown
 *      Unable to determine the state of the device at the moment.
 *
 *   MTRDeviceStateReachable
 *      Communication with the device is expected to succeed.
 *
 *   MTRDeviceStateUnreachable
 *      The device is currently unreachable.
 */
@property (nonatomic, readonly) MTRDeviceState state;

/**
 * Is the device cache primed for this device?
 *
 * This will be true after the deviceCachePrimed: delegate callback has been called, false if not.
 *
 * Please note if you have a storage delegate implemented, the cache is then stored persistently, so
 * the delegate would then only be called once, ever - and this property would basically always be true
 * if a subscription has ever been established at any point in the past.
 *
 */
@property (readonly) BOOL deviceCachePrimed MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

/**
 * The estimated device system start time.
 *
 * A device can report its events with either calendar time or time since system start time. When events are reported with time
 * since system start time, this property will return an estimation of the device system start time. Because a device may report
 * timestamps this way due to the lack of a wall clock, system start time can only be estimated based on event receive time and the
 * timestamp value, and this estimation may change over time.
 *
 * Device reboots may also cause the estimated device start time to jump forward.
 *
 * If events are always reported with calendar time, then this property will return nil.
 */
@property (nonatomic, readonly, nullable)
    NSDate * estimatedStartTime MTR_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));

/**
 * The controller this device was created for.  May return nil if that
 * controller has been shut down.
 */
@property (nonatomic, readonly, nullable) MTRDeviceController * deviceController MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

/**
 * The node ID of the node this device corresponds to.
 */
@property (nonatomic, readonly, copy) NSNumber * nodeID NS_REFINED_FOR_SWIFT MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

/**
 * An estimate of how much time is likely to elapse between setDelegate being
 * called and the current device state (attributes, stored events) being known.
 *
 * nil if no such estimate is available.  Otherwise, the NSNumber stores an NSTimeInterval.
 */
@property (nonatomic, readonly, nullable, copy) NSNumber * estimatedSubscriptionLatency MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

/**
 * The Vendor Identifier associated with the device.
 *
 * A non-nil value if the vendor identifier has been determined from the device, nil if unknown.
 */
@property (nonatomic, readonly, nullable, copy) NSNumber * vendorID MTR_AVAILABLE(ios(18.3), macos(15.3), watchos(11.3), tvos(18.3));

/**
 * The Product Identifier associated with the device.
 *
 * A non-nil value if the product identifier has been determined from the device, nil if unknown.
 */
@property (nonatomic, readonly, nullable, copy) NSNumber * productID MTR_AVAILABLE(ios(18.3), macos(15.3), watchos(11.3), tvos(18.3));

/**
 * Network commissioning features supported by the device.
 */
@property (nonatomic, readonly) MTRNetworkCommissioningFeature networkCommissioningFeatures MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

/**
 * Set the delegate to receive asynchronous callbacks about the device.
 *
 * The delegate will be called on the provided queue, for attribute reports, event reports, and device state changes.
 */
- (void)setDelegate:(id<MTRDeviceDelegate>)delegate queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use addDelegate:queue:interestedPaths:", ios(16.1, 18.0), macos(13.0, 15.0), watchos(9.1, 11.0), tvos(16.1, 18.0));

/**
 * Adds a delegate to receive asynchronous callbacks about the device.
 *
 * The delegate will be called on the provided queue, for attribute reports, event reports, and device state changes.
 *
 * MTRDevice holds a weak reference to the delegate object.
 */
- (void)addDelegate:(id<MTRDeviceDelegate>)delegate queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.0), macos(15.0), watchos(11.0), tvos(18.0));

/**
 * Adds a delegate to receive asynchronous callbacks about the device, and limit attribute and/or event reports to a specific set of paths.
 *
 * interestedPathsForAttributes may contain either MTRClusterPath or MTRAttributePath to specify interested clusters and attributes, or NSNumber for endpoints.
 *
 * interestedPathsForEvents may contain either MTRClusterPath or MTREventPath to specify interested clusters and events, or NSNumber for endpoints.
 *
 * For both interested paths arguments, if nil is specified, then no filter will be applied.
 *
 * Calling addDelegate: again with the same delegate object will update the interested paths for attributes and events for this delegate.
 *
 * MTRDevice holds a weak reference to the delegate object.
 */
- (void)addDelegate:(id<MTRDeviceDelegate>)delegate queue:(dispatch_queue_t)queue interestedPathsForAttributes:(NSArray * _Nullable)interestedPathsForAttributes interestedPathsForEvents:(NSArray * _Nullable)interestedPathsForEvents MTR_AVAILABLE(ios(18.0), macos(15.0), watchos(11.0), tvos(18.0));

/**
 * Removes the delegate from receiving callbacks about the device.
 */
- (void)removeDelegate:(id<MTRDeviceDelegate>)delegate MTR_AVAILABLE(ios(18.0), macos(15.0), watchos(11.0), tvos(18.0));

/**
 * Read attribute in a designated attribute path.  If there is no value available
 * for the attribute, whether because the device does not implement it or
 * because the subscription priming read has not yet gotten to this attribute,
 * nil will be returned.
 *
 * TODO: Need to fully document that this returns "the system's best guess" of attribute values.
 *
 * @return a data-value dictionary of the attribute as described in MTRDeviceResponseHandler,
 *         or nil if there is no value.
 */
- (NSDictionary<NSString *, id> * _Nullable)readAttributeWithEndpointID:(NSNumber *)endpointID
                                                              clusterID:(NSNumber *)clusterID
                                                            attributeID:(NSNumber *)attributeID
                                                                 params:(MTRReadParams * _Nullable)params;

/**
 * Write to attribute in a designated attribute path
 *
 * @param value       A data-value NSDictionary object as described in
 *                    MTRDeviceResponseHandler.
 *
 * @param expectedValueInterval  maximum interval in milliseconds during which reads of the attribute will return the value being
 * written. This value must be within [1, UINT32_MAX], and will be clamped to this range.
 *
 * TODO: document that -readAttribute... will return the expected value for the [endpoint,cluster,attribute] until one of the
 * following:
 *  1. Another write for the same attribute happens.
 *  2. expectedValueIntervalMs (clamped) expires. Need to figure out phrasing here.
 *  3. We succeed at writing the attribute.
 *  4. We fail at writing the attribute and give up on the write
 *
 * @param timeout   timeout in milliseconds for timed write, or nil. This value must be within [1, UINT16_MAX], and will be clamped
 * to this range.
 * TODO: make timeout arguments uniform
 */
- (void)writeAttributeWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                         attributeID:(NSNumber *)attributeID
                               value:(id)value
               expectedValueInterval:(NSNumber *)expectedValueInterval
                   timedWriteTimeout:(NSNumber * _Nullable)timeout;

/**
 * Read the attributes identified by the provided attribute paths.  The paths
 * can include wildcards.
 *
 * Paths that do not correspond to any existing attributes, or that the
 * MTRDevice does not have attribute values for, will not be present in the
 * return value from this function.
 *
 * @return an array of response-value dictionaries as described in the
 *         documentation for MTRDeviceResponseHandler.  Each one will have an
 *         MTRAttributePathKey and an MTRDataKey.
 */
- (NSArray<NSDictionary<NSString *, id> *> *)readAttributePaths:(NSArray<MTRAttributeRequestPath *> *)attributePaths MTR_AVAILABLE(ios(18.2), macos(15.2), watchos(11.2), tvos(18.2));

/**
 * Read all known attributes from descriptor clusters on all known endpoints.
 *
 * @return A dictionary with the paths of the attributes as keys and the
 *         data-values (as described in the documentation for
 *         MTRDeviceResponseHandler) as values.
 */
- (NSDictionary<MTRAttributePath *, NSDictionary<NSString *, id> *> *)descriptorClusters MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

/**
 * Invoke a command with a designated command path
 *
 * @param commandFields command fields object. If not nil, the object must be a data-value
 *                      NSDictionary object as described in the MTRDeviceResponseHandler
 *                      documentation. The value must be a Structure, i.e., the NSDictionary
 *                      MTRTypeKey key must have the value MTRStructureValueType.
 *
 *                      If commandFields is nil, it will be treated as a Structure with no fields.
 *
 * @param expectedValues The expected values of attributes that will be affected by the command, if
 *                       any.  If these are provided, the relevant attributes will have the provided
 *                       values when read until one of the following happens:
 *
 *                       1. Something (another invoke or a write) sets different expected values.
 *                       2. expectedValueInterval elapses without the device reporting the
 *                          attributes changing their values to the expected values.
 *                       3. The command invoke fails.
 *                       4. The device reports some other values for these attributes.
 *
 *                       The dictionaries in this array are expected to be response-value
 *                       dictionaries as documented in the documentation of
 *                       MTRDeviceResponseHandler, and each one must have an MTRAttributePathKey.
 *
 *                       The expectedValues and expectedValueInterval arguments need to be both
 *                       nil or both non-nil, or both will be both ignored.
 *
 * @param expectedValueInterval  maximum interval in milliseconds during which reads of the
 *                               attributes that had expected values provided will return the
 *                               expected values. If the value is less than 1, both this value and
 *                               expectedValues will be ignored. If this value is greater than
 *                               UINT32_MAX, it will be clamped to UINT32_MAX.
 *
 * @param completion  response handler will receive either values or error.  A
 *                    path-specific error status from the command invocation
 *                    will result in an error being passed to the completion, so
 *                    values will only be passed in when the command succeeds.
 *
 *                    If values are passed, the array length will always be 1 and the single
 *                    response-value in it will have an MTRCommandPathKey.  If the command
 *                    response is just a success status, there will be no MTRDataKey.  If the
 *                    command response has data fields, there will be an MTRDataKey, whose value
 *                    will be of type MTRStructureValueType and describe the response payload.
 */
- (void)invokeCommandWithEndpointID:(NSNumber *)endpointID
                          clusterID:(NSNumber *)clusterID
                          commandID:(NSNumber *)commandID
                      commandFields:(NSDictionary<NSString *, id> * _Nullable)commandFields
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues
              expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval
                              queue:(dispatch_queue_t)queue
                         completion:(MTRDeviceResponseHandler)completion MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (void)invokeCommandWithEndpointID:(NSNumber *)endpointID
                          clusterID:(NSNumber *)clusterID
                          commandID:(NSNumber *)commandID
                      commandFields:(id)commandFields
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues
              expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval
                 timedInvokeTimeout:(NSNumber * _Nullable)timeout
                              queue:(dispatch_queue_t)queue
                         completion:(MTRDeviceResponseHandler)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Invoke one or more groups of commands.
 *
 * For any given group, if any command in any preceding group failed, the group
 * will be skipped.  If all commands in all preceding groups succeeded, the
 * commands within the group will be invoked, with no ordering guarantees within
 * that group.
 *
 * Results from all commands that were invoked will be passed to the provided
 * completion as an array of response-value dictionaries.  Each of these will
 * have the command path of the command (see MTRCommandPathKey) and one of three
 * things:
 *
 * 1) No other fields, indicating that the command invoke returned a succcess
 *    status.
 * 2) A field for MTRErrorKey, indicating that the invoke returned a failure
 *    status (which is the value of the field).
 * 3) A field for MTRDataKey, indicating that the invoke returned a data
 *    response.  In this case the data-value representing the response will be
 *    the value of this field.
 */
- (void)invokeCommands:(NSArray<NSArray<MTRCommandWithRequiredResponse *> *> *)commands
                 queue:(dispatch_queue_t)queue
            completion:(MTRDeviceResponseHandler)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

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
    MTR_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2));

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
                                      completion:(MTRDeviceOpenCommissioningWindowHandler)completion
    MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));

/**
 * Download log of the desired type from the device.
 *
 * Note: The consumer of this API should move the file that the url points to or open it for reading before the
 * completion handler returns. Otherwise, the file will be deleted, and the data will be lost.
 *
 * @param type       The type of log being requested. This should correspond to a value in the enum MTRDiagnosticLogType.
 * @param timeout    The timeout for getting the log. If the timeout expires, completion will be called with whatever
 *                   has been retrieved by that point (which might be none or a partial log).
 *                   If the timeout is set to 0, the request will not expire and completion will not be called until
 *                   the log is fully retrieved or an error occurs.
 * @param queue      The queue on which completion will be called.
 * @param completion The completion handler that is called after attempting to retrieve the requested log.
 *                     - In case of success, the completion handler is called with a non-nil URL and a nil error.
 *                     - If there is an error, a non-nil error is used and the url can be non-nil too if some logs have already been downloaded.
 */
- (void)downloadLogOfType:(MTRDiagnosticLogType)type
                  timeout:(NSTimeInterval)timeout
                    queue:(dispatch_queue_t)queue
               completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

/**
 * Sets up the provided completion to be called when any of the following
 * happens:
 *
 * 1) A set of attributes reaches certain values: completion called with nil.
 * 2) The provided timeout expires: completion called with MTRErrorCodeTimeout error.
 * 3) The wait is canceled: completion called with MTRErrorCodeCancelled error.
 *
 * If the MTRAttributeValueWaiter is destroyed before the
 * completion is called, that is treated the same as canceling the waiter.
 *
 * The attributes and values to wait for are represented as a dictionary which
 * has the attribute paths as keys and the expected data-values as values.
 */
- (MTRAttributeValueWaiter *)waitForAttributeValues:(NSDictionary<MTRAttributePath *, NSDictionary<NSString *, id> *> *)values
                                            timeout:(NSTimeInterval)timeout
                                              queue:(dispatch_queue_t)queue
                                         completion:(void (^)(NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.3), macos(15.3), watchos(11.3), tvos(18.3));

@end

MTR_EXTERN NSString * const MTRPreviousDataKey MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
MTR_EXTERN NSString * const MTRDataVersionKey MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@protocol MTRDeviceDelegate <NSObject>
@required
/**
 * @param state The current state of the device
 */
- (void)device:(MTRDevice *)device stateChanged:(MTRDeviceState)state;

/**
 * Notifies delegate of attribute reports from the MTRDevice
 *
 * @param attributeReport  An array of response-value objects as described in MTRDeviceResponseHandler
 *
 *                In addition to MTRDataKey, each response-value dictionary in the array may also have this key:
 *
 *                MTRPreviousDataKey : Same data-value dictionary format as the object for MTRDataKey. This is included when the previous value is known for an attribute.
 *
 *                The data-value dictionary also contains this key:
 *
 *                MTRDataVersionKey : NSNumber-wrapped uin32_t.
 */
- (void)device:(MTRDevice *)device receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport;

/**
 * Notifies delegate of event reports from the MTRDevice
 *
 * @param eventReport  An array of response-value objects as described in MTRDeviceResponseHandler
 *
 *                In addition to the MTREventPathKey and MTRDataKey containing the path and event values, eventReport also contains
 *                these keys:
 *
 *                MTREventNumberKey : NSNumber-wrapped uint64_t value. Monotonically increasing, and consecutive event reports
 *                                    should have consecutive numbers unless device reboots, or if events are lost.
 *                MTREventPriorityKey : NSNumber-wrapped MTREventPriority value.
 *                MTREventTimeTypeKey : NSNumber-wrapped MTREventTimeType value.
 *                MTREventSystemUpTimeKey : NSNumber-wrapped NSTimeInterval value.
 *                MTREventTimestampDateKey : NSDate object.
 *
 *                Only one of MTREventTimestampDateKey and MTREventSystemUpTimeKey will be present, depending on the value for
 *                MTREventTimeTypeKey.
 */
- (void)device:(MTRDevice *)device receivedEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport;

@optional
/**
 * Notifies delegate the device is currently actively communicating.
 *
 * This can be used as a hint that now is a good time to send commands to the
 * device, especially if the device is sleepy and might not be active very often.
 */
- (void)deviceBecameActive:(MTRDevice *)device MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Notifies delegate when the device attribute cache has been primed with initial configuration data of the device
 *
 * This is called when the MTRDevice object goes from not knowing the device to having cached the first attribute reports that include basic mandatory information, e.g. Descriptor clusters.
 *
 * The intention is that after this is called, the client should be able to call read for mandatory attributes and likely expect non-nil values.
 */
- (void)deviceCachePrimed:(MTRDevice *)device MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

/**
 * This is called when the MTRDevice object detects a change in the device configuration.
 *
 * Device configuration is the set of functionality implemented by the device.
 *
 */
- (void)deviceConfigurationChanged:(MTRDevice *)device MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

@interface MTRDevice (Deprecated)

/**
 * Deprecated MTRDevice APIs.
 */
+ (MTRDevice *)deviceWithNodeID:(uint64_t)nodeID
               deviceController:(MTRDeviceController *)deviceController
    MTR_DEPRECATED(
        "Please use deviceWithNodeID:controller:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)invokeCommandWithEndpointID:(NSNumber *)endpointID
                          clusterID:(NSNumber *)clusterID
                          commandID:(NSNumber *)commandID
                      commandFields:(id)commandFields
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues
              expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval
                 timedInvokeTimeout:(NSNumber * _Nullable)timeout
                        clientQueue:(dispatch_queue_t)queue
                         completion:(MTRDeviceResponseHandler)completion
    MTR_DEPRECATED("Please use "
                   "invokeCommandWithEndpointID:clusterID:commandID:commandFields:expectedValues:expectedValueInterval:"
                   "timedInvokeTimeout:queue:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

NS_ASSUME_NONNULL_END
