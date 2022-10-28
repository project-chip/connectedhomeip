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

#import "ConversionUtils.hpp"
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

@property void (^_Nonnull onConnectionSuccessCallback)(VideoPlayer *);

@property void (^_Nonnull onConnectionFailureCallback)(MatterError *);

@property void (^_Nonnull onNewOrUpdatedEndpointCallback)(ContentApp *);

@property NSMutableDictionary * commandResponseCallbacks;

@property NSMutableDictionary * subscriptionEstablishedCallbacks;

@property NSMutableDictionary * subscriptionReadSuccessCallbacks;

@property NSMutableDictionary * subscriptionReadFailureCallbacks;

@property NSMutableDictionary * readSuccessCallbacks;

@property NSMutableDictionary * readFailureCallbacks;

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
    }
    return self;
}

- (void)initApp:(AppParameters * _Nullable)appParameters
             clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    initAppStatusHandler:(nullable void (^)(bool))initAppStatusHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().initApp() called");

    dispatch_async(_chipWorkQueue, ^{
        bool initAppStatus = true;

        AppParams appParams;

        CHIP_ERROR err = CastingServer::GetInstance()->Init();
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "CastingServerBridge().initApp() failed: %" CHIP_ERROR_FORMAT, err.Format());
            initAppStatus = false;
        }

        dispatch_async(clientQueue, ^{
            initAppStatusHandler(initAppStatus);
        });
    });

    CastingServer::GetInstance()->Init();
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
        chip::Optional<TargetVideoPlayerInfo *> associatedConnectableVideoPlayer;
        DiscoveredNodeData * commissioner = nil;
        const chip::Dnssd::DiscoveredNodeData * cppDiscoveredNodeData
            = CastingServer::GetInstance()->GetDiscoveredCommissioner(index, associatedConnectableVideoPlayer);
        if (cppDiscoveredNodeData != nullptr) {
            commissioner = [ConversionUtils convertToObjCDiscoveredNodeDataFrom:cppDiscoveredNodeData];
            if (associatedConnectableVideoPlayer.HasValue()) {
                VideoPlayer * connectableVideoPlayer =
                    [ConversionUtils convertToObjCVideoPlayerFrom:associatedConnectableVideoPlayer.Value()];
                [commissioner setConnectableVideoPlayer:connectableVideoPlayer];
            }
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

- (void)sendUserDirectedCommissioningRequest:(DiscoveredNodeData * _Nonnull)commissioner
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                       udcRequestSentHandler:(nullable void (^)(bool))udcRequestSentHandler
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().sendUserDirectedCommissioningRequest() called with IP %s port %d platformInterface %d deviceName: "
        "%s",
        [commissioner.ipAddresses[0] UTF8String], commissioner.port, commissioner.platformInterface,
        [commissioner.deviceName UTF8String]);

    dispatch_async(_chipWorkQueue, ^{
        bool udcRequestStatus;

        chip::Dnssd::DiscoveredNodeData cppCommissioner;
        if ([ConversionUtils convertToCppDiscoveredNodeDataFrom:commissioner outDiscoveredNodeData:cppCommissioner]
            != CHIP_NO_ERROR) {
            ChipLogError(AppServer,
                "CastingServerBridge().sendUserDirectedCommissioningRequest() failed to convert Commissioner(DiscoveredNodeData) "
                "to Cpp type");
            udcRequestStatus = false;
        } else {
            CHIP_ERROR err = CastingServer::GetInstance()->SendUserDirectedCommissioningRequest(&cppCommissioner);
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

- (void)openBasicCommissioningWindow:(dispatch_queue_t _Nonnull)clientQueue
    commissioningWindowRequestedHandler:(void (^_Nonnull)(bool))commissioningWindowRequestedHandler
          commissioningCompleteCallback:(void (^_Nonnull)(bool))commissioningCompleteCallback
            onConnectionSuccessCallback:(void (^_Nonnull)(VideoPlayer * _Nonnull))onConnectionSuccessCallback
            onConnectionFailureCallback:(void (^_Nonnull)(MatterError * _Nonnull))onConnectionFailureCallback
         onNewOrUpdatedEndpointCallback:(void (^_Nonnull)(ContentApp * _Nonnull))onNewOrUpdatedEndpointCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().openBasicCommissioningWindow() called");

    _commissioningCompleteCallback = commissioningCompleteCallback;
    _onConnectionSuccessCallback = onConnectionSuccessCallback;
    _onConnectionFailureCallback = onConnectionFailureCallback;
    _onNewOrUpdatedEndpointCallback = onNewOrUpdatedEndpointCallback;

    CHIP_ERROR OpenBasicCommissioningWindow(std::function<void(CHIP_ERROR)> commissioningCompleteCallback,
        std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess, std::function<void(CHIP_ERROR)> onConnectionFailure,
        std::function<void(TargetEndpointInfo *)> onNewOrUpdatedEndpoint);

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->OpenBasicCommissioningWindow(
            [](CHIP_ERROR err) { [CastingServerBridge getSharedInstance].commissioningCompleteCallback(CHIP_NO_ERROR == err); },
            [](TargetVideoPlayerInfo * cppTargetVideoPlayerInfo) {
                VideoPlayer * videoPlayer = [ConversionUtils convertToObjCVideoPlayerFrom:cppTargetVideoPlayerInfo];
                [CastingServerBridge getSharedInstance].onConnectionSuccessCallback(videoPlayer);
            },
            [](CHIP_ERROR err) {
                [CastingServerBridge getSharedInstance].onConnectionFailureCallback(
                    [[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            [](TargetEndpointInfo * cppTargetEndpointInfo) {
                ContentApp * contentApp = [ConversionUtils convertToObjCContentAppFrom:cppTargetEndpointInfo];
                [CastingServerBridge getSharedInstance].onNewOrUpdatedEndpointCallback(contentApp);
            });

        dispatch_async(clientQueue, ^{
            commissioningWindowRequestedHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)getActiveTargetVideoPlayers:(dispatch_queue_t _Nonnull)clientQueue
    activeTargetVideoPlayersHandler:(nullable void (^)(NSMutableArray * _Nullable))activeTargetVideoPlayersHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().getActiveTargetVideoPlayers() called");

    dispatch_async(_chipWorkQueue, ^{
        NSMutableArray * videoPlayers = nil;
        TargetVideoPlayerInfo * cppTargetVideoPlayerInfo = CastingServer::GetInstance()->GetActiveTargetVideoPlayer();
        if (cppTargetVideoPlayerInfo != nullptr) {
            videoPlayers = [NSMutableArray new];
            videoPlayers[0] = [ConversionUtils convertToObjCVideoPlayerFrom:cppTargetVideoPlayerInfo];
        }

        dispatch_async(clientQueue, ^{
            activeTargetVideoPlayersHandler(videoPlayers);
        });
    });
}

- (void)readCachedVideoPlayers:(dispatch_queue_t _Nonnull)clientQueue
    readCachedVideoPlayersHandler:(nullable void (^)(NSMutableArray * _Nullable))readCachedVideoPlayersHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().readCachedVideoPlayers() called");

    dispatch_async(_chipWorkQueue, ^{
        NSMutableArray * videoPlayers = nil;
        TargetVideoPlayerInfo * cppTargetVideoPlayerInfos = CastingServer::GetInstance()->ReadCachedTargetVideoPlayerInfos();
        if (cppTargetVideoPlayerInfos != nullptr) {
            videoPlayers = [NSMutableArray new];
            for (size_t i = 0; cppTargetVideoPlayerInfos[i].IsInitialized(); i++) {
                ChipLogProgress(AppServer,
                    "CastingServerBridge().readCachedVideoPlayers() with nodeId: 0x" ChipLogFormatX64
                    " fabricIndex: %d deviceName: %s vendorId: %d",
                    ChipLogValueX64(cppTargetVideoPlayerInfos[i].GetNodeId()), cppTargetVideoPlayerInfos[i].GetFabricIndex(),
                    cppTargetVideoPlayerInfos[i].GetDeviceName(), cppTargetVideoPlayerInfos[i].GetVendorId());
                videoPlayers[i] = [ConversionUtils convertToObjCVideoPlayerFrom:&cppTargetVideoPlayerInfos[i]];
            }
        }

        dispatch_async(clientQueue, ^{
            readCachedVideoPlayersHandler(videoPlayers);
        });
    });
}

- (void)verifyOrEstablishConnection:(VideoPlayer * _Nonnull)videoPlayer
                        clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                 requestSentHandler:(nullable void (^)(MatterError * _Nonnull))requestSentHandler
        onConnectionSuccessCallback:(void (^_Nonnull)(VideoPlayer * _Nonnull))onConnectionSuccessCallback
        onConnectionFailureCallback:(void (^_Nonnull)(MatterError * _Nonnull))onConnectionFailureCallback
     onNewOrUpdatedEndpointCallback:(void (^_Nonnull)(ContentApp * _Nonnull))onNewOrUpdatedEndpointCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().verifyOrEstablishConnection() called");
    _onConnectionSuccessCallback = onConnectionSuccessCallback;
    _onConnectionFailureCallback = onConnectionFailureCallback;
    _onNewOrUpdatedEndpointCallback = onNewOrUpdatedEndpointCallback;

    dispatch_async(_chipWorkQueue, ^{
        TargetVideoPlayerInfo targetVideoPlayerInfo;
        [ConversionUtils convertToCppTargetVideoPlayerInfoFrom:videoPlayer outTargetVideoPlayerInfo:targetVideoPlayerInfo];

        CHIP_ERROR err = CastingServer::GetInstance()->VerifyOrEstablishConnection(
            targetVideoPlayerInfo,
            [](TargetVideoPlayerInfo * cppTargetVideoPlayerInfo) {
                VideoPlayer * videoPlayer = [ConversionUtils convertToObjCVideoPlayerFrom:cppTargetVideoPlayerInfo];
                [CastingServerBridge getSharedInstance].onConnectionSuccessCallback(videoPlayer);
            },
            [](CHIP_ERROR err) {
                [CastingServerBridge getSharedInstance].onConnectionFailureCallback(
                    [[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            },
            [](TargetEndpointInfo * cppTargetEndpointInfo) {
                ContentApp * contentApp = [ConversionUtils convertToObjCContentAppFrom:cppTargetEndpointInfo];
                [CastingServerBridge getSharedInstance].onNewOrUpdatedEndpointCallback(contentApp);
            });

        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)shutdownAllSubscriptions:(dispatch_queue_t _Nonnull)clientQueue requestSentHandler:(nullable void (^)())requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().shutdownAllSubscriptions() called");
    dispatch_async(_chipWorkQueue, ^{
        CastingServer::GetInstance()->ShutdownAllSubscriptions();
        dispatch_async(clientQueue, ^{
            requestSentHandler();
        });
    });
}

- (void)disconnect:(dispatch_queue_t _Nonnull)clientQueue requestSentHandler:(nullable void (^)())requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().disconnect() called");
    dispatch_async(_chipWorkQueue, ^{
        CastingServer::GetInstance()->Disconnect();
        dispatch_async(clientQueue, ^{
            requestSentHandler();
        });
    });
}

- (void)contentLauncher_launchUrl:(ContentApp * _Nonnull)contentApp
                       contentUrl:(NSString * _Nonnull)contentUrl
                contentDisplayStr:(NSString * _Nonnull)contentDisplayStr
                 responseCallback:(void (^_Nonnull)(bool))responseCallback
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
               requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().contentLauncher_launchUrl() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"contentLauncher_launchUrl"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];
        CHIP_ERROR err = CastingServer::GetInstance()->ContentLauncherLaunchURL(
            &endpoint, [contentUrl UTF8String], [contentDisplayStr UTF8String], [](CHIP_ERROR err) {
                void (^responseCallback)(bool) =
                    [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"contentLauncher_launchUrl"];
                responseCallback(CHIP_NO_ERROR == err);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)contentLauncher_launchContent:(ContentApp * _Nonnull)contentApp
                        contentSearch:(ContentLauncher_ContentSearch * _Nonnull)contentSearch
                             autoPlay:(bool)autoPlay
                                 data:(NSString * _Nullable)data
                     responseCallback:(void (^_Nonnull)(bool))responseCallback
                          clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                   requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().contentLauncher_launchContent() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"contentLauncher_launchContent"];

    // Make a copy of params before we go async.
    contentSearch = [contentSearch copy];
    data = [data copy];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

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

        CHIP_ERROR err = CastingServer::GetInstance()->ContentLauncher_LaunchContent(&endpoint, cppSearch, autoPlay,
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

- (void)contentLauncher_subscribeSupportedStreamingProtocols:(ContentApp * _Nonnull)contentApp
                                                 minInterval:(uint16_t)minInterval
                                                 maxInterval:(uint16_t)maxInterval
                                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                                          requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                                             successCallback:(void (^_Nonnull)(uint32_t))successCallback
                                             failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
                             subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().contentLauncher_subscribeSupportedStreamingProtocols() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"contentLauncher_subscribeSupportedStreamingProtocols"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"contentLauncher_subscribeSupportedStreamingProtocols"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback
                                          forKey:@"contentLauncher_subscribeSupportedStreamingProtocols"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->ContentLauncher_SubscribeToSupportedStreamingProtocols(
            &endpoint, nullptr,
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

- (void)levelControl_step:(ContentApp * _Nonnull)contentApp
                 stepMode:(uint8_t)stepMode
                 stepSize:(uint8_t)stepSize
           transitionTime:(uint16_t)transitionTime
               optionMask:(uint8_t)optionMask
           optionOverride:(uint8_t)optionOverride
         responseCallback:(void (^_Nonnull)(bool))responseCallback
              clientQueue:(dispatch_queue_t _Nonnull)clientQueue
       requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(
        AppServer, "CastingServerBridge().levelControl_step() called on Content App with endpoint ID %d", contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"levelControl_step"];
    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->LevelControl_Step(&endpoint,
            static_cast<chip::app::Clusters::LevelControl::StepMode>(stepMode), stepSize, transitionTime, optionMask,
            optionOverride, [](CHIP_ERROR err) {
                void (^responseCallback)(bool) =
                    [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"levelControl_step"];
                responseCallback(CHIP_NO_ERROR == err);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)levelControl_moveToLevel:(ContentApp * _Nonnull)contentApp
                           level:(uint8_t)level
                  transitionTime:(uint16_t)transitionTime
                      optionMask:(uint8_t)optionMask
                  optionOverride:(uint8_t)optionOverride
                responseCallback:(void (^_Nonnull)(bool))responseCallback
                     clientQueue:(dispatch_queue_t _Nonnull)clientQueue
              requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().levelControl_moveToLevel() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"levelControl_moveToLevel"];
    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->LevelControl_MoveToLevel(
            &endpoint, level, transitionTime, optionMask, optionOverride, [](CHIP_ERROR err) {
                void (^responseCallback)(bool) =
                    [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"levelControl_moveToLevel"];
                responseCallback(CHIP_NO_ERROR == err);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)levelControl_subscribeCurrentLevel:(ContentApp * _Nonnull)contentApp
                               minInterval:(uint16_t)minInterval
                               maxInterval:(uint16_t)maxInterval
                               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                        requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                           successCallback:(void (^_Nonnull)(NSNumber * _Nullable))successCallback
                           failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
           subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().levelControl_subscribeCurrentLevel() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"levelControl_subscribeCurrentLevel"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"levelControl_subscribeCurrentLevel"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"levelControl_subscribeCurrentLevel"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->LevelControl_SubscribeToCurrentLevel(
            &endpoint, nullptr,
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

- (void)levelControl_subscribeMinLevel:(ContentApp * _Nonnull)contentApp
                           minInterval:(uint16_t)minInterval
                           maxInterval:(uint16_t)maxInterval
                           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                       successCallback:(void (^_Nonnull)(uint8_t))successCallback
                       failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
       subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().levelControl_subscribeMinLevel() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"levelControl_subscribeMinLevel"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"levelControl_subscribeMinLevel"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"levelControl_subscribeMinLevel"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->LevelControl_SubscribeToMinLevel(
            &endpoint, nullptr,
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

- (void)levelControl_subscribeMaxLevel:(ContentApp * _Nonnull)contentApp
                           minInterval:(uint16_t)minInterval
                           maxInterval:(uint16_t)maxInterval
                           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                       successCallback:(void (^_Nonnull)(uint8_t))successCallback
                       failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
       subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().levelControl_subscribeMaxLevel() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"levelControl_subscribeMaxLevel"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"levelControl_subscribeMaxLevel"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"levelControl_subscribeMaxLevel"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->LevelControl_SubscribeToMaxLevel(
            &endpoint, nullptr,
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

- (void)mediaPlayback_play:(ContentApp * _Nonnull)contentApp
          responseCallback:(void (^_Nonnull)(bool))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(
        AppServer, "CastingServerBridge().mediaPlayback_play() called on Content App with endpoint ID %d", contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"mediaPlayback_play"];
    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_Play(&endpoint, [](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"mediaPlayback_play"];
            responseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)mediaPlayback_pause:(ContentApp * _Nonnull)contentApp
           responseCallback:(void (^_Nonnull)(bool))responseCallback
                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
         requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(
        AppServer, "CastingServerBridge().mediaPlayback_pause() called on Content App with endpoint ID %d", contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"mediaPlayback_pause"];
    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_Pause(&endpoint, [](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"mediaPlayback_pause"];
            responseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)mediaPlayback_stopPlayback:(ContentApp * _Nonnull)contentApp
                  responseCallback:(void (^_Nonnull)(bool))responseCallback
                       clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_stopPlayback() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"mediaPlayback_stopPlayback"];
    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_StopPlayback(&endpoint, [](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"mediaPlayback_stopPlayback"];
            responseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)mediaPlayback_next:(ContentApp * _Nonnull)contentApp
          responseCallback:(void (^_Nonnull)(bool))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(
        AppServer, "CastingServerBridge().mediaPlayback_next() called on Content App with endpoint ID %d", contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"mediaPlayback_next"];
    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_Next(&endpoint, [](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"mediaPlayback_next"];
            responseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)mediaPlayback_seek:(ContentApp * _Nonnull)contentApp
                  position:(uint8_t)position
          responseCallback:(void (^_Nonnull)(bool))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(
        AppServer, "CastingServerBridge().mediaPlayback_seek() called on Content App with endpoint ID %d", contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"mediaPlayback_seek"];
    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_Seek(&endpoint, position, [](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"mediaPlayback_seek"];
            responseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)mediaPlayback_skipForward:(ContentApp * _Nonnull)contentApp
        deltaPositionMilliseconds:(uint64_t)deltaPositionMilliseconds
                 responseCallback:(void (^_Nonnull)(bool))responseCallback
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
               requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_skipForward() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"mediaPlayback_skipForward"];
    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err
            = CastingServer::GetInstance()->MediaPlayback_SkipForward(&endpoint, deltaPositionMilliseconds, [](CHIP_ERROR err) {
                  void (^responseCallback)(bool) =
                      [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"mediaPlayback_skipForward"];
                  responseCallback(CHIP_NO_ERROR == err);
              });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)mediaPlayback_skipBackward:(ContentApp * _Nonnull)contentApp
         deltaPositionMilliseconds:(uint64_t)deltaPositionMilliseconds
                  responseCallback:(void (^_Nonnull)(bool))responseCallback
                       clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_skipBackward() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"mediaPlayback_skipBackward"];
    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err
            = CastingServer::GetInstance()->MediaPlayback_SkipBackward(&endpoint, deltaPositionMilliseconds, [](CHIP_ERROR err) {
                  void (^responseCallback)(bool) =
                      [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"mediaPlayback_skipBackward"];
                  responseCallback(CHIP_NO_ERROR == err);
              });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)mediaPlayback_subscribeCurrentState:(ContentApp * _Nonnull)contentApp
                                minInterval:(uint16_t)minInterval
                                maxInterval:(uint16_t)maxInterval
                                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                            successCallback:(void (^_Nonnull)(MediaPlayback_PlaybackState))successCallback
                            failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
            subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().mediaPlayback_subscribeCurrentState() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeCurrentState"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeCurrentState"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeCurrentState"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToCurrentState(
            &endpoint, nullptr,
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

- (void)mediaPlayback_subscribeStartTime:(ContentApp * _Nonnull)contentApp
                             minInterval:(uint16_t)minInterval
                             maxInterval:(uint16_t)maxInterval
                             clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                      requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                         successCallback:(void (^_Nonnull)(NSNumber * _Nullable))successCallback
                         failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
         subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_subscribeStartTime() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeStartTime"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeStartTime"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeStartTime"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToStartTime(
            &endpoint, nullptr,
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

- (void)mediaPlayback_subscribeDuration:(ContentApp * _Nonnull)contentApp
                            minInterval:(uint16_t)minInterval
                            maxInterval:(uint16_t)maxInterval
                            clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                     requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                        successCallback:(void (^_Nonnull)(NSNumber * _Nullable))successCallback
                        failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
        subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_subscribeDuration() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeDuration"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeDuration"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeDuration"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToDuration(
            &endpoint, nullptr,
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

- (void)mediaPlayback_subscribeSampledPosition:(ContentApp * _Nonnull)contentApp
                                   minInterval:(uint16_t)minInterval
                                   maxInterval:(uint16_t)maxInterval
                                   clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                            requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                               successCallback:(void (^_Nonnull)(MediaPlayback_PlaybackPosition * _Nullable))successCallback
                               failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
               subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().mediaPlayback_subscribeSampledPosition() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeSampledPosition"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeSampledPosition"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeSampledPosition"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToSampledPosition(
            &endpoint, nullptr,
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

- (void)mediaPlayback_subscribePlaybackSpeed:(ContentApp * _Nonnull)contentApp
                                 minInterval:(uint16_t)minInterval
                                 maxInterval:(uint16_t)maxInterval
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                          requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                             successCallback:(void (^_Nonnull)(float))successCallback
                             failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
             subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().mediaPlayback_subscribePlaybackSpeed() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribePlaybackSpeed"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribePlaybackSpeed"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribePlaybackSpeed"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToPlaybackSpeed(
            &endpoint, nullptr,
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

- (void)mediaPlayback_subscribeSeekRangeEnd:(ContentApp * _Nonnull)contentApp
                                minInterval:(uint16_t)minInterval
                                maxInterval:(uint16_t)maxInterval
                                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                            successCallback:(void (^_Nonnull)(NSNumber * _Nullable))successCallback
                            failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
            subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().mediaPlayback_subscribeSeekRangeEnd() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeSeekRangeEnd"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeSeekRangeEnd"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeSeekRangeEnd"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToDuration(
            &endpoint, nullptr,
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

- (void)mediaPlayback_subscribeSeekRangeStart:(ContentApp * _Nonnull)contentApp
                                  minInterval:(uint16_t)minInterval
                                  maxInterval:(uint16_t)maxInterval
                                  clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                           requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                              successCallback:(void (^_Nonnull)(NSNumber * _Nullable))successCallback
                              failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
              subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().mediaPlayback_subscribeSeekRangeStart() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeSeekRangeStart"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeSeekRangeStart"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeSeekRangeStart"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToDuration(
            &endpoint, nullptr,
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

- (void)applicationLauncher_launchApp:(ContentApp * _Nonnull)contentApp
                      catalogVendorId:(uint16_t)catalogVendorId
                        applicationId:(NSString * _Nonnull)applicationId
                                 data:(NSData * _Nullable)data
                     responseCallback:(void (^_Nonnull)(bool))responseCallback
                          clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                   requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationLauncher_launchApp() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"applicationLauncher_launchApp"];

    chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application;
    application.catalogVendorId = catalogVendorId;
    application.applicationId = chip::CharSpan::fromCharString([applicationId UTF8String]);

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationLauncher_LaunchApp(&endpoint, application,
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

- (void)applicationLauncher_stopApp:(ContentApp * _Nonnull)contentApp
                    catalogVendorId:(uint16_t)catalogVendorId
                      applicationId:(NSString * _Nonnull)applicationId
                   responseCallback:(void (^_Nonnull)(bool))responseCallback
                        clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                 requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationLauncher_stopApp() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"applicationLauncher_stopApp"];

    chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application;
    application.catalogVendorId = catalogVendorId;
    application.applicationId = chip::CharSpan::fromCharString([applicationId UTF8String]);

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationLauncher_StopApp(&endpoint, application, [](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"applicationLauncher_stopApp"];
            responseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)applicationLauncher_hideApp:(ContentApp * _Nonnull)contentApp
                    catalogVendorId:(uint16_t)catalogVendorId
                      applicationId:(NSString * _Nonnull)applicationId
                   responseCallback:(void (^_Nonnull)(bool))responseCallback
                        clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                 requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationLauncher_hideApp() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"applicationLauncher_hideApp"];

    chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application;
    application.catalogVendorId = catalogVendorId;
    application.applicationId = chip::CharSpan::fromCharString([applicationId UTF8String]);

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationLauncher_HideApp(&endpoint, application, [](CHIP_ERROR err) {
            void (^responseCallback)(bool) =
                [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"applicationLauncher_hideApp"];
            responseCallback(CHIP_NO_ERROR == err);
        });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)targetNavigator_navigateTarget:(ContentApp * _Nonnull)contentApp
                                target:(uint8_t)target
                                  data:(NSString * _Nullable)data
                      responseCallback:(void (^_Nonnull)(bool))responseCallback
                           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                    requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().targetNavigator_navigateTarget() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"targetNavigator_navigateTarget"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->TargetNavigator_NavigateTarget(
            &endpoint, target, chip::MakeOptional(chip::CharSpan::fromCharString([data UTF8String])), [](CHIP_ERROR err) {
                void (^responseCallback)(bool) = [[CastingServerBridge getSharedInstance].commandResponseCallbacks
                    objectForKey:@"targetNavigator_navigateTarget"];
                responseCallback(CHIP_NO_ERROR == err);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)targetNavigator_subscribeTargetList:(ContentApp * _Nonnull)contentApp
                                minInterval:(uint16_t)minInterval
                                maxInterval:(uint16_t)maxInterval
                                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                            successCallback:(void (^_Nonnull)(NSMutableArray *))successCallback
                            failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
            subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().targetNavigator_subscribeTargetList() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"targetNavigator_subscribeTargetList"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"targetNavigator_subscribeTargetList"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"targetNavigator_subscribeTargetList"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->TargetNavigator_SubscribeToTargetList(
            &endpoint, nullptr,
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

- (void)targetNavigator_subscribeCurrentTarget:(ContentApp * _Nonnull)contentApp
                                   minInterval:(uint16_t)minInterval
                                   maxInterval:(uint16_t)maxInterval
                                   clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                            requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                               successCallback:(void (^_Nonnull)(uint8_t))successCallback
                               failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
               subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().targetNavigator_subscribeCurrentTarget() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"targetNavigator_subscribeCurrentTarget"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"targetNavigator_subscribeCurrentTarget"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"targetNavigator_subscribeCurrentTarget"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->TargetNavigator_SubscribeToCurrentTarget(
            &endpoint, nullptr,
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

- (void)keypadInput_sendKey:(ContentApp * _Nonnull)contentApp
                    keyCode:(uint8_t)keyCode
           responseCallback:(void (^_Nonnull)(bool))responseCallback
                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
         requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    ChipLogProgress(
        AppServer, "CastingServerBridge().keypadInput_sendKey() called on Content App with endpoint ID %d", contentApp.endpointId);

    [_commandResponseCallbacks setObject:responseCallback forKey:@"keypadInput_sendKey"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->KeypadInput_SendKey(
            &endpoint, static_cast<chip::app::Clusters::KeypadInput::CecKeyCode>(keyCode), [](CHIP_ERROR err) {
                void (^responseCallback)(bool) =
                    [[CastingServerBridge getSharedInstance].commandResponseCallbacks objectForKey:@"keypadInput_sendKey"];
                responseCallback(CHIP_NO_ERROR == err);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler(CHIP_NO_ERROR == err);
        });
    });
}

- (void)applicationBasic_subscribeVendorName:(ContentApp * _Nonnull)contentApp
                                 minInterval:(uint16_t)minInterval
                                 maxInterval:(uint16_t)maxInterval
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                          requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                             successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                             failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
             subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().applicationBasic_subscribeVendorName() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_subscribeVendorName"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_subscribeVendorName"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"applicationBasic_subscribeVendorName"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToVendorName(
            &endpoint, nullptr,
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

- (void)applicationBasic_subscribeVendorID:(ContentApp * _Nonnull)contentApp
                               minInterval:(uint16_t)minInterval
                               maxInterval:(uint16_t)maxInterval
                               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                        requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                           successCallback:(void (^_Nonnull)(NSNumber * _Nonnull))successCallback
                           failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
           subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().applicationBasic_subscribeVendorID() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_subscribeVendorID"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_subscribeVendorID"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"applicationBasic_subscribeVendorID"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToVendorID(
            &endpoint, nullptr,
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

- (void)applicationBasic_subscribeApplicationName:(ContentApp * _Nonnull)contentApp
                                      minInterval:(uint16_t)minInterval
                                      maxInterval:(uint16_t)maxInterval
                                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                               requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                                  successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                                  failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
                  subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().applicationBasic_subscribeApplicationName() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_subscribeApplicationName"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_subscribeApplicationName"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback
                                          forKey:@"applicationBasic_subscribeApplicationName"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToApplicationName(
            &endpoint, nullptr,
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

- (void)applicationBasic_subscribeProductID:(ContentApp * _Nonnull)contentApp
                                minInterval:(uint16_t)minInterval
                                maxInterval:(uint16_t)maxInterval
                                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                            successCallback:(void (^_Nonnull)(uint16_t))successCallback
                            failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
            subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().applicationBasic_subscribeProductID() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_subscribeProductID"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_subscribeProductID"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"applicationBasic_subscribeProductID"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToProductID(
            &endpoint, nullptr,
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

- (void)applicationBasic_subscribeApplicationVersion:(ContentApp * _Nonnull)contentApp
                                         minInterval:(uint16_t)minInterval
                                         maxInterval:(uint16_t)maxInterval
                                         clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                                  requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                                     successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                                     failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
                     subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().applicationBasic_subscribeApplicationVersion() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_subscribeApplicationVersion"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_subscribeApplicationVersion"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback
                                          forKey:@"applicationBasic_subscribeApplicationVersion"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToApplicationVersion(
            &endpoint, nullptr,
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

- (void)applicationBasic_readVendorName:(ContentApp * _Nonnull)contentApp
                            clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                     requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                        successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                        failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_readVendorName() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_readSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_readVendorName"];
    [_readFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_readVendorName"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_ReadVendorName(
            &endpoint, nullptr,
            [](void * context,
                chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo::DecodableArgType vendorName) {
                void (^callback)(NSString * _Nonnull) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"applicationBasic_readVendorName"];
                callback([NSString stringWithUTF8String:vendorName.data()]);
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"applicationBasic_readVendorName"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)applicationBasic_readVendorID:(ContentApp * _Nonnull)contentApp
                          clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                   requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                      successCallback:(void (^_Nonnull)(NSNumber * _Nonnull))successCallback
                      failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_readVendorID() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_readSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_readVendorID"];
    [_readFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_readVendorID"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_ReadVendorID(
            &endpoint, nullptr,
            [](void * context, chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType vendorID) {
                void (^callback)(NSNumber * _Nonnull) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"applicationBasic_readVendorID"];
                callback(@(vendorID));
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"applicationBasic_readVendorID"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)applicationBasic_readApplicationName:(ContentApp * _Nonnull)contentApp
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                          requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                             successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                             failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().applicationBasic_readApplicationName() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_readSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_readApplicationName"];
    [_readFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_readApplicationName"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_ReadApplicationName(
            &endpoint, nullptr,
            [](void * context,
                chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo::DecodableArgType applicationName) {
                void (^callback)(NSString * _Nonnull) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"applicationBasic_readApplicationName"];
                callback([NSString stringWithUTF8String:applicationName.data()]);
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"applicationBasic_readApplicationName"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)applicationBasic_readProductID:(ContentApp * _Nonnull)contentApp
                           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                       successCallback:(void (^_Nonnull)(uint16_t))successCallback
                       failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_readProductID() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_readSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_readProductID"];
    [_readFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_readProductID"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_ReadProductID(
            &endpoint, nullptr,
            [](void * context, chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo::DecodableArgType productID) {
                void (^callback)(uint16_t) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"applicationBasic_readProductID"];
                callback(productID);
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"applicationBasic_readProductID"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)applicationBasic_readApplicationVersion:(ContentApp * _Nonnull)contentApp
                                    clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                             requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                                successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                                failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().applicationBasic_readApplicationVersion() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_readSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_readApplicationVersion"];
    [_readFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_readApplicationVersion"];

    dispatch_async(_chipWorkQueue, ^{
        TargetEndpointInfo endpoint;
        [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

        CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_ReadApplicationVersion(
            &endpoint, nullptr,
            [](void * context,
                chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo::DecodableArgType
                    applicationVersion) {
                void (^callback)(NSString * _Nonnull) = [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                    objectForKey:@"applicationBasic_readApplicationVersion"];
                callback([NSString stringWithUTF8String:applicationVersion.data()]);
            },
            [](void * context, CHIP_ERROR err) {
                void (^callback)(MatterError *) = [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                    objectForKey:@"applicationBasic_readApplicationVersion"];
                callback([[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]]);
            });
        dispatch_async(clientQueue, ^{
            requestSentHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                         message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

@end
