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

@protocol MTRDeviceSubscriptionDelegate;

@interface MTRDevice : NSObject
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Directly instantiate a MTRDevice with a MTRDeviceController as a shim.
 *
 * All device-specific information would be stored on the device controller, and
 * retrieved when performing actions using a combination of MTRBaseDevice
 * and MTRAsyncCallbackQueue.
 */
+ (instancetype)deviceWithDeviceID:(uint64_t)deviceID deviceController:(MTRDeviceController *)deviceController;

/**
 * Subscribe to receive attribute reports for everything (all endpoints, all
 * clusters, all attributes, all events) on the device.
 *
 * The subscriber provides a delegate object conforming to MTRDeviceSubscriptionDelegate
 */
- (void)subscribeWithDelegate:(id<MTRDeviceSubscriptionDelegate>)delegate
                        queue:(dispatch_queue_t)queue
                  minInterval:(uint16_t)minInterval
                  maxInterval:(uint16_t)maxInterval
                       params:(MTRSubscribeParams * _Nullable)params;

/**
 * Read attribute in a designated attribute path
 *
 * @return a data-value dictionary of the attribute as described in MTRDeviceResponseHandler
 */
- (NSDictionary<NSString *, id> *)readAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                                                    clusterId:(NSNumber * _Nullable)clusterId
                                                  attributeId:(NSNumber * _Nullable)attributeId
                                                       params:(MTRReadParams * _Nullable)params;

/**
 * Write to attribute in a designated attribute path
 *
 * @param value       A data-value NSDictionary object as described in
 *                    MTRDeviceResponseHandler.
 *
 * @param expectedValueIntervalMs  interval that the write value is assumed to hold true before actual interaction happens. This
 * value will be clamped to timeoutMs.
 *
 * @param timeoutMs   timeout in milliseconds for timed write, or nil.
 *
 *                    Received values are an NSArray object with response-value element as described in
 *                    readAttributeWithEndpointId:clusterId:attributeId:clientQueue:completion:.
 */
- (void)writeAttributeWithEndpointId:(NSNumber *)endpointId
                           clusterId:(NSNumber *)clusterId
                         attributeId:(NSNumber *)attributeId
                               value:(id)value
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                   timedWriteTimeout:(NSNumber * _Nullable)timeoutMs;

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
 *
 * @param expectedValueIntervalMs  interval that the write value is assumed to hold true before actual interaction happens. This
 * value will be clamped to timeoutMs.
 *
 * @param timeoutMs   timeout in milliseconds for timed invoke, or nil.
 *
 * @param completion  response handler will receive either values or error.
 */
- (void)invokeCommandWithEndpointId:(NSNumber *)endpointId
                          clusterId:(NSNumber *)clusterId
                          commandId:(NSNumber *)commandId
                      commandFields:(id)commandFields
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                 timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion;

@end

@protocol MTRDeviceSubscriptionDelegate <NSObject>
@required
/**
 * subscriptionEstablished
 *
 * Called once the subscription is established.  This will
 * be _after_ the first (priming) call to both report callbacks.
 */
- (void)subscriptionEstablished;

/**
 * subscriptionReceivedAttributeReport:
 *
 * Notifies delegate of attribute reports from the MTRDevice
 *
 * @param attributeReport  An array of response-value objects as described in MTRDeviceResponseHandler
 */
- (void)subscriptionReceivedAttributeReport:(NSArray *)attributeReport;

/**
 * subscriptionReceivedEventReport:
 *
 * Notifies delegate of event reports from the MTRDevice
 *
 * @param eventReport  An array of MTREventReport objects
 */
- (void)subscriptionReceivedEventReport:(NSArray *)eventReport;

/**
 * subscriptionEndedWithError:
 *
 * Called any time there is an error for the
 * entire subscription (with a non-nil "error"), and terminate the subscription
 */
- (void)subscriptionEndedWithError:(NSError *)error;
@end

NS_ASSUME_NONNULL_END
