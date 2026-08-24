/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <app/server/ThreadRendezvousAnnouncement.h>
#include <lib/core/CHIPError.h>
#include <lib/dnssd/wire/Parser.h>
#include <lib/dnssd/wire/RecordData.h>
#include <lib/support/CodeUtils.h>

#include <optional>

using namespace chip;
using namespace chip::app;

class TestThreadRendezvousAnnouncement : public ::testing::Test
{
};

namespace {

class AnnouncementParser : public Dnssd::ParserDelegate
{
public:
    explicit AnnouncementParser(const Dnssd::BytesRange & packet) : mPacket(packet) {}

    void OnHeader(Dnssd::ConstHeaderRef &) override {}
    void OnQuery(const Dnssd::QueryData &) override {}

    void OnResource(Dnssd::ResourceType, const Dnssd::ResourceData & data) override
    {
        mResourceCount++;
        if (data.GetType() != Dnssd::QType::SRV)
        {
            return;
        }

        Dnssd::SrvRecord srv;
        mSrvParsed = srv.Parse(data.GetData(), mPacket);
        if (mSrvParsed)
        {
            mSrvPort = srv.GetPort();
        }
    }

    size_t GetResourceCount() const { return mResourceCount; }
    bool SrvParsed() const { return mSrvParsed; }
    uint16_t GetSrvPort() const { return mSrvPort; }

private:
    Dnssd::BytesRange mPacket;
    size_t mResourceCount = 0;
    bool mSrvParsed       = false;
    uint16_t mSrvPort     = 0;
};

} // namespace

TEST_F(TestThreadRendezvousAnnouncement, TxtStringsBuilder)
{
    Dnssd::CommissionAdvertisingParameters params;
    params.SetVendorId(std::make_optional<uint16_t>(123))
        .SetProductId(std::make_optional<uint16_t>(456))
        .SetLongDiscriminator(789)
        .SetCommissioningMode(Dnssd::CommissioningMode::kEnabledBasic)
        .SetDeviceType(std::make_optional<uint32_t>(1))
        .SetDeviceName(std::make_optional<const char *>("TestDevice"))
        .SetRotatingDeviceId(std::make_optional<const char *>("1234567890"))
        .SetPairingHint(std::make_optional<uint16_t>(2))
        .SetPairingInstruction(std::make_optional<const char *>("Press button"));

    TxtStringsBuilder builder;
    EXPECT_EQ(builder.Fill(params), CHIP_NO_ERROR);
    EXPECT_EQ(builder.GetCount(), 8u);
    const char * const * entries = builder.GetEntries();

    EXPECT_STREQ(entries[0], "VP=123+456");
    EXPECT_STREQ(entries[1], "D=789");
    EXPECT_STREQ(entries[2], "CM=1");
    EXPECT_STREQ(entries[3], "DT=1");
    EXPECT_STREQ(entries[4], "DN=TestDevice");
    EXPECT_STREQ(entries[5], "RI=1234567890");
    EXPECT_STREQ(entries[6], "PH=2");
    EXPECT_STREQ(entries[7], "PI=Press button");
}

TEST_F(TestThreadRendezvousAnnouncement, TxtStringsBuilderOverflow)
{
    Dnssd::CommissionAdvertisingParameters params;
    params.SetVendorId(std::make_optional<uint16_t>(65535))
        .SetProductId(std::make_optional<uint16_t>(65535))
        .SetLongDiscriminator(789)
        .SetCommissioningMode(Dnssd::CommissioningMode::kEnabledBasic)
        .SetDeviceType(std::make_optional<uint32_t>(0xFFFFFFFF))
        .SetPairingHint(std::make_optional<uint16_t>(0xFFFF));

    char longString[128];
    memset(longString, 'A', sizeof(longString) - 1);
    longString[sizeof(longString) - 1] = '\0';

    params.SetDeviceName(std::make_optional<const char *>(longString));         // max 32
    params.SetRotatingDeviceId(std::make_optional<const char *>(longString));   // max 100
    params.SetPairingInstruction(std::make_optional<const char *>(longString)); // max 128

    TxtStringsBuilder builder;
    CHIP_ERROR err = builder.Fill(params);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);
}

TEST_F(TestThreadRendezvousAnnouncement, BuildThreadRendezvousAnnouncement)
{
    Dnssd::CommissionAdvertisingParameters params;
    params.SetPort(5540);
    params.SetLongDiscriminator(789);

    System::PacketBufferHandle buffer;
    CHIP_ERROR err = BuildThreadRendezvousAnnouncement(params, buffer);
    ASSERT_EQ(err, CHIP_NO_ERROR);
    ASSERT_FALSE(buffer.IsNull());

    Dnssd::BytesRange packet(buffer->Start(), buffer->Start() + buffer->DataLength());
    AnnouncementParser parser(packet);
    ASSERT_TRUE(Dnssd::ParsePacket(packet, &parser));
    EXPECT_EQ(parser.GetResourceCount(), 2u);
    EXPECT_TRUE(parser.SrvParsed());
    EXPECT_EQ(parser.GetSrvPort(), 5540u);
}
