/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "../common/CHIPCommandBridge.h"
#include "OTAProviderDelegate.h"

static constexpr uint16_t SW_VER_STR_MAX_LEN = 64;
static constexpr uint16_t OTA_URL_MAX_LEN = 512;

class OTASoftwareUpdateBase : public CHIPCommandBridge {
public:
    OTASoftwareUpdateBase(const char * _Nonnull commandName)
        : CHIPCommandBridge(commandName)
    {
    }
    CHIP_ERROR SetCandidatesFromFilePath(char * _Nonnull filePath);
    CHIP_ERROR SetUserConsentStatus(char * _Nonnull status);
    static constexpr size_t kFilepathBufLen = 256;

    CHIP_ERROR Run() override;

    /////////// CHIPCommandBridge Interface /////////
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(120); }

private:
    NSString * _Nullable mOTAFilePath;
    void SetOTAFilePath(const char * _Nonnull path);
};

class OTASoftwareUpdateSetFilePath : public OTASoftwareUpdateBase {
public:
    OTASoftwareUpdateSetFilePath()
        : OTASoftwareUpdateBase("candidate-file-path")
    {
        AddArgument("path", &mOTACandidatesFilePath);
    }

    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override;

private:
    char * _Nonnull mOTACandidatesFilePath;
};

class OTASoftwareUpdateSetStatus : public OTASoftwareUpdateBase {
public:
    OTASoftwareUpdateSetStatus()
        : OTASoftwareUpdateBase("set-consent-status")
    {
        AddArgument("status", &mUserConsentStatus);
    }

    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override;

private:
    char * _Nonnull mUserConsentStatus;
};
