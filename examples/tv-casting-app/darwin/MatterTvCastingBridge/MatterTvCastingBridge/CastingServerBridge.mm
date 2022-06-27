/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#import "CastingServerBridge.h"
#import "CastingServer.h"

#import "DiscoveredNodeDataConverter.hpp"
#import "OnboardingPayload.h"

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/CHIPMem.h>
#include <platform/PlatformManager.h>
#include <platform/TestOnlyCommissionableDataProvider.h>

@interface CastingServerBridge ()

// queue used to serialize all work performed by the CastingServerBridge
@property (atomic, readonly) dispatch_queue_t chipWorkQueue;

@end

@implementation CastingServerBridge

+ (CastingServerBridge * _Nullable)getSharedInstance
{
    static CastingServerBridge * instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

- (instancetype)init
{
    if (self = [super init]) {
        CHIP_ERROR err = chip::Platform::MemoryInit();
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "MemoryInit failed: %s", ErrorStr(err));
            return nil;
        }

        err = chip::DeviceLayer::PlatformMgr().InitChipStack();
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "InitChipStack failed: %s", ErrorStr(err));
            return nil;
        }

        chip::DeviceLayer::TestOnlyCommissionableDataProvider TestOnlyCommissionableDataProvider;
        uint32_t defaultTestPasscode = 0;
        VerifyOrDie(TestOnlyCommissionableDataProvider.GetSetupPasscode(defaultTestPasscode) == CHIP_NO_ERROR);
        uint16_t defaultTestSetupDiscriminator = 0;
        VerifyOrDie(TestOnlyCommissionableDataProvider.GetSetupDiscriminator(defaultTestSetupDiscriminator) == CHIP_NO_ERROR);
        _onboardingPayload = [[OnboardingPayload alloc] initWithSetupPasscode:defaultTestPasscode
                                                           setupDiscriminator:defaultTestSetupDiscriminator];

        // Initialize device attestation config
        SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());

        // Initialize device attestation verifier from a constant version
        {
            // TODO: Replace testingRootStore with a AttestationTrustStore that has the necessary official PAA roots available
            const chip::Credentials::AttestationTrustStore * testingRootStore = chip::Credentials::GetTestAttestationTrustStore();
            SetDeviceAttestationVerifier(GetDefaultDACVerifier(testingRootStore));
        }

        // init app Server
        static chip::CommonCaseDeviceServerInitParams initParams;
        err = initParams.InitializeStaticResourcesBeforeServerInit();
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "InitializeStaticResourcesBeforeServerInit failed: %s", ErrorStr(err));
            return nil;
        }
        err = chip::Server::GetInstance().Init(initParams);
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "chip::Server init failed: %s", ErrorStr(err));
            return nil;
        }

        _chipWorkQueue = chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue();

        chip::DeviceLayer::PlatformMgrImpl().StartEventLoopTask();

        CastingServer::GetInstance()->Init();
    }
    return self;
}

- (void)discoverCommissioners:(dispatch_queue_t _Nonnull)clientQueue
    discoveryRequestSentHandler:(nullable void (^)(bool))discoveryRequestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().discoverCommissioners() called");
    dispatch_async(_chipWorkQueue, ^{
        bool discoveryRequestStatus = true;
        CHIP_ERROR err = CastingServer::GetInstance()->DiscoverCommissioners();
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "CastingServerBridge().discoverCommissioners() failed: %" CHIP_ERROR_FORMAT, err.Format());
            discoveryRequestStatus = false;
        }

        dispatch_async(clientQueue, ^{
            discoveryRequestSentHandler(discoveryRequestStatus);
        });
    });
}

