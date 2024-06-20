/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

#import "../zap-generated/MCClusterObjects.h"
#import "../zap-generated/MCEndpointClusterType.h"
#import "CastingPlayerDiscoveryListenerCompat.h"
#import "DataSourceCompat.h"
#import "MCCastingApp.h"
#import "MCCastingPlayerDiscovery.h"
#import "MCCastingPlayer_Internal.h"
#import "MCConnectionCallbacks.h"
#import "MCEndpoint.h"
#import "MCErrorUtils.h"
#import "MCIdentificationDeclarationOptions.h"

#import "MatterCallbacks.h"
#import "OnboardingPayload.h"

#include <app/data-model/ListLargeSystemExtensions.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <lib/support/CHIPMem.h>
#include <platform/PlatformManager.h>

static const uint32_t kTargetPlayerDeviceType = 0x23;

@interface CastingServerBridge ()

@property DataSourceCompat * dataSource;
@property dispatch_block_t cancelDiscoveryCommissionersWork;
@property dispatch_queue_t commissioningClientQueue;
@property CommissioningCallbackHandlers * commissioningCallbackHandlers;
@property void (^_Nonnull onConnectionSuccessCallback)(VideoPlayer * _Nonnull);
@property void (^_Nonnull onConnectionFailureCallback)(MatterError * _Nonnull);
@property void (^_Nonnull onNewOrUpdatedEndpointCallback)(ContentApp * _Nonnull);
@property AppParameters * appParameters;
+ (MCCluster * _Nullable)getClusterWith:(MCEndpointClusterType)type contentApp:(ContentApp * _Nonnull)contentApp;

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

+ (MCCluster * _Nullable)getClusterWith:(MCEndpointClusterType)type contentApp:(ContentApp * _Nonnull)contentApp
{
    MCCastingPlayer * castingPlayer = [MCCastingPlayer getTargetCastingPlayer];
    VerifyOrReturnValue(castingPlayer != nil, nil, ChipLogError(AppServer, "CastingServerBridge.clusterForType no active target CastingPlayer found"));

    MCEndpoint * endpoint = [[castingPlayer endpoints] filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(MCEndpoint * evalEndpoint, NSDictionary * bindings) {
        return [[evalEndpoint identifier] intValue] == contentApp.endpointId;
    }]].firstObject;

    VerifyOrReturnValue(endpoint != nil, nil, ChipLogError(AppServer, "CastingServerBridge.clusterForType endpoint with ID: %d not found", contentApp.endpointId));
    return [endpoint clusterForType:type];
}

- (MatterError *)initializeApp:(AppParameters * _Nullable)appParameters
                   clientQueue:(dispatch_queue_t _Nonnull)clientQueue
          initAppStatusHandler:(nullable void (^)(MatterError * _Nonnull))initAppStatusHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().initializeApp() called");

    _appParameters = appParameters;
    _dataSource = [[DataSourceCompat alloc] initWithClientQueue:clientQueue];
    [_dataSource setAppParameters:appParameters];

    NSError * err = [MCCastingApp.getSharedInstance initializeWithDataSource:_dataSource];
    VerifyOrReturnValue(err == nil, [MCErrorUtils MatterErrorFromNsError:err]);

    [MCCastingApp.getSharedInstance startWithCompletionBlock:^(NSError * _Nullable e) {
        dispatch_async(clientQueue, ^{
            initAppStatusHandler([MCErrorUtils MatterErrorFromNsError:e]);
        });
    }];

    return MATTER_NO_ERROR;
}

- (void)setDacHolder:(DeviceAttestationCredentialsHolder * _Nonnull)deviceAttestationCredentials
           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    setDacHolderStatus:(void (^_Nonnull)(MatterError * _Nonnull))setDacHolderStatus
{
    ChipLogProgress(AppServer, "CastingServerBridge().setDacHolder() called");

    dispatch_sync([MCCastingApp.getSharedInstance getWorkQueue], ^{
        [self.dataSource setDacHolder:deviceAttestationCredentials];
        dispatch_async(clientQueue, ^{
            setDacHolderStatus(MATTER_NO_ERROR);
        });
    });
}

- (void)discoverCommissioners:(dispatch_queue_t _Nonnull)clientQueue
      discoveryRequestSentHandler:(nullable void (^)(MatterError * _Nonnull))discoveryRequestSentHandler
    discoveredCommissionerHandler:(nullable void (^)(DiscoveredNodeData *))discoveredCommissionerHandler
{
    [self discoverCommissioners:clientQueue timeoutInSeconds:0 discoveryRequestSentHandler:discoveryRequestSentHandler discoveredCommissionerHandler:discoveredCommissionerHandler];
}

