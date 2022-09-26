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

- (void)openBasicCommissioningWindow:(void (^_Nonnull)(bool))commissioningCompleteCallback
                            clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    commissioningWindowRequestedHandler:(void (^_Nonnull)(bool))commissioningWindowRequestedHandler
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

- (void)contentLauncher_launchUrl:(NSString * _Nonnull)contentUrl
                contentDisplayStr:(NSString * _Nonnull)contentDisplayStr
                 responseCallback:(void (^_Nonnull)(bool))responseCallback
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
               requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().contentLauncher_launchUrl() called");

    _contentLauncher_launchUrlResponseCallback = responseCallback;
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->ContentLauncherLaunchURL(
            [contentUrl UTF8String], [contentDisplayStr UTF8String], [](CHIP_ERROR err) {
                [CastingServerBridge getSharedInstance].contentLauncher_launchUrlResponseCallback(CHIP_NO_ERROR == err);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)levelControl_step:(uint8_t)stepMode
                 stepSize:(uint8_t)stepSize
           transitionTime:(uint16_t)transitionTime
               optionMask:(uint8_t)optionMask
           optionOverride:(uint8_t)optionOverride
         responseCallback:(void (^_Nonnull)(bool))responseCallback
              clientQueue:(dispatch_queue_t _Nonnull)clientQueue
       requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().levelControl_step() called");

    _levelControl_stepResponseCallback = responseCallback;
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err
            = CastingServer::GetInstance()->LevelControl_Step(static_cast<chip::app::Clusters::LevelControl::StepMode>(stepMode),
                stepSize, transitionTime, optionMask, optionOverride, [](CHIP_ERROR err) {
                    [CastingServerBridge getSharedInstance].levelControl_stepResponseCallback(CHIP_NO_ERROR == err);
                });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)levelControl_moveToLevel:(uint8_t)level
                  transitionTime:(uint16_t)transitionTime
                      optionMask:(uint8_t)optionMask
                  optionOverride:(uint8_t)optionOverride
                responseCallback:(void (^_Nonnull)(bool))responseCallback
                     clientQueue:(dispatch_queue_t _Nonnull)clientQueue
              requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().levelControl_moveToLevel() called");

    _levelControl_moveToLevelResponseCallback = responseCallback;
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->LevelControl_MoveToLevel(
            level, transitionTime, optionMask, optionOverride, [](CHIP_ERROR err) {
                [CastingServerBridge getSharedInstance].levelControl_moveToLevelResponseCallback(CHIP_NO_ERROR == err);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)mediaPlayback_play:(void (^_Nonnull)(bool))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_play() called");

    _mediaPlayback_playResponseCallback = responseCallback;
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_Play([](CHIP_ERROR err) {
            [CastingServerBridge getSharedInstance].mediaPlayback_playResponseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)mediaPlayback_pause:(void (^_Nonnull)(bool))responseCallback
                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
         requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_pause() called");

    _mediaPlayback_pauseResponseCallback = responseCallback;
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_Pause([](CHIP_ERROR err) {
            [CastingServerBridge getSharedInstance].mediaPlayback_pauseResponseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)mediaPlayback_stopPlayback:(void (^_Nonnull)(bool))responseCallback
                       clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_stopPlayback() called");

    _mediaPlayback_pauseResponseCallback = responseCallback;
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_StopPlayback([](CHIP_ERROR err) {
            [CastingServerBridge getSharedInstance].mediaPlayback_stopPlaybackResponseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)mediaPlayback_next:(void (^_Nonnull)(bool))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_next() called");

    _mediaPlayback_nextResponseCallback = responseCallback;
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_Next([](CHIP_ERROR err) {
            [CastingServerBridge getSharedInstance].mediaPlayback_nextResponseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)mediaPlayback_seek:(uint8_t)position
          responseCallback:(void (^_Nonnull)(bool))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_seek() called");

    _mediaPlayback_seekResponseCallback = responseCallback;
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_Seek(position, [](CHIP_ERROR err) {
            [CastingServerBridge getSharedInstance].mediaPlayback_seekResponseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)mediaPlayback_skipForward:(uint64_t)deltaPositionMilliseconds
                 responseCallback:(void (^_Nonnull)(bool))responseCallback
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
               requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_skipForward() called");

    _mediaPlayback_skipForwardResponseCallback = responseCallback;
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SkipForward(deltaPositionMilliseconds, [](CHIP_ERROR err) {
            [CastingServerBridge getSharedInstance].mediaPlayback_skipForwardResponseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)mediaPlayback_skipBackward:(uint64_t)deltaPositionMilliseconds
                  responseCallback:(void (^_Nonnull)(bool))responseCallback
                       clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_skipBackward() called");

    _mediaPlayback_skipBackwardResponseCallback = responseCallback;
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SkipBackward(deltaPositionMilliseconds, [](CHIP_ERROR err) {
            [CastingServerBridge getSharedInstance].mediaPlayback_skipBackwardResponseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)applicationLauncher_launchApp:(uint16_t)catalogVendorId
                        applicationId:(NSString * _Nonnull)applicationId
                                 data:(NSData * _Nullable)data
                     responseCallback:(void (^_Nonnull)(bool))responseCallback
                          clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                   requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationLauncher_launchApp() called");

    _applicationLauncher_launchAppResponseCallback = responseCallback;

    chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application;
    application.catalogVendorId = catalogVendorId;
    application.applicationId = chip::CharSpan::fromCharString([applicationId UTF8String]);

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationLauncher_LaunchApp(application,
            chip::MakeOptional(chip::ByteSpan(static_cast<const uint8_t *>(data.bytes), data.length)), [](CHIP_ERROR err) {
                [CastingServerBridge getSharedInstance].applicationLauncher_launchAppResponseCallback(CHIP_NO_ERROR == err);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)applicationLauncher_stopApp:(uint16_t)catalogVendorId
                      applicationId:(NSString * _Nonnull)applicationId
                   responseCallback:(void (^_Nonnull)(bool))responseCallback
                        clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                 requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationLauncher_stopApp() called");

    _applicationLauncher_stopAppResponseCallback = responseCallback;

    chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application;
    application.catalogVendorId = catalogVendorId;
    application.applicationId = chip::CharSpan::fromCharString([applicationId UTF8String]);

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationLauncher_StopApp(application, [](CHIP_ERROR err) {
            [CastingServerBridge getSharedInstance].applicationLauncher_stopAppResponseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)applicationLauncher_hideApp:(uint16_t)catalogVendorId
                      applicationId:(NSString * _Nonnull)applicationId
                   responseCallback:(void (^_Nonnull)(bool))responseCallback
                        clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                 requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationLauncher_hideApp() called");

    _applicationLauncher_hideAppResponseCallback = responseCallback;

    chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application;
    application.catalogVendorId = catalogVendorId;
    application.applicationId = chip::CharSpan::fromCharString([applicationId UTF8String]);

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationLauncher_HideApp(application, [](CHIP_ERROR err) {
            [CastingServerBridge getSharedInstance].applicationLauncher_hideAppResponseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)targetNavigator_navigateTarget:(uint8_t)target
                                  data:(NSString * _Nullable)data
                      responseCallback:(void (^_Nonnull)(bool))responseCallback
                           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                    requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().targetNavigator_navigateTarget() called");

    _targetNavigator_navigateTargetResponseCallback = responseCallback;

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->TargetNavigator_NavigateTarget(
            target, chip::MakeOptional(chip::CharSpan::fromCharString([data UTF8String])), [](CHIP_ERROR err) {
                [CastingServerBridge getSharedInstance].targetNavigator_navigateTargetResponseCallback(CHIP_NO_ERROR == err);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)keypadInput_sendKey:(uint8_t)keyCode
           responseCallback:(void (^_Nonnull)(bool))responseCallback
                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
         requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().keypadInput_sendKey() called");

    _keypadInput_sendKeyResponseCallback = responseCallback;

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->KeypadInput_SendKey(
            static_cast<chip::app::Clusters::KeypadInput::CecKeyCode>(keyCode), [](CHIP_ERROR err) {
                [CastingServerBridge getSharedInstance].keypadInput_sendKeyResponseCallback(CHIP_NO_ERROR == err);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

@end
