/**
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

#import "../MatterError.h"
#import "AppParameters.h"
#import "CommissioningCallbackHandlers.h"
#import "ContentApp.h"
#import "ContentLauncherTypes.h"
#import "DiscoveredNodeData.h"
#import "MediaPlaybackTypes.h"
#import "OnboardingPayload.h"
#import "TargetNavigatorTypes.h"
#import "VideoPlayer.h"
#import <Foundation/Foundation.h>

#ifndef CastingServerBridge_h
#define CastingServerBridge_h

__attribute__((deprecated("Use the APIs described in /examples/tv-casting-app/APIs.md instead.")))
@interface CastingServerBridge : NSObject

+ (CastingServerBridge * _Nullable)getSharedInstance;

- (MatterError * _Nonnull)initializeApp:(AppParameters * _Nullable)appParameters
                            clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                   initAppStatusHandler:(nullable void (^)(MatterError * _Nonnull))initAppStatusHandler;

- (void)setDacHolder:(DeviceAttestationCredentialsHolder * _Nonnull)deviceAttestationCredentials
           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    setDacHolderStatus:(void (^_Nonnull)(MatterError * _Nonnull))setDacHolderStatus;

/*!
 @brief Browse for on-network commissioner TVs

 @param clientQueue Queue to dispatch the call to the discoveryRequestSentHandler on

 @param timeoutInSeconds time after which this discovery request should be auto-canceled

 @param discoveryRequestSentHandler Handler to call after the Commissioner discovery request has been sent
 */
- (void)discoverCommissioners:(dispatch_queue_t _Nonnull)clientQueue
                 timeoutInSeconds:(NSUInteger)timeoutInSeconds
      discoveryRequestSentHandler:(nullable void (^)(MatterError * _Nonnull))discoveryRequestSentHandler
    discoveredCommissionerHandler:(nullable void (^)(DiscoveredNodeData * _Nonnull))discoveredCommissionerHandler;

- (void)discoverCommissioners:(dispatch_queue_t _Nonnull)clientQueue
      discoveryRequestSentHandler:(nullable void (^)(MatterError * _Nonnull))discoveryRequestSentHandler
    discoveredCommissionerHandler:(nullable void (^)(DiscoveredNodeData * _Nonnull))discoveredCommissionerHandler;

/*!
 @brief Send a User Directed Commissioning request to a commissioner TV

 @param commissioner Commissioner to request commissioning from

 @param clientQueue Queue to dispatch the call to the udcRequestSentHandler on

 @param udcRequestSentHandler Handler to call on sending the User Directed Commissioning request
 */
- (void)sendUserDirectedCommissioningRequest:(DiscoveredNodeData * _Nonnull)commissioner
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                       udcRequestSentHandler:(nullable void (^)(MatterError * _Nonnull))udcRequestSentHandler;

/*!
 @brief Send a User Directed Commissioning request to a commissioner TV

 @param commissioner Commissioner to request commissioning from

 @param clientQueue Queue to dispatch the call to the udcRequestSentHandler on

 @param udcRequestSentHandler Handler to call on sending the User Directed Commissioning request

 @param desiredContentAppVendorId VendorId of the ContentApp that the client wants to interact with / cast to.  If this value is passed in, the CastingServerBridge will force User Directed
 * Commissioning, in case the desired ContentApp is not found in the on-device cached information
 */
- (void)sendUserDirectedCommissioningRequest:(DiscoveredNodeData * _Nonnull)commissioner
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                       udcRequestSentHandler:(nullable void (^)(MatterError * _Nonnull))udcRequestSentHandler
                   desiredContentAppVendorId:(uint16_t)desiredContentAppVendorId;

/*!
 @brief Return the onboarding payload for this app (setup passcode, discriminator)

 @return Onboarding payload
 */
- (OnboardingPayload * _Nonnull)getOnboardingPayload;

/*!
 @brief Request opening of a basic commissioning window

 @param clientQueue Queue to dispatch the call to the commissioningWindowRequestedHandler on

 @param commissioningCallbackHandlers Optional parameter to specific handlers for callbacks during commissioning

 @param onConnectionSuccessCallback Handles a VideoPlayer * once connection is successfully established

 @param onConnectionFailureCallback Handles MatterError if there is a failure in establishing connection

 @param onNewOrUpdatedEndpointCallback Handles a ContentApp * for each new ContentApp is found. May be called multiple times based
 on the number of ContentApp
 */