- (void)discoverCommissioners:(dispatch_queue_t _Nonnull)clientQueue
                 timeoutInSeconds:(NSUInteger)timeoutInSeconds
      discoveryRequestSentHandler:(nullable void (^)(MatterError * _Nonnull))discoveryRequestSentHandler
    discoveredCommissionerHandler:(nullable void (^)(DiscoveredNodeData *))discoveredCommissionerHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().discoverCommissioners() called with timeoutInSeconds: %lu", timeoutInSeconds);

    dispatch_sync([[MCCastingApp getSharedInstance] getWorkQueue], ^{
        // cancel any future dispatches to cancelDiscoveryCommissionersWork
        if (self->_cancelDiscoveryCommissionersWork) {
            ChipLogProgress(AppServer, "CastingServerBridge().discoverCommissioners() canceling dispatch to cancelDiscoveryCommissionersWork");
            dispatch_block_cancel(self->_cancelDiscoveryCommissionersWork);
            self->_cancelDiscoveryCommissionersWork = nil;
        }
    });

    // stop previously triggered discovery and remove observers, if any
    NSError * e = [MCCastingPlayerDiscovery.sharedInstance stop];
    if (e != nil) {
        ChipLogError(AppServer, "CastingServerBridge().discoverCommissioners() MCCastingPlayerDiscovery stop error: %@", e.description);
    }
    [CastingPlayerDiscoveryListenerCompat removeObservers];

    // add observers and start discovery
    [CastingPlayerDiscoveryListenerCompat addObservers:clientQueue discoveredCommissionerHandler:discoveredCommissionerHandler];
    NSError * err = [MCCastingPlayerDiscovery.sharedInstance start:kTargetPlayerDeviceType];
    if (err == nil) {
        // if positive timeoutInSeconds specified, dispatch call to cancel this discovery request AFTER timeoutInSeconds
        if (timeoutInSeconds > 0) {
            self->_cancelDiscoveryCommissionersWork
                = dispatch_block_create(static_cast<dispatch_block_flags_t>(0), ^{
                      if (dispatch_block_testcancel(self->_cancelDiscoveryCommissionersWork)) {
                          ChipLogProgress(AppServer, "CastingServerBridge().discoverCommissioners() cancel timer invalidated.");
                          return;
                      }

                      // stop previously triggered discovery and remove observers, if any
                      ChipLogProgress(AppServer, "CastingServerBridge().discoverCommissioners() canceling previous discovery request");
                      NSError * e = [MCCastingPlayerDiscovery.sharedInstance stop];
                      if (e != nil) {
                          ChipLogError(AppServer, "CastingServerBridge().discoverCommissioners() MCCastingPlayerDiscovery stop error: %@", e.description);
                      }
                      [CastingPlayerDiscoveryListenerCompat removeObservers];
                      self->_cancelDiscoveryCommissionersWork = nil;
                  });

            ChipLogProgress(AppServer, "CastingServerBridge().discoverCommissioners() dispatching cancelDiscoveryCommissionersWork for %lu sec later", timeoutInSeconds);
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, timeoutInSeconds * NSEC_PER_SEC), [[MCCastingApp getSharedInstance] getWorkQueue], self->_cancelDiscoveryCommissionersWork);
        }
    } else {
        ChipLogError(AppServer, "CastingServerBridge().discoverCommissioners() MCCastingPlayerDiscovery start error: %@", err.description);
        [CastingPlayerDiscoveryListenerCompat removeObservers];
    }

    dispatch_async(clientQueue, ^{
        discoveryRequestSentHandler([MCErrorUtils MatterErrorFromNsError:err]);
    });
}

- (OnboardingPayload *)getOnboardingPayload
{
    return _appParameters.onboardingPayload;
}

- (void)openBasicCommissioningWindow:(dispatch_queue_t _Nonnull)clientQueue
       commissioningCallbackHandlers:(CommissioningCallbackHandlers * _Nullable)commissioningCallbackHandlers
         onConnectionSuccessCallback:(void (^_Nonnull)(VideoPlayer * _Nonnull))onConnectionSuccessCallback
         onConnectionFailureCallback:(void (^_Nonnull)(MatterError * _Nonnull))onConnectionFailureCallback
      onNewOrUpdatedEndpointCallback:(void (^_Nonnull)(ContentApp * _Nonnull))onNewOrUpdatedEndpointCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().openBasicCommissioningWindow() called");
    // capture the callbacks
    _commissioningClientQueue = clientQueue;
    _commissioningCallbackHandlers = commissioningCallbackHandlers;
    _onConnectionSuccessCallback = onConnectionSuccessCallback;
    _onConnectionFailureCallback = onConnectionFailureCallback;
    _onNewOrUpdatedEndpointCallback = onNewOrUpdatedEndpointCallback;

    // no-op: short-circuit commissioningWindow callbacks with MATTER_NO_ERROR
    dispatch_async(clientQueue, ^{
        if (commissioningCallbackHandlers != nil) {
            if (commissioningCallbackHandlers.commissioningWindowRequestedHandler != nil) {
                commissioningCallbackHandlers.commissioningWindowRequestedHandler(MATTER_NO_ERROR);
            }
            if (commissioningCallbackHandlers.commissioningWindowOpenedCallback != nil) {
                commissioningCallbackHandlers.commissioningWindowOpenedCallback(MATTER_NO_ERROR);
            }
        }
    });
}

- (void)sendUserDirectedCommissioningRequest:(DiscoveredNodeData * _Nonnull)commissioner
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                       udcRequestSentHandler:(nullable void (^)(MatterError * _Nonnull))udcRequestSentHandler
{
    [self sendUserDirectedCommissioningRequest:commissioner clientQueue:clientQueue udcRequestSentHandler:udcRequestSentHandler];
}

