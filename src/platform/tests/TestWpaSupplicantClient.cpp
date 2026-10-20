#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <clusters/NetworkCommissioning/Structs.h>
#include <platform/Linux/WpaSupplicantClient.h>

#include <cstring>
#include <initializer_list>

#include <pw_unit_test/framework.h>

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

    bool Decode() { return mClient.GetBssInfo(mBss.get(), mResponse); }

    void ExpectSecurity(uint8_t expected)
    {
        ASSERT_TRUE(Decode());
        EXPECT_EQ(mResponse.security.Raw(), expected);
        EXPECT_EQ(mResponse.security.Raw() & 0x80, 0);
    }

    void ExpectScanResultTlvSecurity(uint8_t expected)
    {
        ASSERT_TRUE(Decode());
        app::Clusters::NetworkCommissioning::Structs::WiFiInterfaceScanResultStruct::Type result;
        result.security = mResponse.security;
        result.ssid     = ByteSpan(mResponse.ssid, mResponse.ssidLen);
        result.bssid    = ByteSpan(mResponse.bssid);
        result.channel  = mResponse.channel;
        result.wiFiBand = mResponse.wiFiBand;
        result.rssi     = mResponse.signal.strength;

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
        EXPECT_EQ(reader.GetTag(), TLV::ContextTag(0));
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
        EXPECT_EQ(decoded.channel, mResponse.channel);
        EXPECT_EQ(decoded.wiFiBand, mResponse.wiFiBand);
        EXPECT_EQ(decoded.rssi, mResponse.signal.strength);
    }

    static constexpr uint8_t kSsid[]  = { 's', 'c', 'a', 'n', 0, 'x' };
    static constexpr uint8_t kBssid[] = { 0x02, 0x11, 0x22, 0x33, 0x44, 0x55 };
    WpaSupplicantClient mClient;
    GAutoPtr<WpaSupplicant1BSS> mBss;
    NetworkCommissioning::WiFiScanResponse mResponse{};
};

TEST_F(TestWpaSupplicantClient, OpenWithoutSecurityProperties)
{
    ExpectSecurity(0x01);
}

TEST_F(TestWpaSupplicantClient, EmptyKeyMgmt)
{
    SetKeyMgmt("WPA", {});
    SetKeyMgmt("RSN", {});
    ExpectSecurity(0x01);
}

TEST_F(TestWpaSupplicantClient, MissingKeyMgmt)
{
    SetProperty("WPA", g_variant_new_array(G_VARIANT_TYPE("{sv}"), nullptr, 0));
    SetProperty("RSN", g_variant_new_array(G_VARIANT_TYPE("{sv}"), nullptr, 0));
    ExpectSecurity(0x01);
}

TEST_F(TestWpaSupplicantClient, UnknownKeyMgmt)
{
    SetKeyMgmt("WPA", { "unknown" });
    SetKeyMgmt("RSN", { "unknown", "" });
    ExpectSecurity(0x01);
}

TEST_F(TestWpaSupplicantClient, WpaEap)
{
    SetKeyMgmt("WPA", { "wpa-eap" });
    ExpectSecurity(0x20);
}

TEST_F(TestWpaSupplicantClient, RsnEap)
{
    SetKeyMgmt("RSN", { "wpa-eap" });
    ExpectSecurity(0x20);
}

TEST_F(TestWpaSupplicantClient, RsnEapSha256)
{
    SetKeyMgmt("RSN", { "wpa-eap-sha256" });
    ExpectSecurity(0x20);
}

TEST_F(TestWpaSupplicantClient, RsnFtEap)
{
    SetKeyMgmt("RSN", { "wpa-ft-eap" });
    ExpectSecurity(0x20);
}

TEST_F(TestWpaSupplicantClient, CaseInsensitiveEap)
{
    SetKeyMgmt("WPA", { "WPA-EAP" });
    SetKeyMgmt("RSN", { "WpA-EaP-ShA256", "WPA-FT-EAP" });
    ExpectSecurity(0x20);
}

TEST_F(TestWpaSupplicantClient, WpaPsk)
{
    SetKeyMgmt("WPA", { "wpa-psk" });
    ExpectSecurity(0x04);
}

TEST_F(TestWpaSupplicantClient, WpaNone)
{
    SetKeyMgmt("WPA", { "wpa-none" });
    ExpectSecurity(0x04);
}

TEST_F(TestWpaSupplicantClient, RsnPsk)
{
    SetKeyMgmt("RSN", { "wpa-psk" });
    ExpectSecurity(0x08);
}

