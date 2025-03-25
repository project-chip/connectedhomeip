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
#import "MTRDeviceController_XPC_Internal.h"

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
#import "MTRDeviceController_Concrete.h"
#import "MTRDeviceController_XPC.h"
#import "MTRDeviceDataValidation.h"
#import "MTRDevice_Concrete.h"
#import "MTRDevice_Internal.h"
#import "MTRDevice_XPC_Internal.h"
#import "MTRError_Internal.h"
#import "MTRKeypair.h"
#import "MTRLogging_Internal.h"
#import "MTRMetricKeys.h"
#import "MTRMetricsCollector.h"
#import "MTRP256KeypairBridge.h"
#import "MTRPersistentStorageDelegateBridge.h"
#import "MTRServerEndpoint_Internal.h"
#import "MTRSetupPayload.h"
#import "MTRTimeUtils.h"
#import "MTRUnfairLock.h"
#import "MTRUtilities.h"
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

@synthesize _internalState;
@synthesize queue = _queue;

- (instancetype)initWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController_XPC *)controller
{
    if (self = [super initForSubclassesWithNodeID:nodeID controller:controller]) {
        _queue = dispatch_queue_create("org.csa-iot.matter.framework.devicexpc.workqueue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    }

    return self;
}

- (void)dealloc
{
    [self _setInternalState:nil];
}

- (NSString *)description
{
    NSString * wifi;
    NSString * thread;
    NSNumber * networkFeatures = [self._internalState objectForKey:kMTRDeviceInternalPropertyNetworkFeatures];

    if (networkFeatures == nil) {
        wifi = @"NO";
        thread = @"NO";
    } else {
        wifi = MTR_YES_NO(networkFeatures.unsignedLongLongValue & MTRNetworkCommissioningFeatureWiFiNetworkInterface);
        thread = MTR_YES_NO(networkFeatures.unsignedLongLongValue & MTRNetworkCommissioningFeatureThreadNetworkInterface);
    }

    // TODO: Add these to the description
    // MTR_OPTIONAL_ATTRIBUTE(kMTRDeviceInternalPropertyLastSubscriptionAttemptWait, _lastSubscriptionAttemptWaitForDescription, properties);
    // MTR_OPTIONAL_ATTRIBUTE(kMTRDeviceInternalPropertyMostRecentReportTime, _mostRecentReportTimeForDescription, properties);
    // MTR_OPTIONAL_ATTRIBUTE(kMTRDeviceInternalPropertyLastSubscriptionFailureTime, _lastSubscriptionFailureTimeForDescription, properties);

    return [NSString stringWithFormat:@"<%@: %p, node: %016llX-%016llX (%llu), VID: %@, PID: %@, WiFi: %@, Thread: %@, controller: %@ state: %lu>",
                     NSStringFromClass(self.class), self,
                     _deviceController.compressedFabricID.unsignedLongLongValue,
                     _nodeID.unsignedLongLongValue,
                     _nodeID.unsignedLongLongValue,
                     [self vendorID],
                     [self productID],
                     wifi,
                     thread,
                     _deviceController.uniqueIdentifier,
                     (unsigned long) self.state];
}

- (nullable NSNumber *)vendorID
{
    return [[self._internalState objectForKey:kMTRDeviceInternalPropertyKeyVendorID] copy];
}

- (nullable NSNumber *)productID
{
    return [[self._internalState objectForKey:kMTRDeviceInternalPropertyKeyProductID] copy];
}

- (MTRNetworkCommissioningFeature)networkCommissioningFeatures
{
    return [[self._internalState objectForKey:kMTRDeviceInternalPropertyNetworkFeatures] unsignedIntValue];
}

#pragma mark - Delegate added/removed callbacks

- (void)_delegateAdded:(id<MTRDeviceDelegate>)delegate
{
    os_unfair_lock_assert_owner(&self->_lock);

    [super _delegateAdded:delegate];
    MTR_LOG("%@ delegate added: %@", self, delegate);

    // dispatch to own queue so we're not holding our lock while calling out to the controller code.
    dispatch_async(self.queue, ^{
        [(MTRDeviceController_XPC *) [self deviceController] _updateRegistrationInfo];
    });
}

- (void)_delegateRemoved:(id<MTRDeviceDelegate>)delegate
{
    os_unfair_lock_assert_owner(&self->_lock);

    [super _delegateRemoved:delegate];
    MTR_LOG("%@ delegate removed: %@", self, delegate);

    // dispatch to own queue so we're not holding our lock while calling out to the controller code.
    dispatch_async(self.queue, ^{
        [(MTRDeviceController_XPC *) [self deviceController] _updateRegistrationInfo];
    });
}

#pragma mark - Client Callbacks (MTRDeviceDelegate)

// required methods for MTRDeviceDelegates
- (oneway void)device:(NSNumber *)nodeID stateChanged:(MTRDeviceState)state
{
    // Not needed, since internal will get this
}

- (oneway void)device:(NSNumber *)nodeID receivedAttributeReport:(NSArray<MTRDeviceResponseValueDictionary> *)attributeReport
{
    MTR_LOG("%@ %s", self, __PRETTY_FUNCTION__);
    if (!MTR_SAFE_CAST(nodeID, NSNumber)) {
        MTR_LOG_ERROR("%@ invalid device:receivedAttributeReport: nodeID: %@", self, nodeID);
        return;
    }

    if (!MTRAttributeReportIsWellFormed(attributeReport)) {
        MTR_LOG_ERROR("%@ invalid device:receivedAttributeReport: attributeReport: %@", self, attributeReport);
        return;
    }

    [self _lockAndCallDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
        [delegate device:self receivedAttributeReport:attributeReport];
    }];

    std::lock_guard lock(_lock);
    for (NSDictionary<NSString *, id> * report in attributeReport) {
        if (!MTR_SAFE_CAST(report, NSDictionary)) {
            MTR_LOG_ERROR("%@ handed a response-value that is not a dictionary: %@", self, report);
            continue;
        }

        MTRAttributePath * path = MTR_SAFE_CAST(report[MTRAttributePathKey], MTRAttributePath);
        if (!path) {
            MTR_LOG_ERROR("%@ no valid path for attribute report %@", self, report);
            continue;
        }

        MTRDeviceDataValueDictionary value = report[MTRDataKey];
        if (!value) {
            // This is normal; this could be an error report.
            continue;
        }

        if (!MTR_SAFE_CAST(value, NSDictionary)) {
            MTR_LOG_ERROR("%@ invalid data-value reported: %@", self, report);
            continue;
        }

        [self _attributeValue:value reportedForPath:path];
    }
}

- (oneway void)device:(NSNumber *)nodeID receivedEventReport:(NSArray<MTRDeviceResponseValueDictionary> *)eventReport
{
    MTR_LOG("%@ %s", self, __PRETTY_FUNCTION__);
    if (!MTR_SAFE_CAST(nodeID, NSNumber)) {
        MTR_LOG_ERROR("%@ invalid device:receivedEventReport: nodeID: %@", self, nodeID);
        return;
    }

    if (!MTREventReportIsWellFormed(eventReport)) {
        MTR_LOG_ERROR("%@ invalid device:receivedEventReport: eventReport: %@", self, eventReport);
        return;
    }

    [self _lockAndCallDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
        [delegate device:self receivedEventReport:eventReport];
    }];
}

