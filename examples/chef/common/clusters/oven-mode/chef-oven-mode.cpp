/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "chef-oven-mode.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <lib/support/logging/CHIPLogging.h>

#ifdef MATTER_DM_PLUGIN_OVEN_MODE_SERVER

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeBase;
using chip::Protocols::InteractionModel::Status;
template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

namespace ChefOvenMode {

constexpr size_t kOvenModeTableSize = MATTER_DM_OVEN_MODE_CLUSTER_SERVER_ENDPOINT_COUNT;
static_assert(kOvenModeTableSize <= kEmberInvalidEndpointIndex, "OvenMode table size error");

std::unique_ptr<OvenMode::ChefDelegate> gDelegateTable[kOvenModeTableSize];
std::unique_ptr<ModeBase::Instance> gInstanceTable[kOvenModeTableSize];

/**
 * Initializes OvenMode cluster for the app (all endpoints).
 */
void InitChefOvenModeCluster()
{
    const uint16_t endpointCount = emberAfEndpointCount();

    for (uint16_t endpointIndex = 0; endpointIndex < endpointCount; endpointIndex++)
    {
        EndpointId endpointId = emberAfEndpointFromIndex(endpointIndex);
        if (endpointId == kInvalidEndpointId)
        {
            continue;
        }

        // Check if endpoint has OvenMode cluster enabled
        uint16_t epIndex =
            emberAfGetClusterServerEndpointIndex(endpointId, OvenMode::Id, MATTER_DM_OVEN_MODE_CLUSTER_SERVER_ENDPOINT_COUNT);
        if (epIndex >= kOvenModeTableSize)
            continue;

        gDelegateTable[epIndex] = std::make_unique<OvenMode::ChefDelegate>();
        TEMPORARY_RETURN_IGNORED gDelegateTable[epIndex]->Init();

        uint32_t featureMap = 0;
        VerifyOrDieWithMsg(OvenMode::Attributes::FeatureMap::Get(endpointId, &featureMap) == Status::Success, DeviceLayer,
                           "Failed to read OvenMode feature map for endpoint %d", endpointId);
        gInstanceTable[epIndex] =
            std::make_unique<ModeBase::Instance>(gDelegateTable[epIndex].get(), endpointId, OvenMode::Id, featureMap);
        TEMPORARY_RETURN_IGNORED gInstanceTable[epIndex]->Init();

        ChipLogProgress(DeviceLayer, "Endpoint %d OvenMode Initialized.", endpointId);
    }
}
} // namespace ChefOvenMode

CHIP_ERROR OvenMode::ChefDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void OvenMode::ChefDelegate::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    EndpointId endpointId = GetInstance()->GetEndpointId();
    ChipLogDetail(DeviceLayer, "HandleChangeToMode: Endpoint %d", endpointId);
    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR OvenMode::ChefDelegate::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    EndpointId endpointId = GetInstance()->GetEndpointId();
    ChipLogDetail(DeviceLayer, "GetModeLabelByIndex: Endpoint %d", endpointId);
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR OvenMode::ChefDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    EndpointId endpointId = GetInstance()->GetEndpointId();
    ChipLogDetail(DeviceLayer, "GetModeValueByIndex: Endpoint %d", endpointId);
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR OvenMode::ChefDelegate::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
{
    EndpointId endpointId = GetInstance()->GetEndpointId();
    ChipLogDetail(DeviceLayer, "GetModeTagsByIndex: Endpoint %d", endpointId);
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    if (tags.size() < kModeOptions[modeIndex].modeTags.size())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::copy(kModeOptions[modeIndex].modeTags.begin(), kModeOptions[modeIndex].modeTags.end(), tags.begin());
    tags.reduce_size(kModeOptions[modeIndex].modeTags.size());

    return CHIP_NO_ERROR;
}

#endif // MATTER_DM_PLUGIN_OVEN_MODE_SERVER