- (void)getDiscoveredCommissioner:(int)index
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    discoveredCommissionerHandler:(nullable void (^)(DiscoveredNodeData * _Nullable))discoveredCommissionerHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().getDiscoveredCommissioner() called");

    dispatch_async(_chipWorkQueue, ^{
        DiscoveredNodeData * commissioner = nil;
        const chip::Dnssd::DiscoveredNodeData * chipDiscoveredNodeData
            = CastingServer::GetInstance()->GetDiscoveredCommissioner(index);
        if (chipDiscoveredNodeData != nullptr) {
            commissioner = [DiscoveredNodeDataConverter convertToObjC:chipDiscoveredNodeData];
        }

        dispatch_async(clientQueue, ^{
            discoveredCommissionerHandler(commissioner);
        });
    });
}

- (void)sendUserDirectedCommissioningRequest:(NSString * _Nonnull)commissionerIpAddress
                            commissionerPort:(uint16_t)commissionerPort
                           platformInterface:(unsigned int)platformInterface
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                       udcRequestSentHandler:(nullable void (^)(bool))udcRequestSentHandler
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().sendUserDirectedCommissioningRequest() called with IP %s port %d platformInterface %d",
        [commissionerIpAddress UTF8String], commissionerPort, platformInterface);

    dispatch_async(_chipWorkQueue, ^{
        bool udcRequestStatus;
        chip::Inet::IPAddress commissionerAddrInet;
        if (chip::Inet::IPAddress::FromString([commissionerIpAddress UTF8String], commissionerAddrInet) == false) {
            ChipLogError(AppServer, "CastingServerBridge().sendUserDirectedCommissioningRequest() failed to parse IP address");
            udcRequestStatus = false;
        } else {
            chip::Inet::InterfaceId interfaceId = chip::Inet::InterfaceId(platformInterface);

            chip::Transport::PeerAddress commissionerPeerAddress
                = chip::Transport::PeerAddress::UDP(commissionerAddrInet, commissionerPort, interfaceId);

            CHIP_ERROR err = CastingServer::GetInstance()->SendUserDirectedCommissioningRequest(commissionerPeerAddress);
            if (err != CHIP_NO_ERROR) {
                ChipLogError(AppServer, "CastingServerBridge().sendUserDirectedCommissioningRequest() failed: %" CHIP_ERROR_FORMAT,
                    err.Format());
                udcRequestStatus = false;
            } else {
                udcRequestStatus = true;
            }
        }

        dispatch_async(clientQueue, ^{
            udcRequestSentHandler(udcRequestStatus);
        });
    });
}

- (OnboardingPayload *)getOnboardingPaylod
{
    return _onboardingPayload;
}

- (void)openBasicCommissioningWindow:(nullable void (^)(bool))commissioningCompleteCallback
                            clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    commissioningWindowRequestedHandler:(nullable void (^)(bool))commissioningWindowRequestedHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().openBasicCommissioningWindow() called");

    _commissioningCompleteCallback = commissioningCompleteCallback;
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->OpenBasicCommissioningWindow(
            [](CHIP_ERROR err) { [CastingServerBridge getSharedInstance].commissioningCompleteCallback(CHIP_NO_ERROR == err); });

        dispatch_async(clientQueue, ^{
            commissioningWindowRequestedHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)contentLauncherLaunchUrl:(NSString * _Nonnull)contentUrl
               contentDisplayStr:(NSString * _Nonnull)contentDisplayStr
       launchUrlResponseCallback:(nullable void (^)(bool))launchUrlResponseCallback
                     clientQueue:(dispatch_queue_t _Nonnull)clientQueue
     launchUrlRequestSentHandler:(nullable void (^)(bool))launchUrlRequestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().contentLauncherLaunchUrl() called");

    _launchUrlResponseCallback = launchUrlResponseCallback;
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err
            = CastingServer::GetInstance()->ContentLauncherLaunchURL([contentUrl UTF8String], [contentDisplayStr UTF8String],
                [](CHIP_ERROR err) { [CastingServerBridge getSharedInstance].launchUrlResponseCallback(CHIP_NO_ERROR == err); });
        dispatch_async(clientQueue, ^{
            launchUrlRequestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

@end
