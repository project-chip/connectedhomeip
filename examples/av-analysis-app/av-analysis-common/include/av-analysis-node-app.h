/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include "av-analysis-delegate.h"
#include "webrtc-peer-controller.h"
#include "webrtc-requestor-delegate.h"

#include <app/clusters/av-analysis-server/AvAnalysisCluster.h>
#include <app/clusters/av-analysis-server/DefaultAvAnalysisCameraClient.h>
#include <app/clusters/webrtc-transport-requestor-server/WebRTCTransportRequestorCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {

class AvAnalysisNodeApp
{
public:
    explicit AvAnalysisNodeApp(EndpointId aEndpointId) : mEndpointId(aEndpointId) {}

    /**
     * @param aPeerController The platform's WebRTC peer connections
     */
    CHIP_ERROR Init(WebRTCPeerController * aPeerController);

    void Shutdown();

private:
    // Maximum number of concurrently established analysis streams
    static constexpr uint8_t kMaxAnalysisStreams = 2;

    EndpointId mEndpointId = kInvalidEndpointId;

    // Supplied by the platform main, provides the SDP offers and owns the peer connections
    WebRTCPeerController * mPeerController = nullptr;

    AvAnalysisNodeDelegate mAvAnalysisDelegate;
    Clusters::DefaultAvAnalysisCameraClient mCameraClient;
    WebRTCRequestorDelegate mRequestorDelegate;
    LazyRegisteredServerCluster<Clusters::AvAnalysisCluster> mAvAnalysisServer;
    // The receiving end of the WebRTC sessions the AV Analysis cluster initiates
    LazyRegisteredServerCluster<Clusters::WebRTCTransportRequestor::WebRTCTransportRequestorCluster> mWebRTCRequestorServer;
};

} // namespace app
} // namespace chip