// optional methods for MTRDeviceDelegates - check for implementation before calling
- (oneway void)deviceBecameActive:(NSNumber *)nodeID
{
    MTR_LOG("%@ %s", self, __PRETTY_FUNCTION__);
    if (!MTR_SAFE_CAST(nodeID, NSNumber)) {
        MTR_LOG_ERROR("%@ invalid deviceBecameActive: nodeID: %@", self, nodeID);
        return;
    }

    [self _lockAndCallDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
        if ([delegate respondsToSelector:@selector(deviceBecameActive:)]) {
            [delegate deviceBecameActive:self];
        }
    }];
}

- (oneway void)deviceCachePrimed:(NSNumber *)nodeID
{
    // Not needed since this is a state update now
}

- (oneway void)deviceConfigurationChanged:(NSNumber *)nodeID
{
    MTR_LOG("%@ %s", self, __PRETTY_FUNCTION__);
    if (!MTR_SAFE_CAST(nodeID, NSNumber)) {
        MTR_LOG_ERROR("%@ invalid deviceConfigurationChanged: nodeID: %@", self, nodeID);
        return;
    }

    [self _lockAndCallDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
        if ([delegate respondsToSelector:@selector(deviceConfigurationChanged:)]) {
            [delegate deviceConfigurationChanged:self];
        }
    }];
}

