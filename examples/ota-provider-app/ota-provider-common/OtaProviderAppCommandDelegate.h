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

class OtaProviderAppCommandDelegate;
extern OTAProviderExample gOtaProvider;

class OtaProviderAppCommandHandler {
public:
    static OtaProviderAppCommandHandler * FromJSON(const char * json, OtaProviderAppCommandDelegate * delegate);

    static void HandleCommand(intptr_t context);
    Json::Value BuildOtaProviderSnapshot(uint16_t endpoint);

    OtaProviderAppCommandHandler(Json::Value && v, OtaProviderAppCommandDelegate * d)
        : mJsonValue(std::move(v))
        , mDelegate(d)
    {
    }

private:
    Json::Value mJsonValue;
    OtaProviderAppCommandDelegate * mDelegate = nullptr;
};

class OtaProviderAppCommandDelegate : public NamedPipeCommandDelegate {
public:
    void OnEventCommandReceived(const char * json) override;
    void SetPipes(NamedPipeCommands * pipes) { mPipes = pipes; }
    NamedPipeCommands * GetPipes() const { return mPipes; }

private:
    NamedPipeCommands * mPipes = nullptr;
};
