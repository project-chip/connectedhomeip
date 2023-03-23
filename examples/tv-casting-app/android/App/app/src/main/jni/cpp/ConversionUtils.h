/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