static const auto * requiredInternalStateKeys = @{
    kMTRDeviceInternalPropertyDeviceState : NSNumber.class,
    kMTRDeviceInternalPropertyLastSubscriptionAttemptWait : NSNumber.class,
};

static const auto * optionalInternalStateKeys = @{
    kMTRDeviceInternalPropertyKeyVendorID : NSNumber.class,
    kMTRDeviceInternalPropertyKeyProductID : NSNumber.class,
    kMTRDeviceInternalPropertyNetworkFeatures : NSNumber.class,
    kMTRDeviceInternalPropertyMostRecentReportTime : NSDate.class,
    kMTRDeviceInternalPropertyLastSubscriptionFailureTime : NSDate.class,
};

- (BOOL)_ensureValidValuesForKeys:(const NSDictionary<NSString *, Class> *)keys inInternalState:(NSMutableDictionary *)internalState valueRequired:(BOOL)required
{
    for (NSString * key in keys) {
        id value = internalState[key];
        if (!value) {
            if (required) {
                MTR_LOG_ERROR("%@ device:internalStateUpdated: handed state with no value for \"%@\": %@", self, key, value);
                return NO;
            }

            continue;
        }
        if (![value isKindOfClass:keys[key]]) {
            MTR_LOG_ERROR("%@ device:internalStateUpdated: handed state with invalid value of type %@ for \"%@\": %@", self,
                NSStringFromClass([value class]), key, value);
            if (required) {
                return NO;
            }

            // If an optional value is invalid, just drop it and press on with
            // the other parts of the state, so we don't break those pieces
            // just because something optional has gone awry.
            [internalState removeObjectForKey:key];
        }
    }

    return YES;
}

- (oneway void)device:(NSNumber *)nodeID internalStateUpdated:(NSDictionary *)dictionary
{
    MTR_LOG("%@ %s", self, __PRETTY_FUNCTION__);
    if (!MTR_SAFE_CAST(nodeID, NSNumber)) {
        MTR_LOG_ERROR("%@ invalid device:internalStateUpdated: nodeID: %@", self, nodeID);
        return;
    }

    if (!MTR_SAFE_CAST(dictionary, NSDictionary)) {
        MTR_LOG_ERROR("%@ invalid device:internalStateUpdated dictionary: %@", self, dictionary);
        return;
    }

    [self _updateInternalState:[dictionary mutableCopy]];
}

- (void)_updateInternalState:(NSMutableDictionary *)newState
{
    VerifyOrReturn([self _ensureValidValuesForKeys:requiredInternalStateKeys inInternalState:newState valueRequired:YES]);
    VerifyOrReturn([self _ensureValidValuesForKeys:optionalInternalStateKeys inInternalState:newState valueRequired:NO]);

    NSNumber * oldStateNumber = MTR_SAFE_CAST(self._internalState[kMTRDeviceInternalPropertyDeviceState], NSNumber);
    NSNumber * newStateNumber = MTR_SAFE_CAST(newState[kMTRDeviceInternalPropertyDeviceState], NSNumber);

    NSNumber * oldPrimedState = MTR_SAFE_CAST(self._internalState[kMTRDeviceInternalPropertyDeviceCachePrimed], NSNumber);
    NSNumber * newPrimedState = MTR_SAFE_CAST(newState[kMTRDeviceInternalPropertyDeviceCachePrimed], NSNumber);

    [self _setInternalState:newState];

    if (!MTREqualObjects(oldStateNumber, newStateNumber)) {
        MTRDeviceState state = self.state;
        [self _lockAndCallDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
            [delegate device:self stateChanged:state];
        }];
    }

    if (!MTREqualObjects(oldPrimedState, newPrimedState)) {
        [self _lockAndCallDelegatesWithBlock:^(id<MTRDeviceDelegate> delegate) {
            if ([delegate respondsToSelector:@selector(deviceCachePrimed:)]) {
                [delegate deviceCachePrimed:self];
            }
        }];
    }
}

- (MTRDeviceState)state
{
    NSNumber * stateNumber = MTR_SAFE_CAST(self._internalState[kMTRDeviceInternalPropertyDeviceState], NSNumber);
    switch (static_cast<MTRDeviceState>(stateNumber.unsignedIntegerValue)) {
    case MTRDeviceStateUnknown:
        return MTRDeviceStateUnknown;

    case MTRDeviceStateUnreachable:
        return MTRDeviceStateUnreachable;

    case MTRDeviceStateReachable:
        return MTRDeviceStateReachable;
    }

    MTR_LOG_ERROR("stateNumber from internal state is an invalid value: %@", stateNumber);

    return MTRDeviceStateUnknown;
}

