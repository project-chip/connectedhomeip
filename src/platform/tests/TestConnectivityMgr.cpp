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
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <clusters/NetworkCommissioning/Structs.h>
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
namespace chip {
namespace DeviceLayer {
namespace Internal {

class TestWpaSupplicantClient : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mBss.reset(WPA_SUPPLICANT_1_BSS(g_object_new(wpa_supplicant_1_bss_proxy_get_type(), "g-interface-name",
                                                     "fi.w1.wpa_supplicant1.BSS", "g-object-path",
                                                     "/fi/w1/wpa_supplicant1/Interfaces/0/BSSs/0", nullptr)));
        ASSERT_NE(mBss.get(), nullptr);
        ASSERT_EQ(g_dbus_proxy_get_connection(G_DBUS_PROXY(mBss.get())), nullptr);
        SetProperty("SSID", g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, kSsid, sizeof(kSsid), sizeof(uint8_t)));
        SetProperty("BSSID", g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, kBssid, sizeof(kBssid), sizeof(uint8_t)));
        SetProperty("Frequency", g_variant_new_uint16(2412));
        SetProperty("Signal", g_variant_new_int16(-47));
    }

    void SetProperty(const char * name, GVariant * value)
    {
        g_dbus_proxy_set_cached_property(G_DBUS_PROXY(mBss.get()), name, value);
    }

    void SetKeyMgmt(const char * property, std::initializer_list<const gchar *> values)
    {
        GVariantBuilder dictionary;
        g_variant_builder_init(&dictionary, G_VARIANT_TYPE_VARDICT);
        g_variant_builder_add(&dictionary, "{sv}", "KeyMgmt",
                              g_variant_new_strv(values.begin(), static_cast<gssize>(values.size())));
        SetProperty(property, g_variant_builder_end(&dictionary));
    }

    void ExpectSecurity(uint8_t expected)
    {
        NetworkCommissioning::WiFiScanResponse response{};
        ASSERT_TRUE(mClient.GetBssInfo(mBss.get(), response));
        EXPECT_EQ(response.security.Raw(), expected);
        EXPECT_EQ(response.security.Raw() & 0x80, 0);
    }

    void ExpectTlvSecurity(uint8_t expected)
    {
        NetworkCommissioning::WiFiScanResponse response{};
        ASSERT_TRUE(mClient.GetBssInfo(mBss.get(), response));
        app::Clusters::NetworkCommissioning::Structs::WiFiInterfaceScanResultStruct::Type result;
        result.security = response.security;
        result.ssid     = ByteSpan(response.ssid, response.ssidLen);
        result.bssid    = ByteSpan(response.bssid);
        result.channel  = response.channel;
        result.wiFiBand = response.wiFiBand;
        result.rssi     = response.signal.strength;

        uint8_t buffer[128];
        TLV::TLVWriter writer;
        writer.Init(buffer);
        ASSERT_EQ(app::DataModel::Encode(writer, TLV::AnonymousTag(), result), CHIP_NO_ERROR);
        ASSERT_EQ(writer.Finalize(), CHIP_NO_ERROR);

        TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
        TLV::TLVType container;
        ASSERT_EQ(reader.EnterContainer(container), CHIP_NO_ERROR);
        ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
        ASSERT_EQ(reader.GetTag(), TLV::ContextTag(0));
        uint8_t security = 0;
        ASSERT_EQ(reader.Get(security), CHIP_NO_ERROR);
        EXPECT_EQ(security, expected);
        EXPECT_EQ(security & 0x80, 0);

        reader.Init(buffer, writer.GetLengthWritten());
        ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
        app::Clusters::NetworkCommissioning::Structs::WiFiInterfaceScanResultStruct::DecodableType decoded;
        ASSERT_EQ(app::DataModel::Decode(reader, decoded), CHIP_NO_ERROR);
        EXPECT_EQ(decoded.security.Raw(), expected);
        EXPECT_TRUE(decoded.ssid.data_equal(ByteSpan(kSsid)));
        EXPECT_TRUE(decoded.bssid.data_equal(ByteSpan(kBssid)));
        EXPECT_EQ(decoded.channel, 1);
        EXPECT_EQ(decoded.wiFiBand, app::Clusters::NetworkCommissioning::WiFiBandEnum::k2g4);
        EXPECT_EQ(decoded.rssi, -47);
    }

    static constexpr uint8_t kSsid[]  = { 's', 'c', 'a', 'n', 0, 'x' };
    static constexpr uint8_t kBssid[] = { 0x02, 0x11, 0x22, 0x33, 0x44, 0x55 };
    WpaSupplicantClient mClient;
    GAutoPtr<WpaSupplicant1BSS> mBss;
};

TEST_F(TestWpaSupplicantClient, Open)
{
    ExpectSecurity(0x01);
}

TEST_F(TestWpaSupplicantClient, Personal)
{
    SetKeyMgmt("WPA", { "wpa-psk" });
    ExpectSecurity(0x04);
    SetKeyMgmt("WPA", { "wpa-none" });
    ExpectSecurity(0x04);
    SetKeyMgmt("RSN", { "wpa-psk", "wpa-psk-sha256", "wpa-ft-psk", "sae" });
    ExpectSecurity(0x1c);
}

TEST_F(TestWpaSupplicantClient, WpaEap)
{
    SetKeyMgmt("WPA", { "wpa-eap" });
    ExpectSecurity(0x20);
}

TEST_F(TestWpaSupplicantClient, RsnEap)
{
    for (const auto * keyMgmt : { "wpa-eap", "wpa-eap-sha256", "wpa-ft-eap", "WPA-EAP" })
    {
        SetKeyMgmt("RSN", { keyMgmt });
        ExpectSecurity(0x20);
    }
}

TEST_F(TestWpaSupplicantClient, MixedSecurity)
{
    SetKeyMgmt("WPA", { "wpa-eap", "wpa-psk" });
    ExpectSecurity(0x24);
    SetKeyMgmt("RSN", { "wpa-eap", "wpa-psk", "sae" });
    ExpectSecurity(0x3c);
}

TEST_F(TestWpaSupplicantClient, EapTlvRoundTrip)
{
    SetKeyMgmt("RSN", { "wpa-eap" });
    ExpectTlvSecurity(0x20);
}

TEST_F(TestWpaSupplicantClient, MixedTlvRoundTrip)
{
    SetKeyMgmt("WPA", { "wpa-psk" });
    SetKeyMgmt("RSN", { "wpa-eap", "wpa-psk", "sae" });
    ExpectTlvSecurity(0x3c);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
#endif
