/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "ContentController.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/config.h>

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::ContentControl;

ContentController::ContentController()
{
    // Create Test Data
}

// Attribute Delegates
bool ContentController::HandleGetEnabled()
{
    return false;
}

CHIP_ERROR ContentController::HandleGetOnDemandRatings(chip::app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(chip::CharSpan());
}

chip::CharSpan ContentController::HandleGetOnDemandRatingThreshold()
{
    return chip::CharSpan();
}

CHIP_ERROR ContentController::HandleGetScheduledContentRatings(chip::app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(chip::CharSpan());
}

chip::CharSpan ContentController::HandleGetScheduledContentRatingThreshold()
{
    return chip::CharSpan();
}

uint32_t ContentController::HandleGetScreenDailyTime()
{
    return (uint32_t) 0xFFFFFFFF;
}

uint32_t ContentController::HandleGetRemainingScreenTime()
{
    return (uint32_t) 0xFFFFFFFF;
}

bool ContentController::HandleGetBlockUnrated()
{
    return false;
}

// Command Delegates
void ContentController::HandleUpdatePIN(chip::Optional<chip::CharSpan> oldPIN, chip::CharSpan newPIN) {}

void ContentController::HandleResetPIN(chip::app::CommandResponseHelper<ResetPINResponseType> & helper) {}

void ContentController::HandleEnable() {}

void ContentController::HandleDisable() {}

void ContentController::HandleAddBonusTime(chip::Optional<chip::CharSpan> PINCode, chip::Optional<uint32_t> bonusTime) {}

void ContentController::HandleSetScreenDailyTime(uint32_t screenDailyTime) {}

void ContentController::HandleBlockUnratedContent() {}

void ContentController::HandleUnblockUnratedContent() {}

void ContentController::HandleSetOnDemandRatingThreshold(chip::CharSpan rating) {}

void ContentController::HandleSetScheduledContentRatingThreshold(chip::CharSpan rating) {}

uint32_t ContentController::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CONTENT_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kEndpointFeatureMap;
    }

    uint32_t featureMap = 0;
    // TODO: ReEnable the code bellow
    // Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}
