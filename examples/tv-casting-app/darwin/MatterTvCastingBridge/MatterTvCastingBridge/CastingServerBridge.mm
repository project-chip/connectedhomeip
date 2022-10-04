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
#import "MatterCallbacks.h"
#import "OnboardingPayload.h"

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/CHIPListUtils.h>
#include <lib/support/CHIPMem.h>
#include <platform/PlatformManager.h>
#include <platform/TestOnlyCommissionableDataProvider.h>

@interface CastingServerBridge ()

// queue used to serialize all work performed by the CastingServerBridge
@property (atomic, readonly) dispatch_queue_t chipWorkQueue;

@property OnboardingPayload * _Nonnull onboardingPayload;

@property void (^_Nonnull commissioningCompleteCallback)(bool);

@property NSMutableDictionary * commandResponseCallbacks;

@property NSMutableDictionary * subscriptionEstablishedCallbacks;

@property NSMutableDictionary * subscriptionReadSuccessCallbacks;

@property NSMutableDictionary * subscriptionReadFailureCallbacks;

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

        _commandResponseCallbacks = [NSMutableDictionary dictionary];
        _subscriptionEstablishedCallbacks = [NSMutableDictionary dictionary];
        _subscriptionReadSuccessCallbacks = [NSMutableDictionary dictionary];
        _subscriptionReadFailureCallbacks = [NSMutableDictionary dictionary];

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

    [_commandResponseCallbacks setObject:responseCallback forKey:@"contentLauncher_launchUrl"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->ContentLauncherLaunchURL(
            [contentUrl UTF8String], [contentDisplayStr UTF8String], [](CHIP_ERROR err) {
                void (^responseCallback)(bool) =
                    [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"contentLauncher_launchUrl"];
                responseCallback(CHIP_NO_ERROR == err);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)contentLauncher_launchContent:(ContentLauncher_ContentSearch * _Nonnull)contentSearch
                             autoPlay:(bool)autoPlay
                                 data:(NSString * _Nullable)data
                     responseCallback:(void (^_Nonnull)(bool))responseCallback
                          clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                   requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().contentLauncher_launchContent() called");

    [_commandResponseCallbacks setObject:responseCallback forKey:@"contentLauncher_launchContent"];

    // Make a copy of params before we go async.
    contentSearch = [contentSearch copy];
    data = [data copy];

    dispatch_async(_chipWorkQueue, ^{
        ListFreer listFreer;
        chip::app::Clusters::ContentLauncher::Structs::ContentSearch::Type cppSearch;
        if (contentSearch.parameterList.count > 0) {
            auto * parameterListHolder
                = new ListHolder<chip::app::Clusters::ContentLauncher::Structs::Parameter::Type>(contentSearch.parameterList.count);
            listFreer.add(parameterListHolder);

            int parameterIndex = 0;
            for (ContentLauncher_Parameter * parameter in contentSearch.parameterList) {
                int externalIdListIndex = 0;
                if (parameter.externalIDList != nil) {
                    auto * externalIdListHolder
                        = new ListHolder<chip::app::Clusters::ContentLauncher::Structs::AdditionalInfo::Type>(
                            parameter.externalIDList.count);
                    listFreer.add(externalIdListHolder);

                    for (ContentLauncher_AdditionalInfo * additionalInfo in parameter.externalIDList) {
                        externalIdListHolder->mList[externalIdListIndex].value = chip::CharSpan([additionalInfo.value UTF8String],
                            [additionalInfo.value lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
                        externalIdListHolder->mList[externalIdListIndex].name = chip::CharSpan([additionalInfo.name UTF8String],
                            [additionalInfo.name lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
                        externalIdListIndex++;
                    }
                    parameterListHolder->mList[parameterIndex].externalIDList = MakeOptional(
                        chip::app::DataModel::List<const chip::app::Clusters::ContentLauncher::Structs::AdditionalInfo::Type>(
                            externalIdListHolder->mList, parameter.externalIDList.count));
                } else {
                    parameterListHolder->mList[parameterIndex].externalIDList = chip::Optional<chip::app::DataModel::List<
                        const chip::app::Clusters::ContentLauncher::Structs::AdditionalInfo::Type>>::Missing();
                }

                parameterListHolder->mList[parameterIndex].type
                    = static_cast<chip::app::Clusters::ContentLauncher::ParameterEnum>(parameter.type);
                parameterListHolder->mList[parameterIndex].value = chip::CharSpan(
                    [parameter.value UTF8String], [parameter.value lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
                parameterIndex++;
                cppSearch.parameterList
                    = chip::app::DataModel::List<chip::app::Clusters::ContentLauncher::Structs::Parameter::Type>(
                        parameterListHolder->mList, contentSearch.parameterList.count);
            }
        }

        CHIP_ERROR err = CastingServer::GetInstance()->ContentLauncher_LaunchContent(cppSearch, autoPlay,
            MakeOptional(chip::CharSpan([data UTF8String], [data lengthOfBytesUsingEncoding:NSUTF8StringEncoding])),
            [](CHIP_ERROR err) {
                void (^responseCallback)(bool) = [[CastingServerBridge getSharedInstance].commandResponseCallbacks
                    objectForKey:@"contentLauncher_launchContent"];
                responseCallback(CHIP_NO_ERROR == err);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)contentLauncher_subscribeSupportedStreamingProtocols:(uint16_t)minInterval
                                                 maxInterval:(uint16_t)maxInterval
                                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                                          requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                                             successCallback:(void (^_Nonnull)(uint32_t))successCallback
                                             failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
                             subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().contentLauncher_subscribeSupportedStreamingProtocols() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"contentLauncher_subscribeSupportedStreamingProtocols"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"contentLauncher_subscribeSupportedStreamingProtocols"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback
                                          forKey:@"contentLauncher_subscribeSupportedStreamingProtocols"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->ContentLauncher_SubscribeToSupportedStreamingProtocols(
            nullptr,
            [](void * context,
                chip::app::Clusters::ContentLauncher::Attributes::SupportedStreamingProtocols::TypeInfo::DecodableArgType
                    supportedStreamingProtocols) {
                void (^callback)(uint32_t) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"contentLauncher_subscribeSupportedStreamingProtocols"];
                callback(supportedStreamingProtocols);
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"contentLauncher_subscribeSupportedStreamingProtocols"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"contentLauncher_subscribeSupportedStreamingProtocols"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
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

    [_commandResponseCallbacks setObject:responseCallback forKey:@"levelControl_step"];
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err
            = CastingServer::GetInstance()->LevelControl_Step(static_cast<chip::app::Clusters::LevelControl::StepMode>(stepMode),
                stepSize, transitionTime, optionMask, optionOverride, [](CHIP_ERROR err) {
                    void (^responseCallback)(bool) =
                        [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"levelControl_step"];
                    responseCallback(CHIP_NO_ERROR == err);
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

    [_commandResponseCallbacks setObject:responseCallback forKey:@"levelControl_moveToLevel"];
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->LevelControl_MoveToLevel(
            level, transitionTime, optionMask, optionOverride, [](CHIP_ERROR err) {
                void (^responseCallback)(bool) =
                    [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"levelControl_moveToLevel"];
                responseCallback(CHIP_NO_ERROR == err);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)levelControl_subscribeCurrentLevel:(uint16_t)minInterval
                               maxInterval:(uint16_t)maxInterval
                               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                        requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                           successCallback:(void (^_Nonnull)(NSNumber * _Nullable))successCallback
                           failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
           subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().levelControl_subscribeCurrentLevel() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"levelControl_subscribeCurrentLevel"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"levelControl_subscribeCurrentLevel"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"levelControl_subscribeCurrentLevel"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->LevelControl_SubscribeToCurrentLevel(
            nullptr,
            [](void * context,
                chip::app::Clusters::LevelControl::Attributes::CurrentLevel::TypeInfo::DecodableArgType currentLevel) {
                void (^callback)(NSNumber * _Nullable) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"levelControl_subscribeCurrentLevel"];
                callback(@(currentLevel.Value()));
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"levelControl_subscribeCurrentLevel"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"levelControl_subscribeCurrentLevel"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)levelControl_subscribeMinLevel:(uint16_t)minInterval
                           maxInterval:(uint16_t)maxInterval
                           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                       successCallback:(void (^_Nonnull)(uint8_t))successCallback
                       failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
       subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().levelControl_subscribeMinLevel() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"levelControl_subscribeMinLevel"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"levelControl_subscribeMinLevel"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"levelControl_subscribeMinLevel"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->LevelControl_SubscribeToMinLevel(
            nullptr,
            [](void * context, chip::app::Clusters::LevelControl::Attributes::MinLevel::TypeInfo::DecodableArgType minLevel) {
                void (^callback)(uint8_t) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"levelControl_subscribeMinLevel"];
                callback(minLevel);
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"levelControl_subscribeMinLevel"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"levelControl_subscribeMinLevel"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)levelControl_subscribeMaxLevel:(uint16_t)minInterval
                           maxInterval:(uint16_t)maxInterval
                           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                       successCallback:(void (^_Nonnull)(uint8_t))successCallback
                       failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
       subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().levelControl_subscribeMaxLevel() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"levelControl_subscribeMaxLevel"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"levelControl_subscribeMaxLevel"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"levelControl_subscribeMaxLevel"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->LevelControl_SubscribeToMaxLevel(
            nullptr,
            [](void * context, chip::app::Clusters::LevelControl::Attributes::MaxLevel::TypeInfo::DecodableArgType maxLevel) {
                void (^callback)(uint8_t) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"levelControl_subscribeMaxLevel"];
                callback(maxLevel);
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"levelControl_subscribeMaxLevel"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"levelControl_subscribeMaxLevel"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)mediaPlayback_play:(void (^_Nonnull)(bool))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_play() called");

    [_commandResponseCallbacks setObject:responseCallback forKey:@"mediaPlayback_play"];
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_Play([](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"mediaPlayback_play"];
            responseCallback(CHIP_NO_ERROR == err);
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

    [_commandResponseCallbacks setObject:responseCallback forKey:@"mediaPlayback_pause"];
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_Pause([](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"mediaPlayback_pause"];
            responseCallback(CHIP_NO_ERROR == err);
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

    [_commandResponseCallbacks setObject:responseCallback forKey:@"mediaPlayback_stopPlayback"];
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_StopPlayback([](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"mediaPlayback_stopPlayback"];
            responseCallback(CHIP_NO_ERROR == err);
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

    [_commandResponseCallbacks setObject:responseCallback forKey:@"mediaPlayback_next"];
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_Next([](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"mediaPlayback_next"];
            responseCallback(CHIP_NO_ERROR == err);
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

    [_commandResponseCallbacks setObject:responseCallback forKey:@"mediaPlayback_seek"];
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_Seek(position, [](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"mediaPlayback_seek"];
            responseCallback(CHIP_NO_ERROR == err);
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

    [_commandResponseCallbacks setObject:responseCallback forKey:@"mediaPlayback_skipForward"];
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SkipForward(deltaPositionMilliseconds, [](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"mediaPlayback_skipForward"];
            responseCallback(CHIP_NO_ERROR == err);
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

    [_commandResponseCallbacks setObject:responseCallback forKey:@"mediaPlayback_skipBackward"];
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SkipBackward(deltaPositionMilliseconds, [](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"mediaPlayback_skipBackward"];
            responseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)mediaPlayback_subscribeCurrentState:(uint16_t)minInterval
                                maxInterval:(uint16_t)maxInterval
                                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                            successCallback:(void (^_Nonnull)(MediaPlayback_PlaybackState))successCallback
                            failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
            subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_subscribeCurrentState() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeCurrentState"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeCurrentState"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeCurrentState"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToCurrentState(
            nullptr,
            [](void * context,
                chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo::DecodableArgType currentState) {
                void (^callback)(MediaPlayback_PlaybackState) =
                    [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                        objectForKey:@"mediaPlayback_subscribeCurrentState"];
                callback(MediaPlayback_PlaybackState(currentState));
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"mediaPlayback_subscribeCurrentState"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"mediaPlayback_subscribeCurrentState"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)mediaPlayback_subscribeStartTime:(uint16_t)minInterval
                             maxInterval:(uint16_t)maxInterval
                             clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                      requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                         successCallback:(void (^_Nonnull)(NSNumber * _Nullable))successCallback
                         failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
         subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_subscribeStartTime() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeStartTime"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeStartTime"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeStartTime"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToStartTime(
            nullptr,
            [](void * context, chip::app::Clusters::MediaPlayback::Attributes::StartTime::TypeInfo::DecodableArgType startTime) {
                void (^callback)(NSNumber * _Nullable) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"mediaPlayback_subscribeStartTime"];
                callback(@(startTime.Value()));
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"mediaPlayback_subscribeStartTime"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"mediaPlayback_subscribeStartTime"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)mediaPlayback_subscribeDuration:(uint16_t)minInterval
                            maxInterval:(uint16_t)maxInterval
                            clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                     requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                        successCallback:(void (^_Nonnull)(NSNumber * _Nullable))successCallback
                        failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
        subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_subscribeDuration() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeDuration"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeDuration"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeDuration"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToDuration(
            nullptr,
            [](void * context, chip::app::Clusters::MediaPlayback::Attributes::Duration::TypeInfo::DecodableArgType startTime) {
                void (^callback)(NSNumber * _Nullable) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"mediaPlayback_subscribeDuration"];
                callback(@(startTime.Value()));
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"mediaPlayback_subscribeDuration"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"mediaPlayback_subscribeDuration"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)mediaPlayback_subscribeSampledPosition:(uint16_t)minInterval
                                   maxInterval:(uint16_t)maxInterval
                                   clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                            requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                               successCallback:(void (^_Nonnull)(MediaPlayback_PlaybackPosition * _Nullable))successCallback
                               failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
               subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_subscribeSampledPosition() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeSampledPosition"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeSampledPosition"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeSampledPosition"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToSampledPosition(
            nullptr,
            [](void * context,
                chip::app::Clusters::MediaPlayback::Attributes::SampledPosition::TypeInfo::DecodableArgType playbackPosition) {
                void (^callback)(MediaPlayback_PlaybackPosition * _Nullable) =
                    [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                        objectForKey:@"mediaPlayback_subscribeSampledPosition"];
                MediaPlayback_PlaybackPosition * objCPlaybackPosition = nil;
                if (!playbackPosition.IsNull()) {
                    if (playbackPosition.Value().position.IsNull()) {
                        objCPlaybackPosition =
                            [[MediaPlayback_PlaybackPosition alloc] initWithUpdatedAt:@(playbackPosition.Value().updatedAt)
                                                                             position:nil];
                    } else {
                        objCPlaybackPosition =
                            [[MediaPlayback_PlaybackPosition alloc] initWithUpdatedAt:@(playbackPosition.Value().updatedAt)
                                                                             position:@(playbackPosition.Value().position.Value())];
                    }
                    callback(objCPlaybackPosition);
                }
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"mediaPlayback_subscribeSampledPosition"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"mediaPlayback_subscribeSampledPosition"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)mediaPlayback_subscribePlaybackSpeed:(uint16_t)minInterval
                                 maxInterval:(uint16_t)maxInterval
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                          requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                             successCallback:(void (^_Nonnull)(float))successCallback
                             failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
             subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_subscribePlaybackSpeed() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribePlaybackSpeed"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribePlaybackSpeed"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribePlaybackSpeed"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToPlaybackSpeed(
            nullptr,
            [](void * context,
                chip::app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::TypeInfo::DecodableArgType playbackSpeed) {
                void (^callback)(float) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"mediaPlayback_subscribePlaybackSpeed"];
                callback(playbackSpeed);
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"mediaPlayback_subscribePlaybackSpeed"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"mediaPlayback_subscribePlaybackSpeed"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)mediaPlayback_subscribeSeekRangeEnd:(uint16_t)minInterval
                                maxInterval:(uint16_t)maxInterval
                                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                            successCallback:(void (^_Nonnull)(NSNumber * _Nullable))successCallback
                            failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
            subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_subscribeSeekRangeEnd() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeSeekRangeEnd"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeSeekRangeEnd"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeSeekRangeEnd"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToDuration(
            nullptr,
            [](void * context,
                chip::app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::TypeInfo::DecodableArgType seekRangeEnd) {
                void (^callback)(NSNumber * _Nullable) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"mediaPlayback_subscribeSeekRangeEnd"];
                callback(@(seekRangeEnd.Value()));
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"mediaPlayback_subscribeSeekRangeEnd"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"mediaPlayback_subscribeSeekRangeEnd"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)mediaPlayback_subscribeSeekRangeStart:(uint16_t)minInterval
                                  maxInterval:(uint16_t)maxInterval
                                  clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                           requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                              successCallback:(void (^_Nonnull)(NSNumber * _Nullable))successCallback
                              failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
              subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_subscribeSeekRangeStart() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeSeekRangeStart"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeSeekRangeStart"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeSeekRangeStart"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToDuration(
            nullptr,
            [](void * context,
                chip::app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::TypeInfo::DecodableArgType seekRangeStart) {
                void (^callback)(NSNumber * _Nullable) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"mediaPlayback_subscribeSeekRangeStart"];
                callback(@(seekRangeStart.Value()));
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"mediaPlayback_subscribeSeekRangeStart"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"mediaPlayback_subscribeSeekRangeStart"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
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

    [_commandResponseCallbacks setObject:responseCallback forKey:@"applicationLauncher_launchApp"];

    chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application;
    application.catalogVendorId = catalogVendorId;
    application.applicationId = chip::CharSpan::fromCharString([applicationId UTF8String]);

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationLauncher_LaunchApp(application,
            chip::MakeOptional(chip::ByteSpan(static_cast<const uint8_t *>(data.bytes), data.length)), [](CHIP_ERROR err) {
                void (^responseCallback)(bool) = [[CastingServerBridge getSharedInstance].commandResponseCallbacks
                    objectForKey:@"applicationLauncher_launchApp"];
                responseCallback(CHIP_NO_ERROR == err);
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

    [_commandResponseCallbacks setObject:responseCallback forKey:@"applicationLauncher_stopApp"];

    chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application;
    application.catalogVendorId = catalogVendorId;
    application.applicationId = chip::CharSpan::fromCharString([applicationId UTF8String]);

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationLauncher_StopApp(application, [](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"applicationLauncher_stopApp"];
            responseCallback(CHIP_NO_ERROR == err);
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

    [_commandResponseCallbacks setObject:responseCallback forKey:@"applicationLauncher_hideApp"];

    chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application;
    application.catalogVendorId = catalogVendorId;
    application.applicationId = chip::CharSpan::fromCharString([applicationId UTF8String]);

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationLauncher_HideApp(application, [](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"applicationLauncher_hideApp"];
            responseCallback(CHIP_NO_ERROR == err);
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

    [_commandResponseCallbacks setObject:responseCallback forKey:@"targetNavigator_navigateTarget"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->TargetNavigator_NavigateTarget(
            target, chip::MakeOptional(chip::CharSpan::fromCharString([data UTF8String])), [](CHIP_ERROR err) {
                void (^responseCallback)(bool) = [[CastingServerBridge getSharedInstance].commandResponseCallbacks
                    objectForKey:@"targetNavigator_navigateTarget"];
                responseCallback(CHIP_NO_ERROR == err);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)targetNavigator_subscribeTargetList:(uint16_t)minInterval
                                maxInterval:(uint16_t)maxInterval
                                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                            successCallback:(void (^_Nonnull)(NSMutableArray *))successCallback
                            failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
            subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().targetNavigator_subscribeTargetList() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"targetNavigator_subscribeTargetList"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"targetNavigator_subscribeTargetList"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"targetNavigator_subscribeTargetList"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->TargetNavigator_SubscribeToTargetList(
            nullptr,
            [](void * context,
                chip::app::Clusters::TargetNavigator::Attributes::TargetList::TypeInfo::DecodableArgType targetList) {
                void (^callback)(NSMutableArray *) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"targetNavigator_subscribeTargetList"];
                NSMutableArray * objCTargetList = nil;
                size_t targetInfoCount;
                targetList.ComputeSize(&targetInfoCount);
                if (targetInfoCount > 0) {
                    objCTargetList = [NSMutableArray arrayWithCapacity:targetInfoCount];
                    auto iter = targetList.begin();
                    while (iter.Next()) {
                        const chip::app::Clusters::TargetNavigator::Structs::TargetInfo::DecodableType & targetInfo
                            = iter.GetValue();
                        TargetNavigator_TargetInfo * objCTargetInfo = [[TargetNavigator_TargetInfo alloc]
                            initWithIdentifier:@(targetInfo.identifier)
                                          name:[NSString stringWithUTF8String:targetInfo.name.data()]];
                        [objCTargetList addObject:objCTargetInfo];
                    }
                }
                callback(objCTargetList);
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"targetNavigator_subscribeTargetList"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"targetNavigator_subscribeTargetList"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)targetNavigator_subscribeCurrentTarget:(uint16_t)minInterval
                                   maxInterval:(uint16_t)maxInterval
                                   clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                            requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                               successCallback:(void (^_Nonnull)(uint8_t))successCallback
                               failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
               subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().targetNavigator_subscribeCurrentTarget() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"targetNavigator_subscribeCurrentTarget"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"targetNavigator_subscribeCurrentTarget"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"targetNavigator_subscribeCurrentTarget"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->TargetNavigator_SubscribeToCurrentTarget(
            nullptr,
            [](void * context,
                chip::app::Clusters::TargetNavigator::Attributes::CurrentTarget::TypeInfo::DecodableArgType currentTarget) {
                void (^callback)(uint8_t) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"targetNavigator_subscribeCurrentTarget"];
                callback(currentTarget);
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"targetNavigator_subscribeCurrentTarget"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"targetNavigator_subscribeCurrentTarget"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)keypadInput_sendKey:(uint8_t)keyCode
           responseCallback:(void (^_Nonnull)(bool))responseCallback
                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
         requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().keypadInput_sendKey() called");

    [_commandResponseCallbacks setObject:responseCallback forKey:@"keypadInput_sendKey"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->KeypadInput_SendKey(
            static_cast<chip::app::Clusters::KeypadInput::CecKeyCode>(keyCode), [](CHIP_ERROR err) {
                void (^responseCallback)(bool) =
                    [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"keypadInput_sendKey"];
                responseCallback(CHIP_NO_ERROR == err);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)applicationBasic_subscribeVendorName:(uint16_t)minInterval
                                 maxInterval:(uint16_t)maxInterval
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                          requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                             successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                             failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
             subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_subscribeVendorName() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_subscribeVendorName"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_subscribeVendorName"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"applicationBasic_subscribeVendorName"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToVendorName(
            nullptr,
            [](void * context,
                chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo::DecodableArgType vendorName) {
                void (^callback)(NSString * _Nonnull) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"applicationBasic_subscribeVendorName"];
                callback([NSString stringWithUTF8String:vendorName.data()]);
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"applicationBasic_subscribeVendorName"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"applicationBasic_subscribeVendorName"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)applicationBasic_subscribeVendorID:(uint16_t)minInterval
                               maxInterval:(uint16_t)maxInterval
                               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                        requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                           successCallback:(void (^_Nonnull)(NSNumber * _Nonnull))successCallback
                           failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
           subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_subscribeVendorID() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_subscribeVendorID"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_subscribeVendorID"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"applicationBasic_subscribeVendorID"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToVendorID(
            nullptr,
            [](void * context, chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType vendorID) {
                void (^callback)(NSNumber * _Nonnull) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"applicationBasic_subscribeVendorID"];
                callback(@(vendorID));
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"applicationBasic_subscribeVendorID"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"applicationBasic_subscribeVendorID"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)applicationBasic_subscribeApplicationName:(uint16_t)minInterval
                                      maxInterval:(uint16_t)maxInterval
                                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                               requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                                  successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                                  failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
                  subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_subscribeApplicationName() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_subscribeApplicationName"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_subscribeApplicationName"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback
                                          forKey:@"applicationBasic_subscribeApplicationName"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToApplicationName(
            nullptr,
            [](void * context,
                chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo::DecodableArgType applicationName) {
                void (^callback)(NSString * _Nonnull) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"applicationBasic_subscribeApplicationName"];
                callback([NSString stringWithUTF8String:applicationName.data()]);
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"applicationBasic_subscribeApplicationName"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"applicationBasic_subscribeApplicationName"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)applicationBasic_subscribeProductID:(uint16_t)minInterval
                                maxInterval:(uint16_t)maxInterval
                                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                            successCallback:(void (^_Nonnull)(uint16_t))successCallback
                            failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
            subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_subscribeProductID() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_subscribeProductID"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_subscribeProductID"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"applicationBasic_subscribeProductID"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToProductID(
            nullptr,
            [](void * context, chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo::DecodableArgType productID) {
                void (^callback)(uint16_t) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"applicationBasic_subscribeProductID"];
                callback(productID);
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"applicationBasic_subscribeProductID"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"applicationBasic_subscribeProductID"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)applicationBasic_subscribeApplicationVersion:(uint16_t)minInterval
                                         maxInterval:(uint16_t)maxInterval
                                         clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                                  requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                                     successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                                     failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
                     subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_subscribeApplicationVersion() called");

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_subscribeApplicationVersion"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_subscribeApplicationVersion"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback
                                          forKey:@"applicationBasic_subscribeApplicationVersion"];

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToApplicationVersion(
            nullptr,
            [](void * context,
                chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo::DecodableArgType
                    applicationVersion) {
                void (^callback)(NSString * _Nonnull) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"applicationBasic_subscribeApplicationVersion"];
                callback([NSString stringWithUTF8String:applicationVersion.data()]);
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"applicationBasic_subscribeApplicationVersion"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            minInterval, maxInterval,
            [](void * context) {
                void (^callback)() = [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                    objectForKey:@"applicationBasic_subscribeApplicationVersion"];
                callback();
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

@end
