/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

/**
 * @brief CastingUtils carries several utility functions that can be used
 * with Linux implementations of the TV Casting app.
 */
#pragma once

#include "CastingServer.h"
#include "TargetEndpointInfo.h"
#include "TargetVideoPlayerInfo.h"

inline constexpr uint32_t kCommissionerDiscoveryTimeoutInMs = 5 * 1000;

CHIP_ERROR ProcessClusterCommand(int argc, char ** argv);

CHIP_ERROR DiscoverCommissioners();

CHIP_ERROR RequestCommissioning(int index);

void PrepareForCommissioning(const chip::Dnssd::CommissionNodeData * selectedCommissioner = nullptr);

void InitCommissioningFlow(intptr_t commandArg);

void doCastingDemoActions(TargetEndpointInfo * endpoint);

void HandleCommissioningCompleteCallback(CHIP_ERROR err);

CHIP_ERROR ConnectToCachedVideoPlayer();

void OnConnectionSuccess(TargetVideoPlayerInfo * videoPlayer);

void OnConnectionFailure(CHIP_ERROR err);

void OnNewOrUpdatedEndpoint(TargetEndpointInfo * endpoint);

void LaunchURLResponseCallback(CHIP_ERROR err);

void OnCurrentStateReadResponseSuccess(
    void * context, chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo::DecodableArgType responseData);

void OnCurrentStateReadResponseFailure(void * context, CHIP_ERROR err);

void OnCurrentStateSubscriptionEstablished(void * context);

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
void HandleUDCSendExpiration(chip::System::Layer * aSystemLayer, void * context);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

void PrintFabrics();