TEST_F(TestWpaSupplicantClient, RsnPskSha256)
{
    SetKeyMgmt("RSN", { "wpa-psk-sha256" });
    ExpectSecurity(0x08);
}

TEST_F(TestWpaSupplicantClient, RsnFtPsk)
{
    SetKeyMgmt("RSN", { "wpa-ft-psk" });
    ExpectSecurity(0x08);
}

TEST_F(TestWpaSupplicantClient, RsnSae)
{
    SetKeyMgmt("RSN", { "sae" });
    ExpectSecurity(0x10);
}

TEST_F(TestWpaSupplicantClient, MixedWpa)
{
    SetKeyMgmt("WPA", { "wpa-eap", "wpa-psk" });
    ExpectSecurity(0x24);
}

TEST_F(TestWpaSupplicantClient, MixedRsn)
{
    SetKeyMgmt("RSN", { "wpa-eap", "wpa-psk", "sae" });
    ExpectSecurity(0x38);
}

TEST_F(TestWpaSupplicantClient, MixedWpaRsn)
{
    SetKeyMgmt("WPA", { "wpa-psk" });
    SetKeyMgmt("RSN", { "wpa-eap", "wpa-psk", "sae" });
    ExpectSecurity(0x3c);
}

TEST_F(TestWpaSupplicantClient, MixedWpaEapRsnPersonal)
{
    SetKeyMgmt("WPA", { "wpa-eap" });
    SetKeyMgmt("RSN", { "wpa-psk", "sae" });
    ExpectSecurity(0x38);
}

TEST_F(TestWpaSupplicantClient, EapScanResultTlvRoundTrip)
{
    SetKeyMgmt("RSN", { "wpa-eap" });
    ExpectScanResultTlvSecurity(0x20);
}

TEST_F(TestWpaSupplicantClient, MixedSecurityScanResultTlvRoundTrip)
{
    SetKeyMgmt("WPA", { "wpa-psk" });
    SetKeyMgmt("RSN", { "wpa-eap", "wpa-psk", "sae" });
    ExpectScanResultTlvSecurity(0x3c);
}

TEST_F(TestWpaSupplicantClient, ScanFields)
{
    ASSERT_TRUE(Decode());
    EXPECT_EQ(mResponse.ssidLen, sizeof(kSsid));
    EXPECT_EQ(std::memcmp(mResponse.ssid, kSsid, sizeof(kSsid)), 0);
    EXPECT_EQ(std::memcmp(mResponse.bssid, kBssid, sizeof(kBssid)), 0);
    EXPECT_EQ(mResponse.channel, 1);
    EXPECT_EQ(mResponse.wiFiBand, app::Clusters::NetworkCommissioning::WiFiBandEnum::k2g4);
    EXPECT_EQ(mResponse.signal.type, NetworkCommissioning::WirelessSignalType::kdBm);
    EXPECT_EQ(mResponse.signal.strength, -47);
}

TEST_F(TestWpaSupplicantClient, FiveGhzFrequency)
{
    SetProperty("Frequency", g_variant_new_uint16(5180));
    ASSERT_TRUE(Decode());
    EXPECT_EQ(mResponse.channel, 36);
    EXPECT_EQ(mResponse.wiFiBand, app::Clusters::NetworkCommissioning::WiFiBandEnum::k5g);
}

TEST_F(TestWpaSupplicantClient, SignalClamping)
{
    SetProperty("Signal", g_variant_new_int16(-200));
    ASSERT_TRUE(Decode());
    EXPECT_EQ(mResponse.signal.strength, -128);
    SetProperty("Signal", g_variant_new_int16(200));
    ASSERT_TRUE(Decode());
    EXPECT_EQ(mResponse.signal.strength, 127);
}

TEST_F(TestWpaSupplicantClient, MissingSsid)
{
    SetProperty("SSID", nullptr);
    EXPECT_FALSE(Decode());
}

TEST_F(TestWpaSupplicantClient, MissingBssid)
{
    SetProperty("BSSID", nullptr);
    EXPECT_FALSE(Decode());
}

TEST_F(TestWpaSupplicantClient, OversizedSsid)
{
    uint8_t ssid[kMaxWiFiSSIDLength + 1]{};
    SetProperty("SSID", g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, ssid, sizeof(ssid), sizeof(uint8_t)));
    EXPECT_FALSE(Decode());
}

TEST_F(TestWpaSupplicantClient, InvalidBssidLength)
{
    SetProperty("BSSID", g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, kBssid, sizeof(kBssid) - 1, sizeof(uint8_t)));
    EXPECT_FALSE(Decode());
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
