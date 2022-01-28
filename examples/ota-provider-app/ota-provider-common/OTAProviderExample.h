/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/CommandHandler.h>
#include <app/clusters/ota-provider/ota-provider-delegate.h>
#include <ota-provider-common/BdxOtaSender.h>

/**
 * A reference implementation for an OTA Provider. Includes a method for providing a path to a local OTA file to serve.
 */
class OTAProviderExample : public chip::app::Clusters::OTAProviderDelegate
{
public:
    OTAProviderExample();

    void SetOTAFilePath(const char * path);
    BdxOtaSender * GetBdxOtaSender() { return &mBdxOtaSender; }

    // Inherited from OTAProviderDelegate
    EmberAfStatus HandleQueryImage(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData) override;
    EmberAfStatus HandleApplyUpdateRequest(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData) override;
    EmberAfStatus HandleNotifyUpdateApplied(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData) override;

    enum QueryImageBehaviorType
    {
        kRespondWithUpdateAvailable,
        kRespondWithBusy,
        kRespondWithNotAvailable
    };
    void SetQueryImageBehavior(QueryImageBehaviorType behavior) { mQueryImageBehavior = behavior; }
    void SetDelayedActionTimeSec(uint32_t time) { mDelayedActionTimeSec = time; }

private:
    BdxOtaSender mBdxOtaSender;
    static constexpr size_t kFilepathBufLen = 256;
    char mOTAFilePath[kFilepathBufLen]; // null-terminated
    QueryImageBehaviorType mQueryImageBehavior;
    uint32_t mDelayedActionTimeSec;
};
