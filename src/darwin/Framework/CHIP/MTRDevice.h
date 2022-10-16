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

#import <Foundation/Foundation.h>
#import <Matter/MTRBaseDevice.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRDeviceController;
@class MTRAsyncCallbackWorkQueue;

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
+ (instancetype)deviceWithNodeID:(uint64_t)nodeID deviceController:(MTRDeviceController *)deviceController;

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
 * Set the delegate to receive asynchronous callbacks about the device.
 *
 * The delegate will be called on the provided queue, for attribute reports, event reports, and device state changes.
 */
- (void)setDelegate:(id<MTRDeviceDelegate>)delegate queue:(dispatch_queue_t)queue;

/**
 * Read attribute in a designated attribute path
 *
 * TODO: Need to document that this returns "the system's best guess" of attribute values.
 *
 * @return a data-value dictionary of the attribute as described in MTRDeviceResponseHandler
 */
- (NSDictionary<NSString *, id> *)readAttributeWithEndpointID:(NSNumber *)endpointID
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
 * written. This value will be clamped to timeoutMs
 *
 * TODO: document that -readAttribute... will return the expected value for the [endpoint,cluster,attribute] until one of the
 * following:
 *  1. Another write for the same attribute happens.
 *  2. expectedValueIntervalMs (clamped) expires. Need to figure out phrasing here.
 *  3. We succeed at writing the attribute.
 *  4. We fail at writing the attribute and give up on the write
 *
 * @param timeout   timeout in milliseconds for timed write, or nil.
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
 * @param commandFields   command fields object. The object must be a data-value NSDictionary object
 *                      as described in the MTRDeviceResponseHandler.
 *                      The attribute must be a Structure, i.e.,
 *                      the NSDictionary MTRTypeKey key must have the value MTRStructureValueType.
 *
 * @param expectedValues  array of dictionaries containing the expected values in the same format as
 *                       attribute read completion handler. Requires MTRAttributePathKey values.
 *                       See MTRDeviceResponseHandler definition for dictionary details.
 * TODO: document better the expectedValues is how this command is expected to change attributes when read, and that the next
 * readAttribute will get these values
 *
 * @param expectedValueInterval  maximum interval in milliseconds during which reads of the attribute will return the value being
 * written. This value will be clamped to timeout
 *
 * @param timeout   timeout in milliseconds for timed invoke, or nil.
 *
 * @param completion  response handler will receive either values or error.
 */
- (void)invokeCommandWithEndpointID:(NSNumber *)endpointID
                          clusterID:(NSNumber *)clusterID
                          commandID:(NSNumber *)commandID
                      commandFields:(id)commandFields
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues
              expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval
                 timedInvokeTimeout:(NSNumber * _Nullable)timeout
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion;

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
                                      completion:(MTRDeviceOpenCommissioningWindowHandler)completion;

@end

@protocol MTRDeviceDelegate <NSObject>
@required
/**
 * device:stateChanged:
 *
 * @param state The current state of the device
 */
- (void)device:(MTRDevice *)device stateChanged:(MTRDeviceState)state;

/**
 * device:receivedAttributeReport:
 *
 * Notifies delegate of attribute reports from the MTRDevice
 *
 * @param attributeReport  An array of response-value objects as described in MTRDeviceResponseHandler
 */
- (void)device:(MTRDevice *)device receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport;

/**
 * subscriptionReceivedEventReport:
 *
 * Notifies delegate of event reports from the MTRDevice
 *
 * @param eventReport  An array of response-value objects as described in MTRDeviceResponseHandler
 */
- (void)device:(MTRDevice *)device receivedEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport;

@end

NS_ASSUME_NONNULL_END
