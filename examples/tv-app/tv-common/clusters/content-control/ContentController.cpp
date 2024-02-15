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

using namespace std;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::ContentControl;

ContentControlManager::ContentControlManager()
{
    // Create Test Data
}

// Attribute Delegates
bool ContentControlManager::HandleGetEnabled()
{
    return false;
}

CHIP_ERROR ContentControlManager::HandleGetOnDemandRatings(chip::app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(chip::CharSpan());
}

chip::CharSpan ContentControlManager::HandleGetOnDemandRatingThreshold()
{
    return chip::CharSpan();
}

CHIP_ERROR ContentControlManager::HandleGetScheduledContentRatings(chip::app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(chip::CharSpan());
}

chip::CharSpan ContentControlManager::HandleGetScheduledContentRatingThreshold()
{
    return chip::CharSpan();
}

uint32_t ContentControlManager::HandleGetScreenDailyTime()
{
    return (uint32_t) 0xFFFFFFFF;
}

uint32_t ContentControlManager::HandleGetRemainingScreenTime()
{
    return (uint32_t) 0xFFFFFFFF;
}

bool ContentControlManager::HandleGetBlockUnrated()
{
    return false;
}

// Command Delegates
void ContentControlManager::HandleUpdatePIN(chip::Optional<chip::CharSpan> oldPIN, chip::CharSpan newPIN) {}

void ContentControlManager::HandleResetPIN(chip::app::CommandResponseHelper<ResetPINResponseType> & helper) {}

void ContentControlManager::HandleEnable() {}

void ContentControlManager::HandleDisable() {}

void ContentControlManager::HandleAddBonusTime(chip::Optional<chip::CharSpan> PINCode, chip::Optional<uint32_t> bonusTime) {}

void ContentControlManager::HandleSetScreenDailyTime(uint32_t screenDailyTime) {}

void ContentControlManager::HandleBlockUnratedContent() {}

void ContentControlManager::HandleUnblockUnratedContent() {}

void ContentControlManager::HandleSetOnDemandRatingThreshold(chip::CharSpan rating) {}

void ContentControlManager::HandleSetScheduledContentRatingThreshold(chip::CharSpan rating) {}

uint32_t ContentControlManager::GetFeatureMap(chip::EndpointId endpoint)
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
