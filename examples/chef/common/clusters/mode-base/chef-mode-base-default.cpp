/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "chef-mode-base-default.h"
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeBase;

CHIP_ERROR DefaultChefDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void DefaultChefDelegate::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    EndpointId endpointId = GetInstance()->GetEndpointId();
    ChipLogDetail(DeviceLayer, "DefaultChefDelegate::HandleChangeToMode: Endpoint %d", endpointId);
    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR DefaultChefDelegate::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    EndpointId endpointId = GetInstance()->GetEndpointId();
    ChipLogDetail(DeviceLayer, "DefaultChefDelegate::GetModeLabelByIndex: Endpoint %d", endpointId);
    if (modeIndex >= mModeOptions.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(mModeOptions[modeIndex].label, label);
}

CHIP_ERROR DefaultChefDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    EndpointId endpointId = GetInstance()->GetEndpointId();
    ChipLogDetail(DeviceLayer, "DefaultChefDelegate::GetModeValueByIndex: Endpoint %d", endpointId);
    if (modeIndex >= mModeOptions.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = mModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultChefDelegate::GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<detail::Structs::ModeTagStruct::Type> & tags)
{
    EndpointId endpointId = GetInstance()->GetEndpointId();
    ChipLogDetail(DeviceLayer, "DefaultChefDelegate::GetModeTagsByIndex: Endpoint %d", endpointId);
    if (modeIndex >= mModeOptions.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    if (tags.size() < mModeOptions[modeIndex].modeTags.size())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::copy(mModeOptions[modeIndex].modeTags.begin(), mModeOptions[modeIndex].modeTags.end(), tags.begin());
    tags.reduce_size(mModeOptions[modeIndex].modeTags.size());

    return CHIP_NO_ERROR;
}