- (void)openBasicCommissioningWindow:(dispatch_queue_t _Nonnull)clientQueue
       commissioningCallbackHandlers:(CommissioningCallbackHandlers * _Nullable)commissioningCallbackHandlers
         onConnectionSuccessCallback:(void (^_Nonnull)(VideoPlayer * _Nonnull))onConnectionSuccessCallback
         onConnectionFailureCallback:(void (^_Nonnull)(MatterError * _Nonnull))onConnectionFailureCallback
      onNewOrUpdatedEndpointCallback:(void (^_Nonnull)(ContentApp * _Nonnull))onNewOrUpdatedEndpointCallback;

/*!
 @brief Gets the list of VideoPlayers currently connected

 @param clientQueue Queue to invoke callbacks on

 @param activeTargetVideoPlayersHandler Handles NSMutableArray of active/currently connected VideoPlayers. Nil, if no such
 VideoPlayers are found.
 */
- (void)getActiveTargetVideoPlayers:(dispatch_queue_t _Nonnull)clientQueue
    activeTargetVideoPlayersHandler:(nullable void (^)(NSMutableArray * _Nullable))activeTargetVideoPlayersHandler;

/*!
 @brief Verify if a connection exists or connect to the VideoPlayer passed in as parameter.

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handles MatterError and called after the request has been sent

 @param onConnectionSuccessCallback Handles a VideoPlayer * once connection is successfully established

 @param onConnectionFailureCallback Handles MatterError if there is a failure in establishing connection

 @param onNewOrUpdatedEndpointCallback Handles a ContentApp * for each new ContentApp is found. May be called multiple times based
 on the number of ContentApp
 */
- (void)verifyOrEstablishConnection:(VideoPlayer * _Nonnull)videoPlayer
                        clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                 requestSentHandler:(nullable void (^)(MatterError * _Nonnull))requestSentHandler
        onConnectionSuccessCallback:(void (^_Nonnull)(VideoPlayer * _Nonnull))onConnectionSuccessCallback
        onConnectionFailureCallback:(void (^_Nonnull)(MatterError * _Nonnull))onConnectionFailureCallback
     onNewOrUpdatedEndpointCallback:(void (^_Nonnull)(ContentApp * _Nonnull))onNewOrUpdatedEndpointCallback;

/*!
 @brief Tears down all active subscriptions.

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Called after the request has been sent
 */
- (void)shutdownAllSubscriptions:(dispatch_queue_t _Nonnull)clientQueue requestSentHandler:(nullable void (^)())requestSentHandler;

/*!
 @brief  Mark any open session with the currently connected Video player as expired.

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Called after the request has been sent
 */
- (void)disconnect:(dispatch_queue_t _Nonnull)clientQueue requestSentHandler:(nullable void (^)())requestSentHandler;

/*!
 @brief Purge data cached by the Matter casting library

 @param clientQueue Queue to invoke callbacks on

 @param responseHandler Called when purgeCache completes
 */
- (void)purgeCache:(dispatch_queue_t _Nonnull)clientQueue responseHandler:(void (^_Nonnull)(MatterError * _Nonnull))responseHandler;

/*!
 @brief Start the Matter server and reconnect to a previously connected Video Player (if any). This API is async

 @param clientQueue Queue to invoke callbacks on

 @param startMatterServerCompletionCallback Called after the Matter server has started and connected (or failed to connect) to a
 previously connected video player (if any) are complete
 */
- (void)startMatterServer:(dispatch_queue_t _Nonnull)clientQueue
    startMatterServerCompletionCallback:(nullable void (^)(MatterError * _Nonnull))startMatterServerCompletionCallback;

/**
 @brief Stop the Matter server
 */
- (void)stopMatterServer;

