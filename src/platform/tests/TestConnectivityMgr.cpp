/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *      This file implements a unit test suite for the Connectivity Manager
 *      code functionality.
 *
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DiagnosticDataProvider.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI || CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
#include <platform/Linux/WpaSupplicantClient.h>

#include <initializer_list>
#endif

using namespace chip;
using namespace chip::Logging;
using namespace chip::Inet;
using namespace chip::DeviceLayer;

// =================================
//      Unit tests
// =================================

struct TestConnectivityMgr : public ::testing::Test
{

    static void SetUpTestSuite()
    {
        auto err = chip::Platform::MemoryInit();
        EXPECT_EQ(err, CHIP_NO_ERROR);
        // TODO: Move initialization of the platform manager from Init test to here
    }

    static void TearDownTestSuite()
    {
        chip::Platform::MemoryShutdown();
        chip::DeviceLayer::PlatformMgr().Shutdown();
    }
};

TEST_F(TestConnectivityMgr, Init)
{
    // ConfigurationManager is initialized from PlatformManager indirectly
    CHIP_ERROR err = PlatformMgr().InitChipStack();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestConnectivityMgr, GetNetworkInterfaces)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    NetworkInterface * netifs = nullptr;

    err = GetDiagnosticDataProvider().GetNetworkInterfaces(&netifs);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_NE(netifs, nullptr);

    GetDiagnosticDataProvider().ReleaseNetworkInterfaces(netifs);
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI || CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
namespace {

GAutoPtr<GVariant> SecurityProperties(std::initializer_list<const gchar *> values)
{
    GVariantBuilder dictionary;
    g_variant_builder_init(&dictionary, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&dictionary, "{sv}", "KeyMgmt", g_variant_new_strv(values.begin(), static_cast<gssize>(values.size())));
    return GAutoPtr<GVariant>(g_variant_ref_sink(g_variant_builder_end(&dictionary)));
}

TEST(TestWiFiSecurity, OpenAndMalformedProperties)
{
    EXPECT_EQ(Internal::GetWiFiSecurity(nullptr, nullptr).Raw(), 0x01);
    auto empty = SecurityProperties({});
    EXPECT_EQ(Internal::GetWiFiSecurity(empty.get(), empty.get()).Raw(), 0x01);
    GAutoPtr<GVariant> invalid(g_variant_ref_sink(g_variant_new_string("invalid")));
    EXPECT_EQ(Internal::GetWiFiSecurity(invalid.get(), invalid.get()).Raw(), 0x01);
    GVariantBuilder dictionary;
    g_variant_builder_init(&dictionary, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&dictionary, "{sv}", "KeyMgmt", g_variant_new_uint32(1));
    GAutoPtr<GVariant> wrongType(g_variant_ref_sink(g_variant_builder_end(&dictionary)));
    EXPECT_EQ(Internal::GetWiFiSecurity(wrongType.get(), wrongType.get()).Raw(), 0x01);
}

TEST(TestWiFiSecurity, NonFtEap)
{
    for (const auto * keyMgmt :
         { "wpa-eap", "wpa-eap-sha256", "wpa-eap-suite-b", "wpa-eap-suite-b-192", "wpa-eap-sha384", "WPA-EAP-SHA384" })
    {
        auto properties = SecurityProperties({ keyMgmt });
        EXPECT_EQ(Internal::GetWiFiSecurity(properties.get(), nullptr).Raw(), 0x20);
        EXPECT_EQ(Internal::GetWiFiSecurity(nullptr, properties.get()).Raw(), 0x20);
        auto mixed = SecurityProperties({ keyMgmt, "wpa-psk", "sae-ext-key" });
        EXPECT_EQ(Internal::GetWiFiSecurity(nullptr, mixed.get()).Raw(), 0x38);
    }
}

TEST(TestWiFiSecurity, FtOnlyAndUnknownAkmDoNotAdvertiseEap)
{
    for (const auto * keyMgmt : { "wpa-ft-eap", "wpa-ft-eap-sha384", "ft-sae", "ft-sae-ext-key", "unknown-akm" })
    {
        auto properties = SecurityProperties({ keyMgmt });
        EXPECT_EQ(Internal::GetWiFiSecurity(properties.get(), nullptr).Raw(), 0x00);
        EXPECT_EQ(Internal::GetWiFiSecurity(nullptr, properties.get()).Raw(), 0x00);
        auto mixed = SecurityProperties({ keyMgmt, "wpa-psk" });
        EXPECT_EQ(Internal::GetWiFiSecurity(nullptr, mixed.get()).Raw(), 0x08);
    }
}

TEST(TestWiFiSecurity, PersonalAndMixedSecurityUseIndependentInputs)
{
    auto psk = SecurityProperties({ "wpa-psk" });
    EXPECT_EQ(Internal::GetWiFiSecurity(psk.get(), nullptr).Raw(), 0x04);
    auto none = SecurityProperties({ "wpa-none" });
    EXPECT_EQ(Internal::GetWiFiSecurity(none.get(), nullptr).Raw(), 0x04);
    EXPECT_EQ(Internal::GetWiFiSecurity(nullptr, none.get()).Raw(), 0x00);
    auto personal = SecurityProperties({ "wpa-psk", "wpa-psk-sha256", "wpa-ft-psk", "sae" });
    EXPECT_EQ(Internal::GetWiFiSecurity(nullptr, personal.get()).Raw(), 0x18);
    auto extended = SecurityProperties({ "sae-ext-key" });
    EXPECT_EQ(Internal::GetWiFiSecurity(nullptr, extended.get()).Raw(), 0x10);
    auto eap = SecurityProperties({ "wpa-eap-suite-b", "wpa-psk", "sae-ext-key" });
    EXPECT_EQ(Internal::GetWiFiSecurity(nullptr, eap.get()).Raw(), 0x38);
    EXPECT_EQ(Internal::GetWiFiSecurity(psk.get(), eap.get()).Raw(), 0x3c);
}

} // namespace
#endif
