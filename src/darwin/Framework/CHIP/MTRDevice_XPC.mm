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
#import "MTRDeviceController.h"
#import "MTRDeviceControllerDelegateBridge.h"
#import "MTRDeviceControllerFactory_Internal.h"
#import "MTRDeviceControllerLocalTestStorage.h"
#import "MTRDeviceControllerStartupParams.h"
#import "MTRDeviceControllerStartupParams_Internal.h"
#import "MTRDeviceControllerXPCParameters.h"
#import "MTRDeviceController_Concrete.h"
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
#import <setup_payload/ManualSetupPayloadGenerator.h>
#import <setup_payload/SetupPayload.h>
#import <zap-generated/MTRBaseClusters.h>

#import "MTRDeviceAttestationDelegateBridge.h"
#import "MTRDeviceConnectionBridge.h"

#include <platform/CHIPDeviceConfig.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/List.h>
#include <app/server/Dnssd.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/CommissioningWindowOpener.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <credentials/attestation_verifier/DacOnlyPartialAttestationVerifier.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <inet/InetInterface.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <platform/LockTracker.h>
#include <platform/PlatformManager.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <system/SystemClock.h>

#include <atomic>
#include <dns_sd.h>
#include <string>

#import <os/lock.h>

#define MTR_SIMPLE_REMOTE_XPC_GETTER(NAME, TYPE, DEFAULT_VALUE, GETTER_NAME) \
\
- (TYPE) NAME { \
    __block TYPE outValue = DEFAULT_VALUE; \
\
    NSXPCConnection * xpcConnection = nil; \
\
    [[xpcConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) { \
        MTR_LOG_ERROR("Error: %@", error); \
    }] deviceController:[[self deviceController] uniqueIdentifier] \
                   nodeID:[self nodeID] \
              GETTER_NAME:^(TYPE returnValue) { \
            outValue = returnValue; \
        }]; \
 \
    return outValue;\
} \


@implementation MTRDevice_XPC


MTR_SIMPLE_REMOTE_XPC_GETTER(state, MTRDeviceState, MTRDeviceStateUnknown, getStateWithReply)
MTR_SIMPLE_REMOTE_XPC_GETTER(deviceCachePrimed, BOOL, NO, getDeviceCachePrimedWithReply)
MTR_SIMPLE_REMOTE_XPC_GETTER(estimatedStartTime, NSDate *, nil, getEstimatedStartTimeWithReply)
MTR_SIMPLE_REMOTE_XPC_GETTER(estimatedSubscriptionLatency, NSNumber *, nil, getEstimatedSubscriptionLatencyWithReply)


- (NSDictionary<NSString *, id> *) readAttributeWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID params:(MTRReadParams * _Nullable)params  {
    __block NSDictionary<NSString *, id> * outValue = nil;

    NSXPCConnection * xpcConnection = nil;

    [[xpcConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) {
        MTR_LOG_ERROR("Error: %@", error);
    }] deviceController:[[self deviceController] uniqueIdentifier]
                 nodeID:[self nodeID]
        readAttributeWithEndpointID: endpointID
                          clusterID: clusterID
                        attributeID: attributeID
                             params: params // TODO: PARAMS NEEDS TO CONORM TO NSCODER
     withReply: ^(NSDictionary<NSString *, id> * returnValue) {
            outValue = returnValue;
        }];

    return outValue;
}

- (void) writeAttributeWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID value:(id)value expectedValueInterval:(NSNumber *)expectedValueInterval timedWriteTimeout:(NSNumber * _Nullable)timeout {
//    __block NSDictionary<NSString *, id> * outValue = nil;

    NSXPCConnection * xpcConnection = nil;

    [[xpcConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) {
        MTR_LOG_ERROR("Error: %@", error);
    }] deviceController:[[self deviceController] uniqueIdentifier]
                 nodeID:[self nodeID]
     writeAttributeWithEndpointID: endpointID clusterID: clusterID attributeID: attributeID value: value expectedValueInterval: expectedValueInterval timedWriteTimeout: timeout];
}



@end
