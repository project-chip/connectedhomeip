/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief CastingUtils carries several utility functions that can be used
 * with Linux implementations of the TV Casting app.
 */
#pragma once

#include "CastingServer.h"
#include "TargetEndpointInfo.h"
#include "TargetVideoPlayerInfo.h"

constexpr uint32_t kCommissionerDiscoveryTimeoutInMs = 5 * 1000;

CHIP_ERROR ProcessClusterCommand(int argc, char ** argv);

CHIP_ERROR DiscoverCommissioners();

CHIP_ERROR RequestCommissioning(int index);

void PrepareForCommissioning(const chip::Dnssd::DiscoveredNodeData * selectedCommissioner = nullptr);

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
