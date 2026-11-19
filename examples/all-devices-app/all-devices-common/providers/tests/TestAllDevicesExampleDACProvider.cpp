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

#include <credentials/CHIPCert.h>
#include <lib/support/CHIPMem.h>
#include <providers/AllDevicesExampleDACProvider.h>
#include <pw_unit_test/framework.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <unistd.h>

namespace {
using namespace chip;
using namespace chip::Credentials;

class TestAllDevicesExampleDACProvider : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    void SetUp() override
    {
        const int fd = mkstemp(mPath);
        ASSERT_GE(fd, 0);
        close(fd);
        std::ofstream file(mPath);
        file << R"({"dac_cert":"010203", "pai_cert":"040506",
                    "dac_cert_ml_dsa_44":"111213", "pai_cert_ml_dsa_44":"212223",
                    "dac_cert_ml_dsa_65":"313233", "pai_cert_ml_dsa_65":"414243"})";
        ASSERT_TRUE(file.good());
    }
    void TearDown() override { std::remove(mPath); }
    char mPath[64] = "/tmp/all-devices-dac-XXXXXX";
};

TEST_F(TestAllDevicesExampleDACProvider, ReportsProfilesAndRetrievesSelectedCertificateSegments)
{
    DeviceLayer::AllDevicesExampleDACProvider provider;
    ASSERT_EQ(provider.Init(std::string(mPath)), CHIP_NO_ERROR);
    EXPECT_TRUE(provider.HasRequiredPqcCredentials());
    EXPECT_EQ(provider.GetPreferredDeviceAttestationChainProfile(), DeviceAttestationCertProfile::kMlDsa65);
    const auto profiles = provider.GetDeviceAttestationProfileSupport();
    EXPECT_EQ(profiles.PAASupportedProfiles.Raw(), 7u);
    EXPECT_EQ(profiles.PAISupportedProfiles.Raw(), 7u);
    EXPECT_EQ(profiles.DACSupportedProfiles.Raw(), 1u);

    for (auto profile : { DeviceAttestationCertProfile::kMlDsa44, DeviceAttestationCertProfile::kMlDsa65 })
    {
        for (auto type : { DeviceAttestationDocumentType::kDACCertificate, DeviceAttestationDocumentType::kPAICertificate })
        {
            uint8_t bytes[3];
            MutableByteSpan certificate(bytes);
            const bool isDac = type == DeviceAttestationDocumentType::kDACCertificate;
            ASSERT_EQ(isDac ? provider.GetDeviceAttestationCertForProfile(profile, certificate)
                            : provider.GetProductAttestationIntermediateCertForProfile(profile, certificate),
                      CHIP_NO_ERROR);
            const uint8_t first = profile == DeviceAttestationCertProfile::kMlDsa44 ? (isDac ? 0x11 : 0x21) : (isDac ? 0x31 : 0x41);
            ASSERT_EQ(certificate.size(), 3u);
            EXPECT_EQ(certificate[0], first);
            uint8_t segmentByte;
            MutableByteSpan segment(&segmentByte, 1);
            size_t size = 0;
            ASSERT_EQ(provider.GetDeviceAttestationDocumentSegment(type, profile, 1, segment, size), CHIP_NO_ERROR);
            EXPECT_EQ(size, 3u);
            EXPECT_EQ(segmentByte, first + 1);
            const DeviceAttestationChainProfile chain{ profile, profile, DeviceAttestationCertProfile::kEcdsaMatterLegacy };
            ASSERT_EQ(provider.GetDeviceAttestationDocumentSegmentForChain(type, chain, 2, segment, size), CHIP_NO_ERROR);
            EXPECT_EQ(size, 3u);
            EXPECT_EQ(segmentByte, first + 2);
        }
    }

    ASSERT_EQ(provider.Init(), CHIP_NO_ERROR);
    EXPECT_FALSE(provider.HasRequiredPqcCredentials());
    EXPECT_EQ(provider.GetPreferredDeviceAttestationChainProfile(), DeviceAttestationCertProfile::kEcdsaMatterLegacy);
    uint8_t bytes[kMaxDERCertLength];
    MutableByteSpan certificate(bytes);
    EXPECT_EQ(provider.GetDeviceAttestationCertForProfile(DeviceAttestationCertProfile::kEcdsaMatterLegacy, certificate),
              CHIP_NO_ERROR);
    EXPECT_GT(certificate.size(), 0u);
}

TEST_F(TestAllDevicesExampleDACProvider, UninitializedProviderRejectsCertificateRequests)
{
    DeviceLayer::AllDevicesExampleDACProvider provider;
    uint8_t bytes[16];
    MutableByteSpan certificate(bytes);
    size_t size = 0;
    const DeviceAttestationChainProfile chain{};
    EXPECT_FALSE(provider.HasRequiredPqcCredentials());
    EXPECT_EQ(provider.GetDeviceAttestationCertForProfile(DeviceAttestationCertProfile::kMlDsa44, certificate),
              CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(provider.GetProductAttestationIntermediateCertForProfile(DeviceAttestationCertProfile::kMlDsa44, certificate),
              CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(provider.GetDeviceAttestationDocumentSegment(DeviceAttestationDocumentType::kDACCertificate,
                                                           DeviceAttestationCertProfile::kMlDsa44, 0, certificate, size),
              CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(provider.GetDeviceAttestationDocumentSegmentForChain(DeviceAttestationDocumentType::kPAICertificate, chain, 0,
                                                                   certificate, size),
              CHIP_ERROR_INCORRECT_STATE);
}
} // namespace
