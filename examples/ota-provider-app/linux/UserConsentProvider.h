/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <ota-provider-common/UserConsentDelegate.h>

class UserConsentProvider : public chip::ota::UserConsentDelegate
{
public:
    void ObtainUserConsentAsync(chip::NodeId nodeId, chip::EndpointId endpoint, uint32_t currentVersion, uint32_t newVersion)
    {
        mNodeId         = nodeId;
        mEndpoint       = endpoint;
        mCurrentVersion = currentVersion;
        mNewVersion     = newVersion;

        // This is not an async operation, std::cin blocks the chip thread
        // TODO: Spawn a new thread for getting user consent
        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(1), &HandleUserConsent, this);
    }

private:
    chip::NodeId mNodeId       = chip::kUndefinedNodeId;
    chip::EndpointId mEndpoint = chip::kInvalidEndpointId;
    uint32_t mCurrentVersion   = 0;
    uint32_t mNewVersion       = 0;

    static void HandleUserConsent(chip::System::Layer * systemLayer, void * args)
    {
        int userConsent                = 0;
        UserConsentProvider * provider = static_cast<UserConsentProvider *>(args);

        std::cout << "============================================================\n";
        std::cout << "OTA upgrade available:\n";
        std::cout << "  Endpoint: " << provider->mEndpoint << "\n";
        std::cout << "  Node: " << provider->mNodeId << "\n";
        std::cout << "  Current version: " << provider->mCurrentVersion << "\n";
        std::cout << "  New version: " << provider->mNewVersion << "\n";
        std::cout << "============================================================\n";
        std::cout << "Continue?[0/1]: ";
        std::cin >> userConsent;

        if (provider->userConsentCallback)
        {
            provider->userConsentCallback(userConsent == 1, provider->mEndpoint, provider->mNodeId);
        }
    }
};