- (void)sendUserDirectedCommissioningRequest:(DiscoveredNodeData * _Nonnull)commissioner
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                       udcRequestSentHandler:(nullable void (^)(MatterError * _Nonnull))udcRequestSentHandler
                   desiredContentAppVendorId:(uint16_t)desiredContentAppVendorId
{
    ChipLogProgress(AppServer, "CastingServerBridge().sendUserDirectedCommissioningRequest() called with desiredContentAppVendorId: %d", desiredContentAppVendorId);

    MCIdentificationDeclarationOptions * identificationDeclarationOptions = [[MCIdentificationDeclarationOptions alloc] init];
    MCTargetAppInfo * targetAppInfo = [[MCTargetAppInfo alloc] initWithVendorId:desiredContentAppVendorId];
    BOOL success = [identificationDeclarationOptions addTargetAppInfo:targetAppInfo];
    if (success) {
        ChipLogProgress(AppServer, "CastingServerBridge().sendUserDirectedCommissioningRequest() Target app info added successfully");
    } else {
        ChipLogProgress(AppServer, "CastingServerBridge().sendUserDirectedCommissioningRequest() Failed to add target app info");
    }
    ChipLogProgress(AppServer, "CastingServerBridge().sendUserDirectedCommissioningRequest() MCIdentificationDeclarationOptions: \n%@", [identificationDeclarationOptions description]);

    void (^connectionCompletionBlock)(NSError * _Nullable) = ^(NSError * _Nullable err) {
        dispatch_async(clientQueue, ^{
            ChipLogError(AppServer, "CastingServerBridge().sendUserDirectedCommissioningRequest() connectionCompleteCallback() completed with error: %@", err.description);
            if (err == nil) {
                if (self->_commissioningCallbackHandlers != nil && self->_commissioningCallbackHandlers.commissioningCompleteCallback != nil) {
                    self->_commissioningCallbackHandlers.commissioningCompleteCallback(MATTER_NO_ERROR);
                }

                MCCastingPlayer * castingPlayer = [MCCastingPlayer getTargetCastingPlayer];
                if (self->_onConnectionSuccessCallback != nil) {
                    self->_onConnectionSuccessCallback([[VideoPlayer alloc] initWithCastingPlayer:castingPlayer]);
                }

                if (self->_onNewOrUpdatedEndpointCallback != nil && castingPlayer != nil) {
                    NSArray<MCEndpoint *> * endpoints = castingPlayer.endpoints;
                    for (MCEndpoint * endpoint in endpoints) {
                        self->_onNewOrUpdatedEndpointCallback([[ContentApp alloc] initWithEndpoint:endpoint]);
                    }
                }
            }
        });
    };

    MCConnectionCallbacks * connectionCallbacks = [[MCConnectionCallbacks alloc] initWithCallbacks:connectionCompletionBlock commissionerDeclarationCallback:nil];

    [commissioner.getCastingPlayer verifyOrEstablishConnectionWithCallbacks:connectionCallbacks identificationDeclarationOptions:identificationDeclarationOptions];

    dispatch_async(clientQueue, ^{
        udcRequestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)getActiveTargetVideoPlayers:(dispatch_queue_t _Nonnull)clientQueue
    activeTargetVideoPlayersHandler:(nullable void (^)(NSMutableArray * _Nullable))activeTargetVideoPlayersHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().getActiveTargetVideoPlayers() called");
    NSMutableArray * videoPlayers = [NSMutableArray new];
    MCCastingPlayer * castingPlayer = [MCCastingPlayer getTargetCastingPlayer];
    if (castingPlayer != nil) {
        VideoPlayer * videoPlayer = [[VideoPlayer alloc] initWithCastingPlayer:castingPlayer];
        videoPlayer.isConnected = true;
        videoPlayers[0] = videoPlayer;
    }
    dispatch_async(clientQueue, ^{
        activeTargetVideoPlayersHandler(videoPlayers);
    });
}

- (void)verifyOrEstablishConnection:(VideoPlayer * _Nonnull)videoPlayer
                        clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                 requestSentHandler:(nullable void (^)(MatterError * _Nonnull))requestSentHandler
        onConnectionSuccessCallback:(void (^_Nonnull)(VideoPlayer * _Nonnull))onConnectionSuccessCallback
        onConnectionFailureCallback:(void (^_Nonnull)(MatterError * _Nonnull))onConnectionFailureCallback
     onNewOrUpdatedEndpointCallback:(void (^_Nonnull)(ContentApp * _Nonnull))onNewOrUpdatedEndpointCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().verifyOrEstablishConnection() is unsupported");

    dispatch_async(clientQueue, ^{
        requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)shutdownAllSubscriptions:(dispatch_queue_t _Nonnull)clientQueue requestSentHandler:(nullable void (^)())requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().shutdownAllSubscriptions() called");
    [[MCCastingApp getSharedInstance] ShutdownAllSubscriptions];
    dispatch_async(clientQueue, ^{
        requestSentHandler();
    });
}

- (void)startMatterServer:(dispatch_queue_t _Nonnull)clientQueue
    startMatterServerCompletionCallback:(nullable void (^)(MatterError * _Nonnull))startMatterServerCompletionCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().startMatterServer() called");
    [[MCCastingApp getSharedInstance] startWithCompletionBlock:^(NSError * _Nullable err) {
        ChipLogError(AppServer, "CastingServerBridge() startWithCompletionBlock called");
        startMatterServerCompletionCallback([MCErrorUtils MatterErrorFromNsError:err]);
    }];
}

