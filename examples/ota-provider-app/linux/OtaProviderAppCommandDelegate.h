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

#pragma once

#include "NamedPipeCommands.h"

#include <json/json.h>
#include <ota-provider-common/OTAProviderExample.h>
#include <platform/DiagnosticDataProvider.h>

#include <string>

class OTAProviderExample;

OTAProviderExample & GetOtaProviderExample();

class OtaProviderAppCommandDelegate : public NamedPipeCommandDelegate
{
public:
    void OnEventCommandReceived(const char * json) override;
    void SetPipes(NamedPipeCommands * pipes) { mPipes = pipes; }
    NamedPipeCommands * GetPipes() const { return mPipes; }

private:
    NamedPipeCommands * mPipes = nullptr;
};

// This class is a helper that parses and validates incoming JSON commands received over the out-of-band channel.
// It is separated from OtaProviderAppCommandDelegate to:
//  - Keep JSON parsing and command dispach logic isolated from OTA logic
//  - Allow early validation and reporting
//  - Extracts generic command fields and delegates the actual OTA operations to OtaProviderAppCommandDelegate
 
class OtaProviderAppCommandHandler
{
public:
    static OtaProviderAppCommandHandler * FromJSON(const char * json, OtaProviderAppCommandDelegate * delegate);

    static void HandleCommand(intptr_t context);
    Json::Value BuildOtaProviderSnapshot(uint16_t endpoint);

    OtaProviderAppCommandHandler(Json::Value && v, OtaProviderAppCommandDelegate * d) : mCommandPayload(std::move(v)), mDelegate(d)
    {}

private:
    Json::Value mCommandPayload;
    OtaProviderAppCommandDelegate * mDelegate = nullptr;
};
