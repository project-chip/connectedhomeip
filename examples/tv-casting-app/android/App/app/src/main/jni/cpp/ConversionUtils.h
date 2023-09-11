/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "AppParams.h"
#include "TargetEndpointInfo.h"
#include "TargetVideoPlayerInfo.h"

#include <lib/dnssd/Resolver.h>

CHIP_ERROR convertJAppParametersToCppAppParams(jobject appParameters, AppParams & outAppParams);

CHIP_ERROR convertJContentAppToTargetEndpointInfo(jobject contentApp, TargetEndpointInfo & outTargetEndpointInfo);

CHIP_ERROR convertTargetEndpointInfoToJContentApp(TargetEndpointInfo * targetEndpointInfo, jobject & outContentApp);

CHIP_ERROR convertJVideoPlayerToTargetVideoPlayerInfo(jobject videoPlayer, TargetVideoPlayerInfo & targetVideoPlayerInfo);

CHIP_ERROR convertTargetVideoPlayerInfoToJVideoPlayer(TargetVideoPlayerInfo * targetVideoPlayerInfo, jobject & outVideoPlayer);

CHIP_ERROR convertJDiscoveredNodeDataToCppDiscoveredNodeData(jobject jDiscoveredNodeData,
                                                             chip::Dnssd::DiscoveredNodeData & cppDiscoveredNodeData);