/*!
 @brief Send a ContentLauncher:LaunchURL request to a TV

 @param contentApp Content app endpoint to target

 @param contentUrl URL of the content to launch on the TV

 @param contentDisplayStr Display string value corresponding to the content

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)contentLauncher_launchUrl:(ContentApp * _Nonnull)contentApp
                       contentUrl:(NSString * _Nonnull)contentUrl
                contentDisplayStr:(NSString * _Nonnull)contentDisplayStr
                 responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
               requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a ContentLauncher:LaunchContent request to a TV

 @param contentApp Content app endpoint to target

 @param contentSearch Indicates the content to launch

 @param autoPlay Play Best match automatically if true, otherwise display matches

 @param data App specific data to be passed to the TV

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)contentLauncher_launchContent:(ContentApp * _Nonnull)contentApp
                        contentSearch:(ContentLauncher_ContentSearch * _Nonnull)contentSearch
                             autoPlay:(bool)autoPlay
                                 data:(NSString * _Nullable)data
                     responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                          clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                   requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a MediaPlayback:Play request to a TV

 @param contentApp Content app endpoint to target

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)mediaPlayback_play:(ContentApp * _Nonnull)contentApp
          responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a MediaPlayback:Pause request to a TV

 @param contentApp Content app endpoint to target

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)mediaPlayback_pause:(ContentApp * _Nonnull)contentApp
           responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a MediaPlayback:StopPlayback request to a TV

 @param contentApp Content app endpoint to target

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)mediaPlayback_stopPlayback:(ContentApp * _Nonnull)contentApp
                  responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                       clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a MediaPlayback:Next request to a TV

 @param contentApp Content app endpoint to target

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)mediaPlayback_next:(ContentApp * _Nonnull)contentApp
          responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a MediaPlayback:Previous request to a TV

 @param contentApp Content app endpoint to target

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)mediaPlayback_previous:(ContentApp * _Nonnull)contentApp
              responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                   clientQueue:(dispatch_queue_t _Nonnull)clientQueue
            requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a MediaPlayback:FastForward request to a TV

 @param contentApp Content app endpoint to target

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)mediaPlayback_fastForward:(ContentApp * _Nonnull)contentApp
                 responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
               requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a MediaPlayback:Rewind request to a TV

 @param contentApp Content app endpoint to target

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)mediaPlayback_rewind:(ContentApp * _Nonnull)contentApp
            responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
          requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a MediaPlayback:StartOver request to a TV

 @param contentApp Content app endpoint to target

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)mediaPlayback_startOver:(ContentApp * _Nonnull)contentApp
               responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                    clientQueue:(dispatch_queue_t _Nonnull)clientQueue
             requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a MediaPlayback:Seek request to a TV

 @param contentApp Content app endpoint to target

 @param position the position (in milliseconds) in the media to seek to

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)mediaPlayback_seek:(ContentApp * _Nonnull)contentApp
                  position:(uint64_t)position
          responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a MediaPlayback:SkipForward request to a TV

 @param contentApp Content app endpoint to target

 @param deltaPositionMilliseconds the duration of the time span to skip forward in the media, in milliseconds

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)mediaPlayback_skipForward:(ContentApp * _Nonnull)contentApp
        deltaPositionMilliseconds:(uint64_t)deltaPositionMilliseconds
                 responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
               requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a MediaPlayback:SkipBackward request to a TV

 @param contentApp Content app endpoint to target

 @param deltaPositionMilliseconds the duration of the time span to skip backward in the media, in milliseconds

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)mediaPlayback_skipBackward:(ContentApp * _Nonnull)contentApp
         deltaPositionMilliseconds:(uint64_t)deltaPositionMilliseconds
                  responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                       clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Subscribe to MediaPlayback:CurrentState

 @param contentApp Content app endpoint to target

 @param minInterval Minimum interval between attribute read reports

 @param maxInterval Maximum interval between attribute read reports

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request

 @param successCallback Callback for when a read report is successfully received

 @param failureCallback Callback for when there is a failure in receiving a read report

 @param subscriptionEstablishedCallback Callback for when the requested subscription has been established successfully
 */
- (void)mediaPlayback_subscribeCurrentState:(ContentApp * _Nonnull)contentApp
                                minInterval:(uint16_t)minInterval
                                maxInterval:(uint16_t)maxInterval
                                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                            successCallback:(void (^_Nonnull)(MediaPlayback_PlaybackState))successCallback
                            failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
            subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback;

/*!
 @brief Send a ApplicationLauncher:LaunchApp request to a TV

 @param contentApp Content app endpoint to target

 @param catalogVendorId CSA-issued vendor ID for the catalog

 @param applicationId application identifier, unique within a catalog, expressed as a string, such as "PruneVideo" or "Company X"

 @param data optional app-specific data to be sent to the app

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)applicationLauncher_launchApp:(ContentApp * _Nonnull)contentApp
                      catalogVendorId:(uint16_t)catalogVendorId
                        applicationId:(NSString * _Nonnull)applicationId
                                 data:(NSData * _Nullable)data
                     responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                          clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                   requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a ApplicationLauncher:StopApp request to a TV

 @param contentApp Content app endpoint to target

 @param catalogVendorId CSA-issued vendor ID for the catalog

 @param applicationId application identifier, unique within a catalog, expressed as a string, such as "PruneVideo" or "Company X"

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)applicationLauncher_stopApp:(ContentApp * _Nonnull)contentApp
                    catalogVendorId:(uint16_t)catalogVendorId
                      applicationId:(NSString * _Nonnull)applicationId
                   responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                        clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                 requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a ApplicationLauncher:HideApp request to a TV

 @param contentApp Content app endpoint to target

 @param catalogVendorId CSA-issued vendor ID for the catalog

 @param applicationId application identifier, unique within a catalog, expressed as a string, such as "PruneVideo" or "Company X"

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)applicationLauncher_hideApp:(ContentApp * _Nonnull)contentApp
                    catalogVendorId:(uint16_t)catalogVendorId
                      applicationId:(NSString * _Nonnull)applicationId
                   responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                        clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                 requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a TargetNavigator:NavigateTarget request to a TV

 @param contentApp Content app endpoint to target

 @param target Identifier for the target for UX navigation, contained within one of the TargetInfo objects in the TargetList
 attribute list.

 @param data Optional app-specific data

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)targetNavigator_navigateTarget:(ContentApp * _Nonnull)contentApp
                                target:(uint8_t)target
                                  data:(NSString * _Nullable)data
                      responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Subscribe to TargetNavigator:TargetList

 @param contentApp Content app endpoint to target

 @param minInterval Minimum interval between attribute read reports

 @param maxInterval Maximum interval between attribute read reports

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request

 @param successCallback Callback for when a read report is successfully received

 @param failureCallback Callback for when there is a failure in receiving a read report

 @param subscriptionEstablishedCallback Callback for when the requested subscription has been established successfully
 */
