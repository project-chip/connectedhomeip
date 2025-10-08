/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "AppOptions.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <system/SystemClock.h>

#include <string>

using namespace chip::ArgParser;
using namespace chip::System;
using namespace chip::app::Clusters::TimeSynchronization::Attributes;

using chip::ArgParser::OptionDef;
using chip::ArgParser::OptionSet;
using chip::ArgParser::PrintArgError;
using chip::System::Clock::ClockBase;
using chip::System::Clock::Microseconds64;
using chip::System::Clock::Milliseconds64;

constexpr uint16_t kOptionMinCommissioningTimeout    = 0xFF02;
constexpr uint16_t kOptionEndUserSupportFilePath     = 0xFF03;
constexpr uint16_t kOptionNetworkDiagnosticsFilePath = 0xFF04;
constexpr uint16_t kOptionCrashFilePath              = 0xFF05;
constexpr uint16_t kOptionUseMockClock               = 0xFF06;

namespace {
struct MockClock : public ClockBase
{
private:
    using Offset = std::chrono::duration<int64_t, std::micro>;

    static CHIP_ERROR GetOffsetFrom(ClockBase & aRealClock, const Microseconds64 & aOverride, Offset & aOffset)
    {
        Microseconds64 curTime;
        auto err = aRealClock.GetClock_RealTime(curTime);
        if (err == CHIP_NO_ERROR)
        {
            aOffset = curTime - aOverride;
        }
        else
        {
            aOffset = Clock::kZero;
        }
        return err;
    }

public:
    MockClock() : mRealClock(SystemClock()) { Clock::Internal::SetSystemClockForTesting(this); }
    ~MockClock() { Clock::Internal::SetSystemClockForTesting(&mRealClock); }

    void SetUTCTime(Microseconds64 aOverride) { GetOffsetFrom(mRealClock, aOverride, mOffset); }

    Microseconds64 GetMonotonicMicroseconds64() override { return mRealClock.GetMonotonicMicroseconds64(); }
    Milliseconds64 GetMonotonicMilliseconds64() override { return mRealClock.GetMonotonicMilliseconds64(); }

    CHIP_ERROR GetClock_RealTime(Microseconds64 & aCurTime) override
    {
        auto err = mRealClock.GetClock_RealTime(aCurTime);
        if (err == CHIP_NO_ERROR)
        {
            aCurTime -= mOffset;
        }
        return err;
    }
    CHIP_ERROR GetClock_RealTimeMS(Milliseconds64 & aCurTime) override
    {
        Microseconds64 curTimeUs;
        auto err = GetClock_RealTime(curTimeUs);
        aCurTime = std::chrono::duration_cast<Milliseconds64>(curTimeUs);
        return err;
    }
    CHIP_ERROR SetClock_RealTime(Microseconds64 aNewCurTime) override { return GetOffsetFrom(mRealClock, aNewCurTime, mOffset); }

private:
    ClockBase & mRealClock;
    Offset mOffset;
};

} // namespace

static chip::Optional<std::string> sEndUserSupportLogFilePath;
static chip::Optional<std::string> sNetworkDiagnosticsLogFilePath;
static chip::Optional<std::string> sCrashLogFilePath;
static chip::Optional<MockClock> sMockClock;

bool AppOptions::IsEmptyString(const char * value)
{
    return (value == nullptr || strlen(value) == 0);
}

bool AppOptions::HandleOptions(const char * program, OptionSet * options, int identifier, const char * name, const char * value)
{
    bool retval = true;
    switch (identifier)
    {
    case kOptionMinCommissioningTimeout: {
        auto & commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
        commissionMgr.OverrideMinCommissioningTimeout(chip::System::Clock::Seconds16(static_cast<uint16_t>(atoi(value))));
        break;
    }
    case kOptionEndUserSupportFilePath: {
        if (!IsEmptyString(value))
        {
            sEndUserSupportLogFilePath.SetValue(value);
        }
        break;
    }
    case kOptionNetworkDiagnosticsFilePath: {
        if (!IsEmptyString(value))
        {
            sNetworkDiagnosticsLogFilePath.SetValue(value);
        }
        break;
    }
    case kOptionCrashFilePath: {
        if (!IsEmptyString(value))
        {
            sCrashLogFilePath.SetValue(value);
        }
        break;
    }
    case kOptionUseMockClock: {
        if (!sMockClock.HasValue())
        {
            sMockClock.Emplace();
            // This ensures that the UTCTime attribute will be reported to have a value.
            TimeSource::Set(chip::kRootEndpointId, chip::app::Clusters::TimeSynchronization::TimeSourceEnum::kUnknown);
        }
        long longValue = atoi(value);
        if (longValue >= 0)
        {
            uint64_t override = uint64_t(longValue) * chip::kMicrosecondsPerSecond;
            retval            = chip::ChipEpochToUnixEpochMicros(override, override);
            sMockClock.Value().SetUTCTime(Microseconds64(override));
        }
        break;
    }
    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", program, name);
        retval = false;
        break;
    }

    return retval;
}

OptionSet * AppOptions::GetOptions()
{
    static OptionDef optionsDef[] = {
        { "min_commissioning_timeout", kArgumentRequired, kOptionMinCommissioningTimeout },
        { "end_user_support_log", kArgumentRequired, kOptionEndUserSupportFilePath },
        { "network_diagnostics_log", kArgumentRequired, kOptionNetworkDiagnosticsFilePath },
        { "crash_log", kArgumentRequired, kOptionCrashFilePath },
        { "use_mock_clock", kArgumentRequired, kOptionUseMockClock },
        {},
    };

    static OptionSet options = {
        AppOptions::HandleOptions, optionsDef, "PROGRAM OPTIONS",
        "  --min_commissioning_timeout <value>\n"
        "       The minimum time in seconds during which commissioning session establishment is allowed by the Node.\n"
        "  --end_user_support_log <value>\n"
        "       The end user support log file to be used for diagnostic logs transfer.\n"
        "  --network_diagnostics_log <value>\n"
        "       The network diagnostics log file to be used for diagnostic logs transfer.\n"
        "  --crash_log <value>\n"
        "       The crash log file to be used for diagnostic logs transfer.\n"
        "  --use_mock_clock <value>\n"
        "       Forces the use of a mock clock, to enable setting an incorrect initial UTC time (value is treated as a Matter "
        "       epoch time in seconds, like the UTCTime attribute of the Time Synchronization cluster).\n"
    };

    return &options;
}

chip::Optional<std::string> AppOptions::GetEndUserSupportLogFilePath()
{
    return sEndUserSupportLogFilePath;
}

chip::Optional<std::string> AppOptions::GetNetworkDiagnosticsLogFilePath()
{
    return sNetworkDiagnosticsLogFilePath;
}

chip::Optional<std::string> AppOptions::GetCrashLogFilePath()
{
    return sCrashLogFilePath;
}
