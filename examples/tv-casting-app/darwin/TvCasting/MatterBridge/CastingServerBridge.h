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

#import "DiscoveredNodeData.h"
#import "OnboardingPayload.h"
#import <Foundation/Foundation.h>

#ifndef CastingServerBridge_h
#define CastingServerBridge_h

@interface CastingServerBridge : NSObject

@property void (^_Nullable commissioningCompleteCallback)(bool);

@property void (^_Nullable launchUrlResponseCallback)(bool);

@property OnboardingPayload * _Nonnull onboardingPayload;

+ (CastingServerBridge * _Nullable)getSharedInstance;

/*!
 @brief Browse for on-network commissioner TVs

 @param clientQueue Queue to dispatch the call to the discoveryRequestSentHandler on

 @param discoveryRequestSentHandler Handler to call after the Commissioner discovery request has been sent
 */
- (void)discoverCommissioners:(dispatch_queue_t _Nonnull)clientQueue
    discoveryRequestSentHandler:(nullable void (^)(bool))discoveryRequestSentHandler;

/*!
 @brief Retrieve a discovered commissioner TV

 @param index Index in the list of discovered commissioners

 @param clientQueue Queue to dispatch the call to the discoveredCommissionerHandler on

 @param discoveredCommissionerHandler Handler to call after a discovered commissioner has been retrieved
 */
- (void)getDiscoveredCommissioner:(int)index
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    discoveredCommissionerHandler:(nullable void (^)(DiscoveredNodeData * _Nullable))discoveredCommissionerHandler;

/*!
 @brief Send a User Directed Commissioning request to a commissioner TV

 @param commissionerIpAddress IP address of the commissioner

 @param commissionerPort Port number at which the commissioner is listening for User Directed Commissioning requests

 @param platformInterface Platform representation of the commissioner's IP address's interface

 @param clientQueue Queue to dispatch the call to the udcRequestSentHandler on

 @param udcRequestSentHandler Handler to call on sending the User Directed Commissioning request
 */
- (void)sendUserDirectedCommissioningRequest:(NSString * _Nonnull)commissionerIpAddress
                            commissionerPort:(uint16_t)commissionerPort
                           platformInterface:(unsigned int)platformInterface
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                       udcRequestSentHandler:(nullable void (^)(bool))udcRequestSentHandler;

/*!
 @brief Return the onboarding payload for this app (setup passcode, discriminator)

 @return Onboarding payload
 */
- (OnboardingPayload * _Nonnull)getOnboardingPaylod;

/*!
 @brief Request opening of a basic commissioning window

 @param commissioningCompleteCallback Callback for when commissioning of this app has been completed  via a call to the general
 commissioning cluster (by usually an on-network TV/Media device acting as a Matter commissioner)

 @param clientQueue Queue to dispatch the call to the commissioningWindowRequestedHandler on

 @param commissioningWindowRequestedHandler Handler to call on requesting the opening of a commissioning window
 */
- (void)openBasicCommissioningWindow:(nullable void (^)(bool))commissioningCompleteCallback
                            clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    commissioningWindowRequestedHandler:(nullable void (^)(bool))commissioningWindowRequestedHandler;

/*!
 @brief Send a Content Launcher:LaunchURL request to a TV

 @param contentUrl URL of the content to launch on the TV

 @param contentDisplayStr Display string value corresponding to the content

 @param launchUrlResponseCallback Callback for when the Launch URL response has been received

 @param clientQueue Queue to dispatch the call to the launchUrlRequestSentHandler on

 @param launchUrlRequestSentHandler Handler to call on sending the Launch URL request
 */
- (void)contentLauncherLaunchUrl:(NSString * _Nonnull)contentUrl
               contentDisplayStr:(NSString * _Nonnull)contentDisplayStr
       launchUrlResponseCallback:(nullable void (^)(bool))launchUrlResponseCallback
                     clientQueue:(dispatch_queue_t _Nonnull)clientQueue
     launchUrlRequestSentHandler:(nullable void (^)(bool))launchUrlRequestSentHandler;
@end

#endif /* CastingServerBridge_h */