- (void)targetNavigator_subscribeTargetList:(ContentApp * _Nonnull)contentApp
                                minInterval:(uint16_t)minInterval
                                maxInterval:(uint16_t)maxInterval
                                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                            successCallback:(void (^_Nonnull)(NSMutableArray * _Nullable))successCallback
                            failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
            subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback;

/*!
 @brief Send a KeypadInput:SendKey request to a TV

 @param contentApp Content app endpoint to target

 @param keyCode Key Code to process. If a second SendKey request with the same KeyCode value is received within 200ms, then the
 endpoint will consider the first key press to be a press and hold. When such a repeat KeyCode value is not received within 200ms,
 then the endpoint will consider the last key press to be a release.

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)keypadInput_sendKey:(ContentApp * _Nonnull)contentApp
                    keyCode:(uint8_t)keyCode
           responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Read ApplicationBasic:VendorName

 @param contentApp Content app endpoint to target

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request

 @param successCallback Callback for when a read report is successfully received

 @param failureCallback Callback for when there is a failure in receiving a read report
 */
- (void)applicationBasic_readVendorName:(ContentApp * _Nonnull)contentApp
                            clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                     requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                        successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                        failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback;

/*!
 @brief Read ApplicationBasic:VendorID

 @param contentApp Content app endpoint to target

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request

 @param successCallback Callback for when a read report is successfully received

 @param failureCallback Callback for when there is a failure in receiving a read report
 */
- (void)applicationBasic_readVendorID:(ContentApp * _Nonnull)contentApp
                          clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                   requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                      successCallback:(void (^_Nonnull)(NSNumber * _Nonnull))successCallback
                      failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback;

/*!
 @brief Read ApplicationBasic:ApplicationName

 @param contentApp Content app endpoint to target

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request

 @param successCallback Callback for when a read report is successfully received

 @param failureCallback Callback for when there is a failure in receiving a read report
 */
- (void)applicationBasic_readApplicationName:(ContentApp * _Nonnull)contentApp
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                          requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                             successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                             failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback;

/*!
 @brief Read ApplicationBasic:ProductID

 @param contentApp Content app endpoint to target

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request

 @param successCallback Callback for when a read report is successfully received

 @param failureCallback Callback for when there is a failure in receiving a read report
 */
- (void)applicationBasic_readProductID:(ContentApp * _Nonnull)contentApp
                           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                       successCallback:(void (^_Nonnull)(uint16_t))successCallback
                       failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback;

/*!
 @brief Read ApplicationBasic:ApplicationVersion

 @param contentApp Content app endpoint to target

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request

 @param successCallback Callback for when a read report is successfully received

 @param failureCallback Callback for when there is a failure in receiving a read report
 */
- (void)applicationBasic_readApplicationVersion:(ContentApp * _Nonnull)contentApp
                                    clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                             requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                                successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                                failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback;
/*!
 @brief Send a OnOff:On request to a TV

 @param contentApp Content app endpoint to target

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)onOff_on:(ContentApp * _Nonnull)contentApp
      responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a OnOff:Off request to a TV

 @param contentApp Content app endpoint to target

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)onOff_off:(ContentApp * _Nonnull)contentApp
      responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;

/*!
 @brief Send a OnOff:Toggle request to a TV

 @param contentApp Content app endpoint to target

 @param responseCallback Callback for when the response has been received

 @param clientQueue Queue to invoke callbacks on

 @param requestSentHandler Handler to call on sending the request
 */
- (void)onOff_toggle:(ContentApp * _Nonnull)contentApp
      responseCallback:(void (^_Nonnull)(MatterError * _Nonnull))responseCallback
           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler;
@end
#endif /* CastingServerBridge_h */
