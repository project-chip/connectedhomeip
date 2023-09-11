/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    CHIP_ERROR SetActionReplyStatus(uint16_t action);
    CHIP_ERROR SetReplyStatus(uint16_t status);
    CHIP_ERROR SetUserConsentStatus(uint16_t status);
    CHIP_ERROR SetUserConsentNeeded(uint16_t status);
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

class OTASoftwareUpdateSetParams : public OTASoftwareUpdateBase {
public:
    OTASoftwareUpdateSetParams()
        : OTASoftwareUpdateBase("set-reply-params")
    {
        AddArgument("action", 0, UINT16_MAX, &mAction);
        AddArgument("status", 0, UINT16_MAX, &mStatus);
        AddArgument("consent", 0, UINT16_MAX, &mUserConsentStatus);
        AddArgument("consentNeeded", 0, UINT16_MAX, &mUserConsentNeeded);
        AddArgument("delayedActionTime", 0, UINT64_MAX, &mDelayedActionTime);
        AddArgument("timedInvokeTimeoutMs", 0, UINT64_MAX, &mTimedInvokeTimeoutMs);
    }

    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override;

    CHIP_ERROR SetParams(chip::Optional<uint16_t> action, chip::Optional<uint16_t> status, chip::Optional<uint16_t> consent,
        chip::Optional<uint16_t> userConsentNeeded, chip::Optional<uint64_t> delayedActionTime,
        chip::Optional<uint64_t> timedInvokeTimeoutMs);

private:
    chip::Optional<uint16_t> mAction;
    chip::Optional<uint16_t> mStatus;
    chip::Optional<uint16_t> mUserConsentStatus;
    chip::Optional<uint16_t> mUserConsentNeeded;
    chip::Optional<uint64_t> mDelayedActionTime;
    chip::Optional<uint64_t> mTimedInvokeTimeoutMs;
};
