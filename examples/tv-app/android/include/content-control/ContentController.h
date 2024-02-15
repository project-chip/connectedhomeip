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

#pragma once

#include <app/AttributeAccessInterface.h>
#include <app/clusters/content-control-server/content-control-server.h>
#include <vector>

using ContentControlDelegate = chip::app::Clusters::ContentControl::Delegate;
using ResetPINResponseType   = chip::app::Clusters::ContentControl::Commands::ResetPINResponse::Type;

class ContentController : public ContentControlDelegate
{
public:
    ContentController();

    // Attribute Delegates
    bool HandleGetEnabled() override;
    CHIP_ERROR HandleGetOnDemandRatings(chip::app::AttributeValueEncoder & aEncoder) override;
    chip::CharSpan HandleGetOnDemandRatingThreshold() override;
    CHIP_ERROR HandleGetScheduledContentRatings(chip::app::AttributeValueEncoder & aEncoder) override;
    chip::CharSpan HandleGetScheduledContentRatingThreshold() override;
    uint32_t HandleGetScreenDailyTime() override;
    uint32_t HandleGetRemainingScreenTime() override;
    bool HandleGetBlockUnrated() override;

    // Command Delegates
    void HandleUpdatePIN(chip::Optional<chip::CharSpan> oldPIN, chip::CharSpan newPIN) override;
    void HandleResetPIN(chip::app::CommandResponseHelper<ResetPINResponseType> & helper) override;
    void HandleEnable() override;
    void HandleDisable() override;
    void HandleAddBonusTime(chip::Optional<chip::CharSpan> PINCode, chip::Optional<uint32_t> bonusTime) override;
    void HandleSetScreenDailyTime(uint32_t screenDailyTime) override;
    void HandleBlockUnratedContent() override;
    void HandleUnblockUnratedContent() override;
    void HandleSetOnDemandRatingThreshold(chip::CharSpan rating) override;
    void HandleSetScheduledContentRatingThreshold(chip::CharSpan rating) override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;
    void SetEndpointId(chip::EndpointId epId) { mEndpointId = epId; };

protected:
private:
    // TODO: set this based upon meta data from app
    static constexpr uint32_t kEndpointFeatureMap = 3;
    chip::EndpointId mEndpointId;
};