- (void)stopMatterServer
{
    ChipLogProgress(AppServer, "CastingServerBridge().stopMatterServer() called");
    [[MCCastingApp getSharedInstance] stopWithCompletionBlock:^(NSError * _Nullable) {
        ChipLogError(AppServer, "CastingServerBridge() stopWithCompletionBlock called");
    }];
}

- (void)disconnect:(dispatch_queue_t _Nonnull)clientQueue requestSentHandler:(nullable void (^)())requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().disconnect() called");
    MCCastingPlayer * castingPlayer = [MCCastingPlayer getTargetCastingPlayer];
    if (castingPlayer != nil) {
        [castingPlayer disconnect];
    }
    dispatch_async(clientQueue, ^{
        requestSentHandler();
    });
}

- (void)purgeCache:(dispatch_queue_t _Nonnull)clientQueue responseHandler:(void (^)(MatterError * _Nonnull))responseHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().purgeCache() called");
    NSError * err = [[MCCastingApp getSharedInstance] ClearCache];
    dispatch_async(clientQueue, ^{
        responseHandler([MCErrorUtils MatterErrorFromNsError:err]);
    });
}

- (void)contentLauncher_launchUrl:(ContentApp * _Nonnull)contentApp
                       contentUrl:(NSString * _Nonnull)contentUrl
                contentDisplayStr:(NSString * _Nonnull)contentDisplayStr
                 responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
               requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().contentLauncher_launchUrl() called");
    MCContentLauncherCluster * cluster = (MCContentLauncherCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeContentLauncher contentApp:contentApp];
    if (cluster == nil || [cluster launchURLCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCContentLauncherClusterLaunchURLParams * request = [[MCContentLauncherClusterLaunchURLParams alloc] init];
    request.contentURL = contentUrl;
    request.displayString = contentDisplayStr;
    [[cluster launchURLCommand] invoke:request
                               context:nil
                            completion:^(void * _Nullable, NSError * _Nullable err, MCContentLauncherClusterLauncherResponseParams * _Nullable response) {
                                dispatch_async(clientQueue, ^{
                                    responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                                });
                            }
                  timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)contentLauncher_launchContent:(ContentApp * _Nonnull)contentApp
                        contentSearch:(ContentLauncher_ContentSearch * _Nonnull)contentSearch
                             autoPlay:(bool)autoPlay
                                 data:(NSString * _Nullable)data
                     responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                          clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                   requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().contentLauncher_launchContent() called");
    MCContentLauncherCluster * cluster = (MCContentLauncherCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeContentLauncher contentApp:contentApp];
    if (cluster == nil || [cluster launchContentCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCContentLauncherClusterLaunchContentParams * request = [MCContentLauncherClusterLaunchContentParams new];
    request.autoPlay = @(autoPlay);
    request.data = data;
    MCContentLauncherClusterContentSearchStruct * search = nil;
    if (contentSearch.parameterList != nil) {
        NSMutableArray * parameterList = [NSMutableArray new];
        for (ContentLauncher_Parameter * srcParameter in contentSearch.parameterList) {
            MCContentLauncherClusterParameterStruct * parameter = [MCContentLauncherClusterParameterStruct new];
            parameter.type = @(srcParameter.type);
            parameter.value = srcParameter.value;
            if (srcParameter.externalIDList != nil) {
                NSMutableArray * externalIdList = [NSMutableArray new];
                for (ContentLauncher_AdditionalInfo * srcExternalId in srcParameter.externalIDList) {
                    MCContentLauncherClusterAdditionalInfoStruct * externalId = [MCContentLauncherClusterAdditionalInfoStruct new];
                    externalId.name = srcExternalId.name;
                    externalId.value = srcExternalId.value;
                    [externalIdList addObject:externalId];
                }
                parameter.externalIDList = externalIdList;
            }
            [parameterList addObject:parameter];
        }

        search = [MCContentLauncherClusterContentSearchStruct new];
        search.parameterList = parameterList;
    }
    request.search = search;

    [[cluster launchContentCommand] invoke:request
                                   context:nil
                                completion:^(void * _Nullable, NSError * _Nullable err, MCContentLauncherClusterLauncherResponseParams * _Nullable response) {
                                    dispatch_async(clientQueue, ^{
                                        responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                                    });
                                }
                      timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)mediaPlayback_play:(ContentApp * _Nonnull)contentApp
          responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_play() called");
    MCMediaPlaybackCluster * cluster = (MCMediaPlaybackCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeMediaPlayback contentApp:contentApp];
    if (cluster == nil || [cluster playCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCMediaPlaybackClusterPlayParams * request = [MCMediaPlaybackClusterPlayParams new];
    [[cluster playCommand] invoke:request
                          context:nil
                       completion:^(void * _Nullable, NSError * _Nullable err, MCMediaPlaybackClusterPlaybackResponseParams * _Nullable response) {
                           dispatch_async(clientQueue, ^{
                               responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                           });
                       }
             timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)mediaPlayback_pause:(ContentApp * _Nonnull)contentApp
           responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_pause() called");
    MCMediaPlaybackCluster * cluster = (MCMediaPlaybackCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeMediaPlayback contentApp:contentApp];
    if (cluster == nil || [cluster pauseCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCMediaPlaybackClusterPauseParams * request = [MCMediaPlaybackClusterPauseParams new];
    [[cluster pauseCommand] invoke:request
                           context:nil
                        completion:^(void * _Nullable, NSError * _Nullable err, MCMediaPlaybackClusterPlaybackResponseParams * _Nullable response) {
                            dispatch_async(clientQueue, ^{
                                responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                            });
                        }
              timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)mediaPlayback_stopPlayback:(ContentApp * _Nonnull)contentApp
                  responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                       clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_stopPlayback() called");
    MCMediaPlaybackCluster * cluster = (MCMediaPlaybackCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeMediaPlayback contentApp:contentApp];
    if (cluster == nil || [cluster stopCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCMediaPlaybackClusterStopParams * request = [MCMediaPlaybackClusterStopParams new];
    [[cluster stopCommand] invoke:request
                          context:nil
                       completion:^(void * _Nullable, NSError * _Nullable err, MCMediaPlaybackClusterPlaybackResponseParams * _Nullable response) {
                           dispatch_async(clientQueue, ^{
                               responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                           });
                       }
             timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)mediaPlayback_next:(ContentApp * _Nonnull)contentApp
          responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_next() called");
    MCMediaPlaybackCluster * cluster = (MCMediaPlaybackCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeMediaPlayback contentApp:contentApp];
    if (cluster == nil || [cluster nextCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCMediaPlaybackClusterNextParams * request = [MCMediaPlaybackClusterNextParams new];
    [[cluster nextCommand] invoke:request
                          context:nil
                       completion:^(void * _Nullable, NSError * _Nullable err, MCMediaPlaybackClusterPlaybackResponseParams * _Nullable response) {
                           dispatch_async(clientQueue, ^{
                               responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                           });
                       }
             timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)mediaPlayback_seek:(ContentApp * _Nonnull)contentApp
                  position:(uint64_t)position
          responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_seek() called");
    MCMediaPlaybackCluster * cluster = (MCMediaPlaybackCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeMediaPlayback contentApp:contentApp];
    if (cluster == nil || [cluster seekCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCMediaPlaybackClusterSeekParams * request = [MCMediaPlaybackClusterSeekParams new];
    request.position = @(position);
    [[cluster seekCommand] invoke:request
                          context:nil
                       completion:^(void * _Nullable, NSError * _Nullable err, MCMediaPlaybackClusterPlaybackResponseParams * _Nullable response) {
                           dispatch_async(clientQueue, ^{
                               responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                           });
                       }
             timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)mediaPlayback_skipForward:(ContentApp * _Nonnull)contentApp
        deltaPositionMilliseconds:(uint64_t)deltaPositionMilliseconds
                 responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
               requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_skipForward() called");
    MCMediaPlaybackCluster * cluster = (MCMediaPlaybackCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeMediaPlayback contentApp:contentApp];
    if (cluster == nil || [cluster skipForwardCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCMediaPlaybackClusterSkipForwardParams * request = [MCMediaPlaybackClusterSkipForwardParams new];
    request.deltaPositionMilliseconds = @(deltaPositionMilliseconds);
    [[cluster skipForwardCommand] invoke:request
                                 context:nil
                              completion:^(void * _Nullable, NSError * _Nullable err, MCMediaPlaybackClusterPlaybackResponseParams * _Nullable response) {
                                  dispatch_async(clientQueue, ^{
                                      responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                                  });
                              }
                    timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)mediaPlayback_skipBackward:(ContentApp * _Nonnull)contentApp
         deltaPositionMilliseconds:(uint64_t)deltaPositionMilliseconds
                  responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                       clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_skipBackward() called");
    MCMediaPlaybackCluster * cluster = (MCMediaPlaybackCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeMediaPlayback contentApp:contentApp];
    if (cluster == nil || [cluster skipBackwardCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCMediaPlaybackClusterSkipBackwardParams * request = [MCMediaPlaybackClusterSkipBackwardParams new];
    request.deltaPositionMilliseconds = @(deltaPositionMilliseconds);
    [[cluster skipBackwardCommand] invoke:request
                                  context:nil
                               completion:^(void * _Nullable, NSError * _Nullable err, MCMediaPlaybackClusterPlaybackResponseParams * _Nullable response) {
                                   dispatch_async(clientQueue, ^{
                                       responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                                   });
                               }
                     timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)mediaPlayback_previous:(ContentApp * _Nonnull)contentApp
              responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                   clientQueue:(dispatch_queue_t _Nonnull)clientQueue
            requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_previous() called");
    MCMediaPlaybackCluster * cluster = (MCMediaPlaybackCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeMediaPlayback contentApp:contentApp];
    if (cluster == nil || [cluster previousCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCMediaPlaybackClusterPreviousParams * request = [MCMediaPlaybackClusterPreviousParams new];
    [[cluster previousCommand] invoke:request
                              context:nil
                           completion:^(void * _Nullable, NSError * _Nullable err, MCMediaPlaybackClusterPlaybackResponseParams * _Nullable response) {
                               dispatch_async(clientQueue, ^{
                                   responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                               });
                           }
                 timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)mediaPlayback_rewind:(ContentApp * _Nonnull)contentApp
            responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
          requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_rewind() called");
    MCMediaPlaybackCluster * cluster = (MCMediaPlaybackCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeMediaPlayback contentApp:contentApp];
    if (cluster == nil || [cluster rewindCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCMediaPlaybackClusterRewindParams * request = [MCMediaPlaybackClusterRewindParams new];
    [[cluster rewindCommand] invoke:request
                            context:nil
                         completion:^(void * _Nullable, NSError * _Nullable err, MCMediaPlaybackClusterPlaybackResponseParams * _Nullable response) {
                             dispatch_async(clientQueue, ^{
                                 responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                             });
                         }
               timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)mediaPlayback_fastForward:(ContentApp * _Nonnull)contentApp
                 responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
               requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_fastForward() called");
    MCMediaPlaybackCluster * cluster = (MCMediaPlaybackCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeMediaPlayback contentApp:contentApp];
    if (cluster == nil || [cluster fastForwardCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCMediaPlaybackClusterFastForwardParams * request = [MCMediaPlaybackClusterFastForwardParams new];
    [[cluster fastForwardCommand] invoke:request
                                 context:nil
                              completion:^(void * _Nullable, NSError * _Nullable err, MCMediaPlaybackClusterPlaybackResponseParams * _Nullable response) {
                                  dispatch_async(clientQueue, ^{
                                      responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                                  });
                              }
                    timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)mediaPlayback_startOver:(ContentApp * _Nonnull)contentApp
               responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                    clientQueue:(dispatch_queue_t _Nonnull)clientQueue
             requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_startOver() called");
    MCMediaPlaybackCluster * cluster = (MCMediaPlaybackCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeMediaPlayback contentApp:contentApp];
    if (cluster == nil || [cluster startOverCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCMediaPlaybackClusterStartOverParams * request = [MCMediaPlaybackClusterStartOverParams new];
    [[cluster startOverCommand] invoke:request
                               context:nil
                            completion:^(void * _Nullable, NSError * _Nullable err, MCMediaPlaybackClusterPlaybackResponseParams * _Nullable response) {
                                dispatch_async(clientQueue, ^{
                                    responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                                });
                            }
                  timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
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
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_subscribeCurrentState() called");
    MCMediaPlaybackCluster * cluster = (MCMediaPlaybackCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeMediaPlayback contentApp:contentApp];
    if (cluster == nil || [cluster currentStateAttribute] == nil) {
        ChipLogError(AppServer, "Cluster/attribute not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    [[cluster currentStateAttribute] subscribe:nil completion:^(void * _Nullable context, NSNumber * _Nullable before, NSNumber * _Nullable after, NSError * _Nullable err) {
        dispatch_async(clientQueue, ^{
            if (err != nil) {
                failureCallback([MCErrorUtils MatterErrorFromNsError:err]);
            } else {
                successCallback((MediaPlayback_PlaybackState)[after unsignedCharValue]);
            }
        });
    } minInterval:@(minInterval) maxInterval:@(maxInterval)];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
        subscriptionEstablishedCallback();
    });
}

- (void)applicationLauncher_launchApp:(ContentApp * _Nonnull)contentApp
                      catalogVendorId:(uint16_t)catalogVendorId
                        applicationId:(NSString * _Nonnull)applicationId
                                 data:(NSData * _Nullable)data
                     responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                          clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                   requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationLauncher_launchApp() called");
    MCApplicationLauncherCluster * cluster = (MCApplicationLauncherCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeApplicationLauncher contentApp:contentApp];
    if (cluster == nil || [cluster launchAppCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCApplicationLauncherClusterLaunchAppParams * request = [MCApplicationLauncherClusterLaunchAppParams new];
    request.data = data;
    MCApplicationLauncherClusterApplicationStruct * application = [MCApplicationLauncherClusterApplicationStruct new];
    application.catalogVendorID = @(catalogVendorId);
    application.applicationID = applicationId;
    request.application = application;

    [[cluster launchAppCommand] invoke:request
                               context:nil
                            completion:^(void * _Nullable, NSError * _Nullable err, MCApplicationLauncherClusterLauncherResponseParams * _Nullable response) {
                                dispatch_async(clientQueue, ^{
                                    responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                                });
                            }
                  timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)applicationLauncher_stopApp:(ContentApp * _Nonnull)contentApp
                    catalogVendorId:(uint16_t)catalogVendorId
                      applicationId:(NSString * _Nonnull)applicationId
                   responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                        clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                 requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationLauncher_stopApp() called");
    MCApplicationLauncherCluster * cluster = (MCApplicationLauncherCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeApplicationLauncher contentApp:contentApp];
    if (cluster == nil || [cluster stopAppCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCApplicationLauncherClusterStopAppParams * request = [MCApplicationLauncherClusterStopAppParams new];
    MCApplicationLauncherClusterApplicationStruct * application = [MCApplicationLauncherClusterApplicationStruct new];
    application.catalogVendorID = @(catalogVendorId);
    application.applicationID = applicationId;
    request.application = application;

    [[cluster stopAppCommand] invoke:request
                             context:nil
                          completion:^(void * _Nullable, NSError * _Nullable err, MCApplicationLauncherClusterLauncherResponseParams * _Nullable response) {
                              dispatch_async(clientQueue, ^{
                                  responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                              });
                          }
                timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)applicationLauncher_hideApp:(ContentApp * _Nonnull)contentApp
                    catalogVendorId:(uint16_t)catalogVendorId
                      applicationId:(NSString * _Nonnull)applicationId
                   responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                        clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                 requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationLauncher_hideApp() called");
    MCApplicationLauncherCluster * cluster = (MCApplicationLauncherCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeApplicationLauncher contentApp:contentApp];
    if (cluster == nil || [cluster hideAppCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCApplicationLauncherClusterHideAppParams * request = [MCApplicationLauncherClusterHideAppParams new];
    MCApplicationLauncherClusterApplicationStruct * application = [MCApplicationLauncherClusterApplicationStruct new];
    application.catalogVendorID = @(catalogVendorId);
    application.applicationID = applicationId;
    request.application = application;

    [[cluster hideAppCommand] invoke:request
                             context:nil
                          completion:^(void * _Nullable, NSError * _Nullable err, MCApplicationLauncherClusterLauncherResponseParams * _Nullable response) {
                              dispatch_async(clientQueue, ^{
                                  responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                              });
                          }
                timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)targetNavigator_navigateTarget:(ContentApp * _Nonnull)contentApp
                                target:(uint8_t)target
                                  data:(NSString * _Nullable)data
                      responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().targetNavigator_navigateTarget() called");
    MCTargetNavigatorCluster * cluster = (MCTargetNavigatorCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeTargetNavigator contentApp:contentApp];
    if (cluster == nil || [cluster navigateTargetCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCTargetNavigatorClusterNavigateTargetParams * request = [MCTargetNavigatorClusterNavigateTargetParams new];
    request.target = @(target);
    request.data = data;

    [[cluster navigateTargetCommand] invoke:request
                                    context:nil
                                 completion:^(void * _Nullable, NSError * _Nullable err, MCTargetNavigatorClusterNavigateTargetResponseParams * _Nullable response) {
                                     dispatch_async(clientQueue, ^{
                                         responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                                     });
                                 }
                       timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
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
    ChipLogProgress(AppServer, "CastingServerBridge().targetNavigator_subscribeTargetList() called");
    MCTargetNavigatorCluster * cluster = (MCTargetNavigatorCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeTargetNavigator contentApp:contentApp];
    if (cluster == nil || [cluster targetListAttribute] == nil) {
        ChipLogError(AppServer, "Cluster/attribute not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    [[cluster targetListAttribute] subscribe:nil completion:^(void * _Nullable context, NSArray * _Nullable, NSArray * _Nullable newTargetList, NSError * _Nullable err) {
        dispatch_async(clientQueue, ^{
            if (err != nil) {
                failureCallback([MCErrorUtils MatterErrorFromNsError:err]);
            } else {
                NSMutableArray * resultTargetList = nil;
                if (newTargetList != nil) {
                    resultTargetList = [NSMutableArray arrayWithCapacity:newTargetList.count];
                    for (MCTargetNavigatorClusterTargetInfoStruct * targetInfo in newTargetList) {
                        [resultTargetList addObject:[[TargetNavigator_TargetInfoStruct alloc] initWithIdentifier:targetInfo.identifier name:targetInfo.name]];
                    }
                }
                successCallback(resultTargetList);
            }
        });
    } minInterval:@(minInterval) maxInterval:@(maxInterval)];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
        subscriptionEstablishedCallback();
    });
}

- (void)keypadInput_sendKey:(ContentApp * _Nonnull)contentApp
                    keyCode:(uint8_t)keyCode
           responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().keypadInput_sendKey() called");
    MCKeypadInputCluster * cluster = (MCKeypadInputCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeKeypadInput contentApp:contentApp];
    if (cluster == nil || [cluster sendKeyCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCKeypadInputClusterSendKeyParams * request = [MCKeypadInputClusterSendKeyParams new];
    request.keyCode = @(keyCode);

    [[cluster sendKeyCommand] invoke:request
                             context:nil
                          completion:^(void * _Nullable, NSError * _Nullable err, MCKeypadInputClusterSendKeyResponseParams * _Nullable response) {
                              dispatch_async(clientQueue, ^{
                                  responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                              });
                          }
                timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)applicationBasic_readVendorName:(ContentApp * _Nonnull)contentApp
                            clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                     requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                        successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                        failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_readVendorName() called");
    MCApplicationBasicCluster * cluster = (MCApplicationBasicCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeApplicationBasic contentApp:contentApp];
    if (cluster == nil || [cluster vendorNameAttribute] == nil) {
        ChipLogError(AppServer, "Cluster/attribute not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    [[cluster vendorNameAttribute] read:nil completion:^(void * _Nullable context, NSString * _Nullable before, NSString * _Nullable after, NSError * _Nullable err) {
        dispatch_async(clientQueue, ^{
            if (err != nil) {
                failureCallback([MCErrorUtils MatterErrorFromNsError:err]);
            } else {
                successCallback(after);
            }
        });
    }];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)applicationBasic_readVendorID:(ContentApp * _Nonnull)contentApp
                          clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                   requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                      successCallback:(void (^_Nonnull)(NSNumber * _Nonnull))successCallback
                      failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_readVendorID() called");
    MCApplicationBasicCluster * cluster = (MCApplicationBasicCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeApplicationBasic contentApp:contentApp];
    if (cluster == nil || [cluster vendorIDAttribute] == nil) {
        ChipLogError(AppServer, "Cluster/attribute not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    [[cluster vendorIDAttribute] read:nil completion:^(void * _Nullable context, NSNumber * _Nullable before, NSNumber * _Nullable after, NSError * _Nullable err) {
        dispatch_async(clientQueue, ^{
            if (err != nil) {
                failureCallback([MCErrorUtils MatterErrorFromNsError:err]);
            } else {
                successCallback(after);
            }
        });
    }];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)applicationBasic_readApplicationName:(ContentApp * _Nonnull)contentApp
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                          requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                             successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                             failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_readApplicationName() called");
    MCApplicationBasicCluster * cluster = (MCApplicationBasicCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeApplicationBasic contentApp:contentApp];
    if (cluster == nil || [cluster applicationNameAttribute] == nil) {
        ChipLogError(AppServer, "Cluster/attribute not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    [[cluster applicationNameAttribute] read:nil completion:^(void * _Nullable context, NSString * _Nullable before, NSString * _Nullable after, NSError * _Nullable err) {
        dispatch_async(clientQueue, ^{
            if (err != nil) {
                failureCallback([MCErrorUtils MatterErrorFromNsError:err]);
            } else {
                successCallback(after);
            }
        });
    }];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)applicationBasic_readProductID:(ContentApp * _Nonnull)contentApp
                           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                       successCallback:(void (^_Nonnull)(uint16_t))successCallback
                       failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_readProductID() called");
    MCApplicationBasicCluster * cluster = (MCApplicationBasicCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeApplicationBasic contentApp:contentApp];
    if (cluster == nil || [cluster productIDAttribute] == nil) {
        ChipLogError(AppServer, "Cluster/attribute not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    [[cluster productIDAttribute] read:nil completion:^(void * _Nullable context, NSNumber * _Nullable before, NSNumber * _Nullable after, NSError * _Nullable err) {
        dispatch_async(clientQueue, ^{
            if (err != nil) {
                failureCallback([MCErrorUtils MatterErrorFromNsError:err]);
            } else {
                successCallback([after unsignedShortValue]);
            }
        });
    }];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)applicationBasic_readApplicationVersion:(ContentApp * _Nonnull)contentApp
                                    clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                             requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                                successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                                failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_readApplicationVersion() called");
    MCApplicationBasicCluster * cluster = (MCApplicationBasicCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeApplicationBasic contentApp:contentApp];
    if (cluster == nil || [cluster applicationVersionAttribute] == nil) {
        ChipLogError(AppServer, "Cluster/attribute not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    [[cluster applicationVersionAttribute] read:nil completion:^(void * _Nullable context, NSString * _Nullable before, NSString * _Nullable after, NSError * _Nullable err) {
        dispatch_async(clientQueue, ^{
            if (err != nil) {
                failureCallback([MCErrorUtils MatterErrorFromNsError:err]);
            } else {
                successCallback(after);
            }
        });
    }];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)onOff_on:(ContentApp * _Nonnull)contentApp
      responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().onOff_on() called");
    MCOnOffCluster * cluster = (MCOnOffCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeOnOff contentApp:contentApp];
    if (cluster == nil || [cluster onCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCOnOffClusterOnParams * request = [MCOnOffClusterOnParams new];
    [[cluster onCommand] invoke:request
                        context:nil
                     completion:^(void * _Nullable, NSError * _Nullable err, MCNullObjectType * _Nullable response) {
                         dispatch_async(clientQueue, ^{
                             responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                         });
                     }
           timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)onOff_off:(ContentApp * _Nonnull)contentApp
      responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().onOff_off() called");
    MCOnOffCluster * cluster = (MCOnOffCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeOnOff contentApp:contentApp];
    if (cluster == nil || [cluster offCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCOnOffClusterOffParams * request = [MCOnOffClusterOffParams new];
    [[cluster offCommand] invoke:request
                         context:nil
                      completion:^(void * _Nullable, NSError * _Nullable err, MCNullObjectType * _Nullable response) {
                          dispatch_async(clientQueue, ^{
                              responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                          });
                      }
            timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}

- (void)onOff_toggle:(ContentApp * _Nonnull)contentApp
      responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
{
    ChipLogProgress(AppServer, "CastingServerBridge().onOff_toggle() called");
    MCOnOffCluster * cluster = (MCOnOffCluster *) [CastingServerBridge getClusterWith:MCEndpointClusterTypeOnOff contentApp:contentApp];
    if (cluster == nil || [cluster toggleCommand] == nil) {
        ChipLogError(AppServer, "Cluster/command not found");
        dispatch_async(clientQueue, ^{
            requestSentHandler([MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // create request
    MCOnOffClusterToggleParams * request = [MCOnOffClusterToggleParams new];
    [[cluster toggleCommand] invoke:request
                            context:nil
                         completion:^(void * _Nullable, NSError * _Nullable err, MCNullObjectType * _Nullable response) {
                             dispatch_async(clientQueue, ^{
                                 responseCallback([MCErrorUtils MatterErrorFromNsError:err]);
                             });
                         }
               timedInvokeTimeoutMs:@5000];

    dispatch_async(clientQueue, ^{
        requestSentHandler(MATTER_NO_ERROR);
    });
}
@end
