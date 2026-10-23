/*
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

#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/network-identity-management-server/NetworkAdministratorSecret.h>
#include <app/clusters/network-identity-management-server/tests/NASS_test_vectors.h>
#include <lib/core/TLV.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPMem.h>

using namespace chip;
using namespace chip::Crypto;

struct TestNetworkAdministratorSecret : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_SUCCESS(chip::Platform::MemoryInit()); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestNetworkAdministratorSecret, DecodeSpecVector)
{
    // Base64-decode the PEM content to get the original TLV bytes.
    uint8_t originalTlv[kNetworkAdministratorSecretDataMaxEncodedLength];
    uint16_t originalLen = Base64Decode(kNASSTestVector1_PEM, static_cast<uint16_t>(strlen(kNASSTestVector1_PEM)), originalTlv);
    ASSERT_NE(originalLen, UINT16_MAX);

    // Decode, then re-encode, and verify the output matches byte-for-byte.
    NetworkAdministratorSecretData secretData;
    ASSERT_SUCCESS(DecodeNetworkAdministratorSecret(ByteSpan(originalTlv, originalLen), secretData));

    uint8_t reEncodedBuf[kNetworkAdministratorSecretDataMaxEncodedLength];
    MutableByteSpan reEncoded(reEncodedBuf);
    ASSERT_SUCCESS(EncodeNetworkAdministratorSecret(secretData, reEncoded));

    EXPECT_TRUE(reEncoded.data_equal(ByteSpan(originalTlv, originalLen)));
}

// Helper: encode a NASS TLV with the given field values.
// Pass nullopt for rawSecret to omit the field entirely.
static CHIP_ERROR EncodeTestNASSTlv(uint8_t version, uint32_t created, std::optional<ByteSpan> rawSecret,
                                    MutableByteSpan & outEncoded)
{
    using namespace TLV;
    TLVWriter writer;
    TLVType outer;

    writer.Init(outEncoded);
    ReturnErrorOnFailure(writer.StartContainer(AnonymousTag(), kTLVType_Structure, outer));
    ReturnErrorOnFailure(writer.Put(ContextTag(1), version));
    ReturnErrorOnFailure(writer.Put(ContextTag(2), created));
    if (rawSecret.has_value())
    {
        ReturnErrorOnFailure(writer.Put(ContextTag(3), rawSecret.value()));
    }
    ReturnErrorOnFailure(writer.EndContainer(outer));
    ReturnErrorOnFailure(writer.Finalize());
    outEncoded.reduce_size(writer.GetLengthWritten());
    return CHIP_NO_ERROR;
}

TEST_F(TestNetworkAdministratorSecret, DecodeRejectsWrongVersion)
{
    uint8_t secret[kNetworkAdministratorRawSecretLength] = {};
    uint8_t buf[kNetworkAdministratorSecretDataMaxEncodedLength];
    MutableByteSpan encoded(buf);
    ASSERT_SUCCESS(EncodeTestNASSTlv(1, 1000, ByteSpan(secret), encoded));

    NetworkAdministratorSecretData secretData;
    EXPECT_NE(DecodeNetworkAdministratorSecret(encoded, secretData), CHIP_NO_ERROR);
}

TEST_F(TestNetworkAdministratorSecret, DecodeRejectsMissingRawSecret)
{
    uint8_t buf[kNetworkAdministratorSecretDataMaxEncodedLength];
    MutableByteSpan encoded(buf);
    ASSERT_SUCCESS(EncodeTestNASSTlv(0, 1000, std::nullopt, encoded));

    NetworkAdministratorSecretData secretData;
    EXPECT_NE(DecodeNetworkAdministratorSecret(encoded, secretData), CHIP_NO_ERROR);
}

TEST_F(TestNetworkAdministratorSecret, DecodeRejectsWrongSecretLength)
{
    uint8_t shortSecret[16] = {};
    uint8_t buf[kNetworkAdministratorSecretDataMaxEncodedLength];
    MutableByteSpan encoded(buf);
    ASSERT_SUCCESS(EncodeTestNASSTlv(0, 1000, ByteSpan(shortSecret), encoded));

    NetworkAdministratorSecretData secretData;
    EXPECT_NE(DecodeNetworkAdministratorSecret(encoded, secretData), CHIP_NO_ERROR);
}

TEST_F(TestNetworkAdministratorSecret, DecodeRejectsUnknownTrailingField)
{
    // Manually encode a valid NASS with an extra field (tag 4) appended.
    using namespace TLV;
    uint8_t secret[kNetworkAdministratorRawSecretLength] = {};
    uint8_t buf[kNetworkAdministratorSecretDataMaxEncodedLength + 20];
    TLVWriter writer;
    TLVType outer;

    writer.Init(buf);
    ASSERT_SUCCESS(writer.StartContainer(AnonymousTag(), kTLVType_Structure, outer));
    ASSERT_SUCCESS(writer.Put(ContextTag(1), static_cast<uint8_t>(0)));
    ASSERT_SUCCESS(writer.Put(ContextTag(2), static_cast<uint32_t>(1000)));
    ASSERT_SUCCESS(writer.Put(ContextTag(3), ByteSpan(secret)));
    ASSERT_SUCCESS(writer.Put(ContextTag(4), static_cast<uint8_t>(0xFF))); // unknown field
    ASSERT_SUCCESS(writer.EndContainer(outer));
    ASSERT_SUCCESS(writer.Finalize());

    ByteSpan encoded(buf, writer.GetLengthWritten());
    NetworkAdministratorSecretData secretData;
    EXPECT_NE(DecodeNetworkAdministratorSecret(encoded, secretData), CHIP_NO_ERROR);
}
