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

#include "av-analysis-node-app.h"

#include <app/server/Server.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

// Ambient contexts this example claims to detect (SupportedAmbientContexts, Fixed)
const std::vector<Descriptor::Structs::SemanticTagStruct::Type> kSupportedAmbientContexts = {
    { std::nullopt, static_cast<uint8_t>(0x49), static_cast<uint8_t>(0x0B),
      MakeOptional(DataModel::Nullable<CharSpan>("Object.Package"_span)) },
    { std::nullopt, static_cast<uint8_t>(0x4B), static_cast<uint8_t>(0x08),
      MakeOptional(DataModel::Nullable<CharSpan>("Activity.Delivery"_span)) },
};

} // namespace

CHIP_ERROR AvAnalysisNodeApp::Init()
{
    ChipLogProgress(AppServer, "AvAnalysisNodeApp: Init on endpoint %u", mEndpointId);

    // The camera's AVSM endpoint, features and stream constraints are discovered from the camera
    ReturnErrorOnFailure(mCameraClient.Init(Server::GetInstance().GetCASESessionManager()));

    // No Zone Management cluster on this endpoint: PerZoneContextDetection is off and MaxZones is Null
    mAvAnalysisServer.Create(mEndpointId, BitFlags<AvAnalysis::Feature>(AvAnalysis::Feature::kRemoteContextDetection),
                             kSupportedAmbientContexts, DataModel::Nullable<uint8_t>(), kMaxAnalysisStreams);
    mAvAnalysisServer.Cluster().SetDelegate(&mAnalysisDelegate);
    mAvAnalysisServer.Cluster().SetCameraClient(&mCameraClient);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mAvAnalysisServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register AVAnalysis on endpoint %u: %" CHIP_ERROR_FORMAT, mEndpointId, err.Format());
        mAvAnalysisServer.Destroy();
        return err;
    }

    return CHIP_NO_ERROR;
}

void AvAnalysisNodeApp::Shutdown()
{
    ChipLogProgress(AppServer, "AvAnalysisNodeApp: Shutdown");

    if (mAvAnalysisServer.IsConstructed())
    {
        CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mAvAnalysisServer.Cluster());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "AVAnalysis unregister error: %" CHIP_ERROR_FORMAT, err.Format());
        }
        mAvAnalysisServer.Destroy();
    }
}

} // namespace app
} // namespace chip
