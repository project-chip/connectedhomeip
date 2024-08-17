//
/**
 *    Copyright (c) 2023 Project CHIP Authors
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

#import "MTRDevice_XPC.h"

// Need to move these to PCHs...
#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceControllerParameters.h>

#import "MTRDeviceController_Internal.h"

#import "MTRAsyncWorkQueue.h"
#import "MTRAttestationTrustStoreBridge.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRCommissionableBrowser.h"
#import "MTRCommissionableBrowserResult_Internal.h"
#import "MTRCommissioningParameters.h"
#import "MTRConversion.h"
#import "MTRDefines_Internal.h"
#import "MTRDeviceAttestationDelegateBridge.h"
#import "MTRDeviceConnectionBridge.h"
#import "MTRDeviceController.h"
#import "MTRDeviceControllerDelegateBridge.h"
#import "MTRDeviceControllerFactory_Internal.h"
#import "MTRDeviceControllerLocalTestStorage.h"
#import "MTRDeviceControllerStartupParams.h"
#import "MTRDeviceControllerStartupParams_Internal.h"
#import "MTRDeviceControllerXPCParameters.h"
#import "MTRDeviceController_Concrete.h"
#import "MTRDeviceController_XPC.h"
#import "MTRDevice_Concrete.h"
#import "MTRDevice_Internal.h"
#import "MTRError_Internal.h"
#import "MTRKeypair.h"
#import "MTRLogging_Internal.h"
#import "MTRMetricKeys.h"
#import "MTRMetricsCollector.h"
#import "MTROperationalCredentialsDelegate.h"
#import "MTRP256KeypairBridge.h"
#import "MTRPersistentStorageDelegateBridge.h"
#import "MTRServerEndpoint_Internal.h"
#import "MTRSetupPayload.h"
#import "MTRTimeUtils.h"
#import "MTRUnfairLock.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <atomic>
#include <dns_sd.h>
#include <string>

#import <os/lock.h>

#define MTR_DEVICE_SIMPLE_REMOTE_XPC_GETTER(NAME, TYPE, DEFAULT_VALUE, GETTER_NAME)                                                                            \
    MTR_SIMPLE_REMOTE_XPC_GETTER([(MTRDeviceController_XPC *) [self deviceController] xpcConnection], NAME, TYPE, DEFAULT_VALUE, GETTER_NAME, deviceController \
                                 : [[self deviceController] uniqueIdentifier] nodeID                                                                           \
                                 : [self nodeID])

#define MTR_DEVICE_COMPLEX_REMOTE_XPC_GETTER(SIGNATURE, TYPE, DEFAULT_VALUE, ADDITIONAL_ARGUMENTS)                                                                            \
    MTR_COMPLEX_REMOTE_XPC_GETTER([(MTRDeviceController_XPC *) [self deviceController] xpcConnection], SIGNATURE, TYPE, DEFAULT_VALUE, ADDITIONAL_ARGUMENTS, deviceController \
                                  : [[self deviceController] uniqueIdentifier] nodeID                                                                                         \
                                  : [self nodeID])

#define MTR_DEVICE_SIMPLE_REMOTE_XPC_COMMAND(METHOD_SIGNATURE, ADDITIONAL_ARGUMENTS)                                                                            \
    MTR_SIMPLE_REMOTE_XPC_COMMAND([(MTRDeviceController_XPC *) [self deviceController] xpcConnection], METHOD_SIGNATURE, ADDITIONAL_ARGUMENTS, deviceController \
                                  : [[self deviceController] uniqueIdentifier] nodeID                                                                           \
                                  : [self nodeID])

@implementation MTRDevice_XPC

#pragma mark - Client Callbacks
- (oneway void)device:(NSNumber *)nodeID stateChanged:(MTRDeviceState)state
{
    [self _callDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
        [delegate device:self stateChanged:state];
    }];
}
- (oneway void)device:(NSNumber *)nodeID receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport
{
    [self _callDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
        [delegate device:self receivedAttributeReport:attributeReport];
    }];
}
- (oneway void)device:(NSNumber *)nodeID receivedEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport
{
    [self _callDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
        [delegate device:self receivedEventReport:eventReport];
    }];
}
- (oneway void)deviceBecameActive:(NSNumber *)nodeID
{
    [self _callDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
        [delegate deviceBecameActive:self];
    }];
}
- (oneway void)deviceCachePrimed:(NSNumber *)nodeID
{
    [self _callDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
        [delegate deviceCachePrimed:self];
    }];
}
- (oneway void)deviceConfigurationChanged:(NSNumber *)nodeID
{
    [self _callDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
        [delegate deviceConfigurationChanged:self];
    }];
}

#pragma mark - Remote Commands

MTR_DEVICE_SIMPLE_REMOTE_XPC_GETTER(state, MTRDeviceState, MTRDeviceStateUnknown, getStateWithReply)
MTR_DEVICE_SIMPLE_REMOTE_XPC_GETTER(deviceCachePrimed, BOOL, NO, getDeviceCachePrimedWithReply)
MTR_DEVICE_SIMPLE_REMOTE_XPC_GETTER(estimatedStartTime, NSDate *, nil, getEstimatedStartTimeWithReply)
MTR_DEVICE_SIMPLE_REMOTE_XPC_GETTER(estimatedSubscriptionLatency, NSNumber *, nil, getEstimatedSubscriptionLatencyWithReply)

typedef NSDictionary<NSString *, id> * readAttributeResponseType;
MTR_DEVICE_COMPLEX_REMOTE_XPC_GETTER(readAttributeWithEndpointID
                                     : (NSNumber *) endpointID clusterID
                                     : (NSNumber *) clusterID attributeID
                                     : (NSNumber *) attributeID params
                                     : (MTRReadParams * _Nullable) params,
                                     readAttributeResponseType,
                                     nil,
                                     readAttributeWithEndpointID
                                     : endpointID clusterID
                                     : clusterID attributeID
                                     : attributeID params
                                     : params withReply)

MTR_DEVICE_SIMPLE_REMOTE_XPC_COMMAND(writeAttributeWithEndpointID
                                     : (NSNumber *) endpointID clusterID
                                     : (NSNumber *) clusterID attributeID
                                     : (NSNumber *) attributeID value
                                     : (id) value expectedValueInterval
                                     : (NSNumber *) expectedValueInterval timedWriteTimeout
                                     : (NSNumber * _Nullable) timeout, writeAttributeWithEndpointID
                                     : endpointID clusterID
                                     : clusterID attributeID
                                     : attributeID value
                                     : value expectedValueInterval
                                     : expectedValueInterval timedWriteTimeout
                                     : timeout)

- (void)invokeCommandWithEndpointID:(NSNumber *)endpointID
                          clusterID:(NSNumber *)clusterID
                          commandID:(NSNumber *)commandID
                      commandFields:(id)commandFields
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues
              expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval
                 timedInvokeTimeout:(NSNumber * _Nullable)timeout
                              queue:(dispatch_queue_t)queue
                         completion:(MTRDeviceResponseHandler)completion
{
    NSXPCConnection * xpcConnection = [(MTRDeviceController_XPC *) [self deviceController] xpcConnection];

    [[xpcConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) {
        MTR_LOG_ERROR("Error: %@", error);
    }] deviceController:[[self deviceController] uniqueIdentifier]
                             nodeID:[self nodeID]
        invokeCommandWithEndpointID:endpointID
                          clusterID:clusterID
                          commandID:commandID
                      commandFields:commandFields
                     expectedValues:expectedValues
              expectedValueInterval:expectedValueInterval
                 timedInvokeTimeout:timeout
                         completion:completion];
}

// Not Supported via XPC
//- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID openCommissioningWindowWithSetupPasscode:(NSNumber *)setupPasscode discriminator:(NSNumber *)discriminator duration:(NSNumber *)duration completion:(MTRDeviceOpenCommissioningWindowHandler)completion;

MTR_DEVICE_SIMPLE_REMOTE_XPC_GETTER(clientDataKeys, NSArray *, nil, getClientDataKeysWithReply)
MTR_DEVICE_COMPLEX_REMOTE_XPC_GETTER(clientDataForKey
                                     : (NSString *) key, id<NSSecureCoding> _Nullable, nil, clientDataForKey
                                     : key withReply)

MTR_DEVICE_SIMPLE_REMOTE_XPC_COMMAND(setClientDataForKey
                                     : (NSString *) key value
                                     : (id<NSSecureCoding>) value, setClientDataForKey
                                     : key value
                                     : value)
MTR_DEVICE_SIMPLE_REMOTE_XPC_COMMAND(removeClientDataForKey
                                     : (NSString *) key, removeClientDataForKey
                                     : key)

MTR_DEVICE_COMPLEX_REMOTE_XPC_GETTER(clientDataKeysForEndpointID
                                     : (NSNumber *) endpointID, NSArray * _Nullable, nil, clientDataKeysForEndpointID
                                     : (NSNumber *) endpointID withReply)
MTR_DEVICE_COMPLEX_REMOTE_XPC_GETTER(clientDataForKey
                                     : (NSString *) key endpointID
                                     : (NSNumber *) endpointID, id<NSSecureCoding> _Nullable, nil, clientDataForKey
                                     : key endpointID
                                     : endpointID withReply)

MTR_DEVICE_SIMPLE_REMOTE_XPC_COMMAND(setClientDataForKey
                                     : (NSString *) key endpointID
                                     : (NSNumber *) endpointID value
                                     : (id<NSSecureCoding>) value, setClientDataForKey
                                     : key endpointID
                                     : endpointID value
                                     : value)
MTR_DEVICE_SIMPLE_REMOTE_XPC_COMMAND(removeClientDataForKey
                                     : (NSString *) key endpointID
                                     : (NSNumber *) endpointID value
                                     : (id<NSSecureCoding>) value, removeClientDataForKey
                                     : key endpointID
                                     : endpointID)

// Not Supported via XPC
// - (oneway void)downloadLogOfType:(MTRDiagnosticLogType)type nodeID:(NSNumber *)nodeID timeout:(NSTimeInterval)timeout completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion;

@end
