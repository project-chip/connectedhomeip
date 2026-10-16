/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app_options/AppOptions.h>
#include <platform/CHIPDeviceConfig.h>
#include <pw_unit_test/framework.h>

#include <cstdint>
#include <string>
#include <vector>

namespace {

TEST(TestAppOptionsPortNumber, AcceptsPortsAcrossTheWholeRange)
{
    uint16_t port = 0;

    EXPECT_TRUE(AppOptions::ParsePortNumber("5540", port));
    EXPECT_EQ(port, static_cast<uint16_t>(5540));

    EXPECT_TRUE(AppOptions::ParsePortNumber("33000", port));
    EXPECT_EQ(port, static_cast<uint16_t>(33000));

    // 0 stays acceptable: --port already allowed it before the parsing was shared.
    EXPECT_TRUE(AppOptions::ParsePortNumber("0", port));
    EXPECT_EQ(port, static_cast<uint16_t>(0));

    EXPECT_TRUE(AppOptions::ParsePortNumber("65535", port));
    EXPECT_EQ(port, static_cast<uint16_t>(65535));
}

TEST(TestAppOptionsPortNumber, AcceptsHexadecimalAndOctalNotation)
{
    uint16_t port = 0;

    EXPECT_TRUE(AppOptions::ParsePortNumber("0x80e8", port));
    EXPECT_EQ(port, static_cast<uint16_t>(33000));

    EXPECT_TRUE(AppOptions::ParsePortNumber("010", port));
    EXPECT_EQ(port, static_cast<uint16_t>(8));
}

TEST(TestAppOptionsPortNumber, RejectsValuesOutsideAUint16)
{
    uint16_t port = 1;

    EXPECT_FALSE(AppOptions::ParsePortNumber("65536", port));
    EXPECT_FALSE(AppOptions::ParsePortNumber("70000", port));
    EXPECT_FALSE(AppOptions::ParsePortNumber("-1", port));

    // Rejected input must leave the caller's value untouched.
    EXPECT_EQ(port, static_cast<uint16_t>(1));
}

TEST(TestAppOptionsPortNumber, RejectsInputThatIsNotEntirelyNumeric)
{
    uint16_t port = 1;

    EXPECT_FALSE(AppOptions::ParsePortNumber(nullptr, port));
    EXPECT_FALSE(AppOptions::ParsePortNumber("", port));
    EXPECT_FALSE(AppOptions::ParsePortNumber("abc", port));
    EXPECT_FALSE(AppOptions::ParsePortNumber("5540x", port));
    EXPECT_FALSE(AppOptions::ParsePortNumber("5540 ", port));

    EXPECT_EQ(port, static_cast<uint16_t>(1));
}

TEST(TestAppOptionsRpcServerPort, IsUnsetUntilTheOptionIsGiven)
{
    // Left unset so that a non-PW_RPC build can tell "--rpc-server-port 33000" apart from the
    // option not being passed at all, and warn in the former case.
    const AppOptions::AppConfig defaults;
    EXPECT_FALSE(defaults.rpcServerPort.has_value());

    // Keep in sync with LinuxDeviceOptions::rpcServerPort so that tooling which does not pass
    // --rpc-server-port keeps reaching the app.
    EXPECT_EQ(AppOptions::kDefaultRpcServerPort, static_cast<uint16_t>(33000));
    EXPECT_EQ(defaults.rpcServerPort.value_or(AppOptions::kDefaultRpcServerPort), AppOptions::kDefaultRpcServerPort);
}

} // namespace

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

namespace {

using FreqList = std::vector<uint16_t>;

TEST(TestAppOptionsWiFiPafFreqList, NoFreqListKey)
{
    EXPECT_EQ(AppOptions::ParseWiFiPafFreqList(""), FreqList{});
    EXPECT_EQ(AppOptions::ParseWiFiPafFreqList("2437"), FreqList{});
    EXPECT_EQ(AppOptions::ParseWiFiPafFreqList("other_key=2437"), FreqList{});
}

TEST(TestAppOptionsWiFiPafFreqList, SingleFrequency)
{
    EXPECT_EQ(AppOptions::ParseWiFiPafFreqList("freq_list=2437"), FreqList{ 2437 });
}

TEST(TestAppOptionsWiFiPafFreqList, MultipleFrequenciesKeepTheirOrder)
{
    // Order matters: the subscribe channel falls back to the first entry when the
    // default publish channel is not listed.
    EXPECT_EQ(AppOptions::ParseWiFiPafFreqList("freq_list=5745,2412,2437"), (FreqList{ 5745, 2412, 2437 }));
}

TEST(TestAppOptionsWiFiPafFreqList, KeyFoundAfterOtherArguments)
{
    EXPECT_EQ(AppOptions::ParseWiFiPafFreqList("some_key=1 freq_list=2412,2437"), (FreqList{ 2412, 2437 }));
}

TEST(TestAppOptionsWiFiPafFreqList, ParsingStopsAtASpace)
{
    EXPECT_EQ(AppOptions::ParseWiFiPafFreqList("freq_list=2437,2412 other_key=5745"), (FreqList{ 2437, 2412 }));
}

TEST(TestAppOptionsWiFiPafFreqList, NonNumericValueYieldsNothing)
{
    EXPECT_EQ(AppOptions::ParseWiFiPafFreqList("freq_list=abc"), FreqList{});
    EXPECT_EQ(AppOptions::ParseWiFiPafFreqList("freq_list="), FreqList{});
}

TEST(TestAppOptionsWiFiPafFreqList, ParsingStopsAtTheFirstNonNumericEntry)
{
    EXPECT_EQ(AppOptions::ParseWiFiPafFreqList("freq_list=2437,abc,2412"), FreqList{ 2437 });
}

TEST(TestAppOptionsWiFiPafFreqList, TrailingCommaIsTolerated)
{
    EXPECT_EQ(AppOptions::ParseWiFiPafFreqList("freq_list=2437,"), FreqList{ 2437 });
}

TEST(TestAppOptionsWiFiPafFreqList, OutOfRangeEntriesAreSkipped)
{
    // 0 is not a frequency, and anything past a uint16_t cannot be sent to
    // wpa_supplicant; neither should displace the valid entries around it.
    EXPECT_EQ(AppOptions::ParseWiFiPafFreqList("freq_list=0,2437"), FreqList{ 2437 });
    EXPECT_EQ(AppOptions::ParseWiFiPafFreqList("freq_list=70000,2437"), FreqList{ 2437 });
}

} // namespace

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
