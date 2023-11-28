/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#pragma once

#include <jni.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/core/NodeId.h>
#include <lib/support/JniTypeWrappers.h>

#include <app/clusters/ota-provider/ota-provider-delegate.h>

#include "BdxOTASender.h"

constexpr uint8_t kUpdateTokenLen = 32;

class OTAProviderDelegateBridge : public chip::app::Clusters::OTAProviderDelegate
{
public:
    ~OTAProviderDelegateBridge() override;
    CHIP_ERROR Init(chip::System::Layer * systemLayer, chip::Messaging::ExchangeManager * exchangeManager,
                    jobject OTAProviderDelegate);
    void Shutdown();

    /**
     * Called to handle a QueryImage command and is responsible for sending the response (if success) or status (if error). The
     * caller is responsible for validating fields in the command.
     */
    void HandleQueryImage(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData) override;
    /**
     * Called to handle an ApplyUpdateRequest command and is responsible for sending the response (if success) or status (if error).
     * The caller is responsible for validating fields in the command.
     */
    void HandleApplyUpdateRequest(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData) override;
    /**
     * Called to handle a NotifyUpdateApplied command and is responsible for sending the status. The caller is responsible for
     * validating fields in the command.
     */
    void HandleNotifyUpdateApplied(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData) override;

private:
    void sendOTAQueryFailure(uint8_t status);

    chip::JniGlobalReference mOtaProviderDelegate;
    std::unique_ptr<BdxOTASender> mBdxOTASender = nullptr;

    uint8_t mToken[kUpdateTokenLen];
};