- (BOOL)deviceCachePrimed
{
    NSNumber * deviceCachePrimedNumber = MTR_SAFE_CAST(self._internalState[kMTRDeviceInternalPropertyDeviceCachePrimed], NSNumber);
    return deviceCachePrimedNumber.boolValue;
}

- (nullable NSDate *)estimatedStartTime
{
    return MTR_SAFE_CAST(self._internalState[kMTRDeviceInternalPropertyEstimatedStartTime], NSDate);
}

- (nullable NSNumber *)estimatedSubscriptionLatency
{
    return MTR_SAFE_CAST(self._internalState[kMTRDeviceInternalPropertyEstimatedSubscriptionLatency], NSNumber);
}

#pragma mark - Remote Commands

typedef NSDictionary<NSString *, id> * _Nullable ReadAttributeResponseType;
MTR_DEVICE_COMPLEX_REMOTE_XPC_GETTER(readAttributeWithEndpointID
                                     : (NSNumber *) endpointID clusterID
                                     : (NSNumber *) clusterID attributeID
                                     : (NSNumber *) attributeID params
                                     : (MTRReadParams * _Nullable) params,
                                     ReadAttributeResponseType,
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

typedef NSArray<NSDictionary<NSString *, id> *> * ReadAttributePathsResponseType;
MTR_DEVICE_COMPLEX_REMOTE_XPC_GETTER(readAttributePaths
                                     : (NSArray<MTRAttributeRequestPath *> *) attributePaths,
                                     ReadAttributePathsResponseType,
                                     [NSArray array], // Default return value
                                     readAttributePaths
                                     : attributePaths withReply)

- (void)_invokeCommandWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                           commandID:(NSNumber *)commandID
                       commandFields:(MTRDeviceDataValueDictionary)commandFields
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues
               expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval
                  timedInvokeTimeout:(NSNumber * _Nullable)timeout
         serverSideProcessingTimeout:(NSNumber * _Nullable)serverSideProcessingTimeout
                               queue:(dispatch_queue_t)queue
                          completion:(MTRDeviceResponseHandler)completion
{
    NSXPCConnection * xpcConnection = [(MTRDeviceController_XPC *) [self deviceController] xpcConnection];

    @try {
        [[xpcConnection remoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) {
            MTR_LOG_ERROR("Invoke error: %@", error);
            dispatch_async(queue, ^{
                completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
            });
        }] deviceController:[[self deviceController] uniqueIdentifier]
                                 nodeID:[self nodeID]
            invokeCommandWithEndpointID:endpointID
                              clusterID:clusterID
                              commandID:commandID
                          commandFields:commandFields
                         expectedValues:expectedValues
                  expectedValueInterval:expectedValueInterval
                     timedInvokeTimeout:timeout
            serverSideProcessingTimeout:serverSideProcessingTimeout
                             completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                 dispatch_async(queue, ^{
                                     if (values == nil && error == nil) {
                                         MTR_LOG_ERROR("%@ got invoke response for (%@, %@, %@) without values or error", self, endpointID, clusterID, commandID);
                                         completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT]);
                                         return;
                                     }

                                     if (error != nil && !MTR_SAFE_CAST(error, NSError)) {
                                         MTR_LOG_ERROR("%@ got invoke response for (%@, %@, %@) that has invalid error object: %@", self, endpointID, clusterID, commandID, error);
                                         completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT]);
                                         return;
                                     }

                                     if (values != nil && !MTRInvokeResponseIsWellFormed(values)) {
                                         MTR_LOG_ERROR("%@ got invoke response for (%@, %@, %@) that has invalid data: %@", self, clusterID, commandID, values, values);
                                         completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT]);
                                         return;
                                     }

                                     if (values != nil && error != nil) {
                                         MTR_LOG_ERROR("%@ got invoke response for (%@, %@, %@) with both values and error: %@, %@", self, endpointID, clusterID, commandID, values, error);
                                         // Just propagate through the error.
                                         completion(nil, error);
                                         return;
                                     }

                                     completion(values, error);
                                 });
                             }];
    } @catch (NSException * exception) {
        MTR_LOG_ERROR("Exception sending XPC message: %@", exception);
        dispatch_async(queue, ^{
            completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
        });
    }
}

