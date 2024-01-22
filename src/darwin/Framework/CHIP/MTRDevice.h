/**
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
#import <Matter/MTRBaseDevice.h>
#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRDeviceController;

typedef NS_ENUM(NSUInteger, MTRDeviceState) {
    MTRDeviceStateUnknown = 0,
    MTRDeviceStateReachable = 1,
    MTRDeviceStateUnreachable = 2
};

@protocol MTRDeviceDelegate;

@interface MTRDevice : NSObject
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * TODO: Document usage better
 *
 * Directly instantiate a MTRDevice with a MTRDeviceController as a shim.
 *
 * All device-specific information would be stored on the device controller, and
 * retrieved when performing actions using a combination of MTRBaseDevice
 * and MTRAsyncCallbackQueue.
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
 * Set the delegate to receive asynchronous callbacks about the device.
 *
 * The delegate will be called on the provided queue, for attribute reports, event reports, and device state changes.
 */
- (void)setDelegate:(id<MTRDeviceDelegate>)delegate queue:(dispatch_queue_t)queue;

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
 *
 * This set of functions allows clients to store metadata for either an entire device or for a specific endpoint.
 *
 * Notes:
 *   • Client data will be removed automatically when devices are deleted from the fabric
 *   • Supported client data object types are currently only:
 *         NSData, NSString, NSArray, NSDictionary, NSNumber
 */

/**
 *
 * List of all client data types supported
 *
 */
- (NSArray *)supportedClientDataClasses MTR_UNSTABLE_API;

/**
 *
 * List of all client data keys stored
 *
 */
- (NSArray * _Nullable)clientDataKeys MTR_UNSTABLE_API;

/**
 *
 * Retrieve client metadata for a key, returns nil if no value is set
 *
 * @param key           NSString * for the key to store the value as
 */
- (id<NSSecureCoding> _Nullable)clientDataForKey:(NSString *)key MTR_UNSTABLE_API;

/**
 *
 * Set client metadata for a key. The value must conform to NSSecureCoding
 *
 * @param key           NSString * for the key to store the value as
 * @param value         id <NSSecureCoding> for the value to store
 */
- (void)setClientDataForKey:(NSString *)key value:(id<NSSecureCoding>)value MTR_UNSTABLE_API;

/**
 *
 * Remove client metadata for a key.
 *
 * @param key           NSString * for the key to store the value as
 */
- (void)removeClientDataForKey:(NSString *)key MTR_UNSTABLE_API;

/**
 *
 * List of all client data keys stored
 *
 */
- (NSArray * _Nullable)clientDataKeysForEndpointID:(NSNumber *)endpointID MTR_UNSTABLE_API;

/**
 *
 * Retrieve client metadata for a key, returns nil if no value is set
 *
 * @param key           NSString * for the key to store the value as
 * @param endpointID    NSNumber * for the endpoint to associate the metadata with
 */
- (id<NSSecureCoding> _Nullable)clientDataForKey:(NSString *)key endpointID:(NSNumber *)endpointID MTR_UNSTABLE_API;

/**
 *
 * Set client metadata for a key. The value must conform to NSSecureCoding.
 *
 * @param key           NSString * for the key to store the value as.
 * @param endpointID    NSNumber * for the endpoint to associate the metadata with
 * @param value         id <NSSecureCoding> for the value to store
 */
- (void)setClientDataForKey:(NSString *)key endpointID:(NSNumber *)endpointID value:(id<NSSecureCoding>)value MTR_UNSTABLE_API;

/**
 *
 * Remove client metadata for a key.
 *
 * @param key           NSString * for the key to store the value as
 * @param endpointID    NSNumber * for the endpoint to associate the metadata with
 */
- (void)removeClientDataForKey:(NSString *)key endpointID:(NSNumber *)endpointID MTR_UNSTABLE_API;

@end

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
