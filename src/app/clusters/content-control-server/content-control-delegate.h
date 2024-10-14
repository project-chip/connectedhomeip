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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandResponseHelper.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ContentControl {

/** @brief
 *    Defines methods for implementing application-specific logic for the Content Control Cluster.
 */
class Delegate
{
public:
    // Attribute Delegates
    virtual bool HandleGetEnabled()                                                            = 0;
    virtual CHIP_ERROR HandleGetOnDemandRatings(app::AttributeValueEncoder & aEncoder)         = 0;
    virtual chip::CharSpan HandleGetOnDemandRatingThreshold()                                  = 0;
    virtual CHIP_ERROR HandleGetScheduledContentRatings(app::AttributeValueEncoder & aEncoder) = 0;
    virtual chip::CharSpan HandleGetScheduledContentRatingThreshold()                          = 0;
    virtual uint32_t HandleGetScreenDailyTime()                                                = 0;
    virtual uint32_t HandleGetRemainingScreenTime()                                            = 0;
    virtual bool HandleGetBlockUnrated()                                                       = 0;

    // Command Delegates
    virtual void HandleUpdatePIN(Optional<chip::CharSpan> oldPIN, chip::CharSpan newPIN)            = 0;
    virtual void HandleResetPIN(CommandResponseHelper<Commands::ResetPINResponse::Type> & helper)   = 0;
    virtual void HandleEnable()                                                                     = 0;
    virtual void HandleDisable()                                                                    = 0;
    virtual void HandleAddBonusTime(Optional<chip::CharSpan> PINCode, Optional<uint32_t> bonusTime) = 0;
    virtual void HandleSetScreenDailyTime(uint32_t screenDailyTime)                                 = 0;
    virtual void HandleBlockUnratedContent()                                                        = 0;
    virtual void HandleUnblockUnratedContent()                                                      = 0;
    virtual void HandleSetOnDemandRatingThreshold(chip::CharSpan rating)                            = 0;
    virtual void HandleSetScheduledContentRatingThreshold(chip::CharSpan rating)                    = 0;

    bool HasFeature(chip::EndpointId endpoint, Feature feature);

    virtual uint32_t GetFeatureMap(chip::EndpointId endpoint) = 0;

    virtual ~Delegate() = default;
};

} // namespace ContentControl
} // namespace Clusters
} // namespace app
} // namespace chip
