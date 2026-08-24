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

#include "analysis-delegate.h"

#include <app/clusters/av-analysis-server/AvAnalysisCluster.h>
#include <app/clusters/av-analysis-server/DefaultAvAnalysisCameraClient.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {

class AvAnalysisNodeApp
{
public:
    explicit AvAnalysisNodeApp(EndpointId aEndpointId) : mEndpointId(aEndpointId) {}

    CHIP_ERROR Init();

    void Shutdown();

private:
    // Maximum number of concurrently established analysis streams (MaxAnalysisStreamCount, Fixed)
    static constexpr uint8_t kMaxAnalysisStreams = 2;

    EndpointId mEndpointId = kInvalidEndpointId;

    AnalysisDelegate mAnalysisDelegate;
    Clusters::DefaultAvAnalysisCameraClient mCameraClient;
    LazyRegisteredServerCluster<Clusters::AvAnalysisCluster> mAvAnalysisServer;
};

} // namespace app
} // namespace chip
