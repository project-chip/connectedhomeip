/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <controller/AutoCommissioner.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMemString.h>

#include <memory>

using namespace chip;
using namespace chip::Dnssd;
using namespace chip::Controller;

namespace {

class AutoCommissionerTest : public ::testing::Test
{
protected:
    AutoCommissioner mCommissioner{};
    CommissioningParameters mParams{};
};

TEST_F(AutoCommissionerTest, DetectsThreadOperationalDatasetExceedsBuffer)
{
    auto up = std::make_unique<uint8_t[]>(CommissioningParameters::kMaxThreadDatasetLen + 1);

    mParams.SetThreadOperationalDataset(ByteSpan{ up.get(), CommissioningParameters::kMaxThreadDatasetLen + 1 });

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    ASSERT_EQ(r, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(AutoCommissionerTest, DetectsWifiCredentialsExceedBuffer)
{
    auto ssid_buffer_up = std::make_unique<uint8_t[]>(CommissioningParameters::kMaxSsidLen + 1);

    auto creds_buffer_up = std::make_unique<uint8_t[]>(CommissioningParameters::kMaxCredentialsLen + 1);

    mParams.SetWiFiCredentials(WiFiCredentials{
        ByteSpan{ ssid_buffer_up.get(), CommissioningParameters::kMaxSsidLen + 1 },
        ByteSpan{ creds_buffer_up.get(), CommissioningParameters::kMaxCredentialsLen + 1 },
    });

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    ASSERT_EQ(r, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(AutoCommissionerTest, DetectsCountryCodeExceedsBuffer)
{
    char overflowing_src_buffer[CommissioningParameters::kMaxCountryCodeLen + 1];

    overflowing_src_buffer[0] = '0';
    overflowing_src_buffer[1] = '1';
    overflowing_src_buffer[2] = '2';

    mParams.SetCountryCode(CharSpan{ overflowing_src_buffer, 3 });

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    ASSERT_EQ(r, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(AutoCommissionerTest, DetectsAttestationNonceExceedsBuffer)
{
    auto attestation_nonce_buffer_up = std::make_unique<uint8_t[]>(kAttestationNonceLength + 1);

    mParams.SetAttestationNonce(ByteSpan{ attestation_nonce_buffer_up.get(), kAttestationNonceLength + 1 });

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    ASSERT_EQ(r, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(AutoCommissionerTest, DetectsCSRNonceExceedsBuffer)
{
    auto csr_nonce_buffer_up = std::make_unique<uint8_t[]>(kCSRNonceLength + 1);

    mParams.SetCSRNonce(ByteSpan{ csr_nonce_buffer_up.get(), kCSRNonceLength + 1 });

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    ASSERT_EQ(r, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(AutoCommissionerTest, FeaturesPassedDSTOffsetsValue)
{
    app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type sDSTBuf;

    sDSTBuf.offset        = int32_t{ 10 };
    sDSTBuf.validStarting = uint64_t{ 1742998643 };
    sDSTBuf.validUntil    = uint64_t{ 1842998643 };
    app::DataModel::List<app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type> list(&sDSTBuf, 1);

    mParams.SetDSTOffsets(list);

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    auto commissioning_params = mCommissioner.GetCommissioningParameters();

    ASSERT_EQ(r, CHIP_NO_ERROR);
    ASSERT_TRUE(commissioning_params.GetDSTOffsets().HasValue());
    ASSERT_EQ(commissioning_params.GetDSTOffsets().Value().size(), size_t{ 1 });
    ASSERT_EQ(commissioning_params.GetDSTOffsets().Value()[0].offset, 10);
    ASSERT_EQ(commissioning_params.GetDSTOffsets().Value()[0].validStarting, uint64_t{ 1742998643 });
    ASSERT_EQ(commissioning_params.GetDSTOffsets().Value()[0].validUntil, uint64_t{ 1842998643 });
}

TEST_F(AutoCommissionerTest, FeaturesPassedTimeZoneValue)
{
    app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type sTimeZoneBuf;

    char country_name[4];
    country_name[0] = 'A';
    country_name[1] = 'R';
    country_name[2] = 'G';
    country_name[3] = 0;

    sTimeZoneBuf.offset  = int32_t{ 10 };
    sTimeZoneBuf.validAt = uint64_t{ 1842998643 };
    sTimeZoneBuf.name.SetValue(chip::CharSpan{ country_name, 4 });

    app::DataModel::List<app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type> list(&sTimeZoneBuf, 1);
    mParams.SetTimeZone(list);

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    auto commissioning_params = mCommissioner.GetCommissioningParameters();

    ASSERT_EQ(r, CHIP_NO_ERROR);
    ASSERT_TRUE(commissioning_params.GetTimeZone().HasValue());
    ASSERT_EQ(commissioning_params.GetTimeZone().Value().size(), size_t{ 1 });
    ASSERT_EQ(commissioning_params.GetTimeZone().Value()[0].offset, 10);
    ASSERT_EQ(commissioning_params.GetTimeZone().Value()[0].validAt, uint64_t{ 1842998643 });
    ASSERT_TRUE(commissioning_params.GetTimeZone().Value()[0].name.HasValue());
    ASSERT_EQ(commissioning_params.GetTimeZone().Value()[0].name.Value()[0], 'A');
    ASSERT_EQ(commissioning_params.GetTimeZone().Value()[0].name.Value()[1], 'R');
    ASSERT_EQ(commissioning_params.GetTimeZone().Value()[0].name.Value()[2], 'G');
    ASSERT_EQ(commissioning_params.GetTimeZone().Value()[0].name.Value()[3], 0);
}

TEST_F(AutoCommissionerTest, FeaturesPassedNTPValue)
{
    char default_ntp_buffer[20];
    default_ntp_buffer[0] = 'd';
    default_ntp_buffer[1] = 'e';
    default_ntp_buffer[2] = 'f';
    default_ntp_buffer[3] = 'a';
    default_ntp_buffer[4] = 'u';
    default_ntp_buffer[5] = 'l';
    default_ntp_buffer[6] = 't';
    default_ntp_buffer[7] = 0;

    mParams.SetDefaultNTP(chip::app::DataModel::MakeNullable(CharSpan{ default_ntp_buffer, 20 }));

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    auto commissioning_params = mCommissioner.GetCommissioningParameters();

    ASSERT_EQ(r, CHIP_NO_ERROR);
    ASSERT_TRUE(commissioning_params.GetDefaultNTP().HasValue());
    ASSERT_EQ(commissioning_params.GetDefaultNTP().Value().Value().size(), size_t{ 20 });
    ASSERT_EQ(commissioning_params.GetDefaultNTP().Value().Value()[0], 'd');
    ASSERT_EQ(commissioning_params.GetDefaultNTP().Value().Value()[1], 'e');
    ASSERT_EQ(commissioning_params.GetDefaultNTP().Value().Value()[2], 'f');
    ASSERT_EQ(commissioning_params.GetDefaultNTP().Value().Value()[3], 'a');
    ASSERT_EQ(commissioning_params.GetDefaultNTP().Value().Value()[4], 'u');
    ASSERT_EQ(commissioning_params.GetDefaultNTP().Value().Value()[5], 'l');
    ASSERT_EQ(commissioning_params.GetDefaultNTP().Value().Value()[6], 't');
}

TEST_F(AutoCommissionerTest, FeaturesPassedICDRegistrationKey)
{
    mParams.SetICDRegistrationStrategy(ICDRegistrationStrategy::kBeforeComplete);

    uint8_t symmetric_key_buffer[Crypto::kAES_CCM128_Key_Length];

    symmetric_key_buffer[0] = 0x01;
    symmetric_key_buffer[1] = 0x02;
    symmetric_key_buffer[2] = 0x03;
    symmetric_key_buffer[3] = 0x04;
    symmetric_key_buffer[4] = 0x05;
    symmetric_key_buffer[5] = 0x10;
    symmetric_key_buffer[6] = 0x11;
    symmetric_key_buffer[7] = 0x12;
    symmetric_key_buffer[8] = 0x13;
    symmetric_key_buffer[9] = 0x14;

    mParams.SetICDSymmetricKey(ByteSpan{ symmetric_key_buffer, Crypto::kAES_CCM128_Key_Length });
    mParams.SetICDCheckInNodeId(NodeId{ 10000 });
    mParams.SetICDMonitoredSubject(uint64_t{ 9999 });
    mParams.SetICDClientType(app::Clusters::IcdManagement::ClientTypeEnum::kPermanent);

    auto r = mCommissioner.SetCommissioningParameters(mParams);

    auto commissioning_params = mCommissioner.GetCommissioningParameters();

    ASSERT_EQ(r, CHIP_NO_ERROR);
    ASSERT_TRUE(commissioning_params.GetICDSymmetricKey().HasValue());
    ASSERT_EQ(commissioning_params.GetICDRegistrationStrategy(), ICDRegistrationStrategy::kBeforeComplete);
    ASSERT_TRUE(commissioning_params.GetICDSymmetricKey().HasValue());
    ASSERT_EQ(commissioning_params.GetICDSymmetricKey().Value().size(), Crypto::kAES_CCM128_Key_Length);
    ASSERT_EQ(commissioning_params.GetICDSymmetricKey().Value()[0], 0x01);
    ASSERT_EQ(commissioning_params.GetICDSymmetricKey().Value()[1], 0x02);
    ASSERT_EQ(commissioning_params.GetICDSymmetricKey().Value()[2], 0x03);
    ASSERT_EQ(commissioning_params.GetICDSymmetricKey().Value()[3], 0x04);
    ASSERT_EQ(commissioning_params.GetICDSymmetricKey().Value()[4], 0x05);
    ASSERT_EQ(commissioning_params.GetICDSymmetricKey().Value()[5], 0x10);
    ASSERT_EQ(commissioning_params.GetICDSymmetricKey().Value()[6], 0x11);
    ASSERT_EQ(commissioning_params.GetICDSymmetricKey().Value()[7], 0x12);
    ASSERT_EQ(commissioning_params.GetICDSymmetricKey().Value()[8], 0x13);
    ASSERT_EQ(commissioning_params.GetICDSymmetricKey().Value()[9], 0x14);
    ASSERT_TRUE(commissioning_params.GetICDCheckInNodeId().HasValue());
    ASSERT_EQ(commissioning_params.GetICDCheckInNodeId().Value(), NodeId{ 10000 });
    ASSERT_TRUE(commissioning_params.GetICDMonitoredSubject().HasValue());
    ASSERT_EQ(commissioning_params.GetICDMonitoredSubject().Value(), uint64_t{ 9999 });
    ASSERT_TRUE(commissioning_params.GetICDClientType().HasValue());
    ASSERT_EQ(commissioning_params.GetICDClientType().Value(), app::Clusters::IcdManagement::ClientTypeEnum::kPermanent);
}

TEST_F(AutoCommissionerTest, FeaturesPassedExtraReadPaths)
{
    chip::app::AttributePathParams attributes[1];

    attributes[0] = chip::app::AttributePathParams{ EndpointId{ 1 }, ClusterId{ 2 }, AttributeId{ 3 } };

    mParams.SetExtraReadPaths(Span<const app::AttributePathParams>{ attributes, 1 });

    auto params = mParams.GetExtraReadPaths();

    ASSERT_EQ(params.size(), size_t{ 1 });
}

} // namespace