- (void)invokeCommands:(NSArray<NSArray<MTRCommandWithRequiredResponse *> *> *)commands
                 queue:(dispatch_queue_t)queue
            completion:(MTRDeviceResponseHandler)completion
{
    NSXPCConnection * xpcConnection = [(MTRDeviceController_XPC *) [self deviceController] xpcConnection];

    @try {
        [[xpcConnection remoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) {
            MTR_LOG_ERROR("%@ Error in %@: %@", self, NSStringFromSelector(_cmd), error);
            dispatch_async(queue, ^{
                completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
            });
        }] deviceController:[[self deviceController] uniqueIdentifier]
                     nodeID:[self nodeID]
             invokeCommands:commands
                 completion:^(NSArray<MTRDeviceResponseValueDictionary> * _Nullable responses, NSError * _Nullable error) {
                     dispatch_async(queue, ^{
                         if (responses == nil && error == nil) {
                             MTR_LOG_ERROR("%@ got invoke responses for %@ without values or error", self, commands);
                             completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT]);
                             return;
                         }

                         if (error != nil && !MTR_SAFE_CAST(error, NSError)) {
                             MTR_LOG_ERROR("%@ got invoke responses for %@ that has invalid error object: %@", self, commands, error);
                             completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT]);
                             return;
                         }

                         if (responses != nil && !MTRInvokeResponsesAreWellFormed(responses)) {
                             MTR_LOG_ERROR("%@ got invoke responses for %@ that has invalid data: %@", self, commands, responses);
                             completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT]);
                             return;
                         }

                         if (responses != nil && error != nil) {
                             MTR_LOG_ERROR("%@ got invoke responses for %@ with both responses and error: %@, %@", self, commands, responses, error);
                             // Just propagate through the error.
                             completion(nil, error);
                             return;
                         }

                         if (error != nil) {
                             MTR_LOG_ERROR("%@ got error trying to invokeCommands: %@", self, error);
                         }

                         completion(responses, error);
                     });
                 }];
    } @catch (NSException * exception) {
        MTR_LOG_ERROR("%@ Exception sending XPC message for %@: %@", self, NSStringFromSelector(_cmd), exception);
        dispatch_async(queue, ^{
            completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
        });
    }
}

- (void)downloadLogOfType:(MTRDiagnosticLogType)type
                  timeout:(NSTimeInterval)timeout
                    queue:(dispatch_queue_t)queue
               completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
{
    NSXPCConnection * xpcConnection = [(MTRDeviceController_XPC *) [self deviceController] xpcConnection];

    @try {
        [[xpcConnection remoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) {
            MTR_LOG_ERROR("%@ Error in %@: %@", self, NSStringFromSelector(_cmd), error);
            dispatch_async(queue, ^{
                completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
            });
        }] deviceController:[[self deviceController] uniqueIdentifier]
                       nodeID:[self nodeID]
            downloadLogOfType:type
                      timeout:timeout
                   completion:^(NSURL * _Nullable url, NSError * _Nullable error) {
                       dispatch_async(queue, ^{
                           completion(url, error);
                           if (url) {
                               [[NSFileManager defaultManager] removeItemAtPath:url.path error:nil];
                           }
                       });
                   }];
    } @catch (NSException * exception) {
        MTR_LOG_ERROR("%@ Exception sending XPC messsage for %@: %@", self, NSStringFromSelector(_cmd), exception);
        dispatch_async(queue, ^{
            completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
        });
    }
}

// Not Supported via XPC
//- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID openCommissioningWindowWithSetupPasscode:(NSNumber *)setupPasscode discriminator:(NSNumber *)discriminator duration:(NSNumber *)duration completion:(MTRDeviceOpenCommissioningWindowHandler)completion;
//- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID openCommissioningWindowWithDiscriminator:(NSNumber *)discriminator duration:(NSNumber *)duration completion:(MTRDeviceOpenCommissioningWindowHandler)completion;

// Not Supported via XPC
// - (oneway void)downloadLogOfType:(MTRDiagnosticLogType)type nodeID:(NSNumber *)nodeID timeout:(NSTimeInterval)timeout completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion;

@end
