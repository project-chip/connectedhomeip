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

/**
 *    @file
 *      This file implements unit tests for the CryptoContext implementation.
 */

#include <errno.h>
#include <stdarg.h>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <protocols/secure_channel/PairingSession.h>
#include <system/SystemClock.h>
#include <system/TLVPacketBufferBackingStore.h>

using namespace chip;
using namespace chip::System::Clock;
using namespace TLV;

class TestPairingSession : public PairingSession, public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        CHIP_ERROR error = chip::Platform::MemoryInit();
        ASSERT_EQ(error, CHIP_NO_ERROR);
    }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    Transport::SecureSession::Type GetSecureSessionType() const override { return Transport::SecureSession::Type::kPASE; }
    ScopedNodeId GetPeer() const override { return ScopedNodeId(); }
    ScopedNodeId GetLocalScopedNodeId() const override { return ScopedNodeId(); }
    CATValues GetPeerCATs() const override { return CATValues(); };

    void OnSessionReleased() override {}

    const ReliableMessageProtocolConfig & GetRemoteMRPConfig() const { return PairingSession::GetRemoteMRPConfig(); }

    CHIP_ERROR DeriveSecureSession(CryptoContext & session) override { return CHIP_NO_ERROR; }

    using PairingSession::DecodeSessionParametersIfPresent;
};

TEST_F(TestPairingSession, PairingSessionEncodeDecodeMRPParams)
{
    ReliableMessageProtocolConfig mrpConfig(Milliseconds32(100), Milliseconds32(200), Milliseconds16(4000));

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(64, 0);
    System::PacketBufferTLVWriter writer;
    writer.Init(buf.Retain());

    TLVType outerContainerType = kTLVType_NotSpecified;
    EXPECT_EQ(writer.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainerType), CHIP_NO_ERROR);

    EXPECT_EQ(PairingSession::EncodeSessionParameters(ContextTag(1), mrpConfig, writer), CHIP_NO_ERROR);

    EXPECT_EQ(writer.EndContainer(outerContainerType), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(&buf), CHIP_NO_ERROR);

    System::PacketBufferTLVReader reader;
    TLVType containerType = kTLVType_Structure;

    reader.Init(std::move(buf));
    EXPECT_EQ(reader.Next(containerType, AnonymousTag()), CHIP_NO_ERROR);
    EXPECT_EQ(reader.EnterContainer(containerType), CHIP_NO_ERROR);

    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(DecodeSessionParametersIfPresent(ContextTag(1), reader, mRemoteSessionParams), CHIP_NO_ERROR);

    EXPECT_EQ(GetRemoteMRPConfig(), mrpConfig);
}

TEST_F(TestPairingSession, PairingSessionTryDecodeMissingMRPParams)
{
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(64, 0);
    System::PacketBufferTLVWriter writer;
    writer.Init(buf.Retain());

    TLVType outerContainerType = kTLVType_NotSpecified;
    EXPECT_EQ(writer.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainerType), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(ContextTag(1), static_cast<uint16_t>(0x1234)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outerContainerType), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(&buf), CHIP_NO_ERROR);

    System::PacketBufferTLVReader reader;
    TLVType containerType = kTLVType_Structure;

    reader.Init(std::move(buf));
    EXPECT_EQ(reader.Next(containerType, AnonymousTag()), CHIP_NO_ERROR);
    EXPECT_EQ(reader.EnterContainer(containerType), CHIP_NO_ERROR);
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(DecodeSessionParametersIfPresent(ContextTag(2), reader, mRemoteSessionParams), CHIP_NO_ERROR);

    EXPECT_EQ(GetRemoteMRPConfig(), GetDefaultMRPConfig());
}

struct DecodeSessionParamsTestCase
{
    char testCaseName[100];
    ReliableMessageProtocolConfig mrpConfig;
    bool includeVersionInfo;
    uint16_t dataModelRev;
    uint16_t interactionModelRev;
    uint32_t specVersion;
    uint16_t maxPaths;
    CHIP_ERROR expectedErrorDecodeParams; // Expected Error from DecodeSessionParameters
    CHIP_ERROR expectedErrorOuterExitContainer = CHIP_NO_ERROR;
    bool testFutureProofing                    = false;
    bool includeEndContainer                   = true;
};

void EncodeSessionParamsHelper(const DecodeSessionParamsTestCase & testCase, TLV::TLVWriter & writer)
{

    TLVType sessionParamsContainer = kTLVType_NotSpecified;
    EXPECT_EQ(writer.StartContainer(ContextTag(1), kTLVType_Structure, sessionParamsContainer), CHIP_NO_ERROR);

    writer.Put(ContextTag(SessionParameters::Tag::kSessionIdleInterval), testCase.mrpConfig.mIdleRetransTimeout.count());
    writer.Put(ContextTag(SessionParameters::Tag::kSessionActiveInterval), testCase.mrpConfig.mActiveRetransTimeout.count());
    writer.Put(ContextTag(SessionParameters::Tag::kSessionActiveThreshold), testCase.mrpConfig.mActiveThresholdTime.count());

    if (testCase.includeVersionInfo)
    {
        writer.Put(ContextTag(SessionParameters::Tag::kDataModelRevision), testCase.dataModelRev);
        writer.Put(ContextTag(SessionParameters::Tag::kInteractionModelRevision), testCase.interactionModelRev);
        writer.Put(ContextTag(SessionParameters::Tag::kSpecificationVersion), testCase.specVersion);
        writer.Put(ContextTag(SessionParameters::Tag::kMaxPathsPerInvoke), testCase.maxPaths);
    }

    if (testCase.testFutureProofing)
    {
        uint32_t futureProofTlvElement = 0x777666;
        writer.Put(ContextTag(8), futureProofTlvElement);
    }

    if (testCase.includeEndContainer)
    {
        EXPECT_EQ(writer.EndContainer(sessionParamsContainer), CHIP_NO_ERROR);
    }
}

bool VerifyDecodedParameters(const DecodeSessionParamsTestCase & testCase, const SessionParameters & params)
{
    if (testCase.mrpConfig.mIdleRetransTimeout.count() > 0)
    {
        if ((params.GetMRPConfig().mIdleRetransTimeout.count()) != testCase.mrpConfig.mIdleRetransTimeout.count())
        {
            printf("Idle retrans timeout mismatch: expected %" PRIu32 "ms, got %" PRIu32 "ms\n",
                   testCase.mrpConfig.mIdleRetransTimeout.count(), params.GetMRPConfig().mIdleRetransTimeout.count());
            return false;
        }
    }

    if (testCase.mrpConfig.mActiveRetransTimeout.count() > 0)
    {
        if (params.GetMRPConfig().mActiveRetransTimeout.count() != testCase.mrpConfig.mActiveRetransTimeout.count())
        {
            printf("Active retrans timeout mismatch: expected %" PRIu32 "ms, got %" PRIu32 "ms\n",
                   testCase.mrpConfig.mActiveRetransTimeout.count(), params.GetMRPConfig().mActiveRetransTimeout.count());
            return false;
        }
    }

    if (testCase.mrpConfig.mActiveThresholdTime.count() > 0)
    {
        if (params.GetMRPConfig().mActiveThresholdTime.count() != testCase.mrpConfig.mActiveThresholdTime.count())
        {
            printf("Active threshold time mismatch: expected %u, got %u\n", testCase.mrpConfig.mActiveThresholdTime.count(),
                   params.GetMRPConfig().mActiveThresholdTime.count());
            return false;
        }
    }

    if (testCase.includeVersionInfo)
    {
        if (params.GetDataModelRevision() != testCase.dataModelRev)
        {
            printf("Data model revision mismatch: expected %u, got %u\n", testCase.dataModelRev,
                   params.GetDataModelRevision().Value());
            return false;
        }
        if (params.GetInteractionModelRevision() != testCase.interactionModelRev)
        {
            printf("Interaction model revision mismatch: expected %u, got %u\n", testCase.interactionModelRev,
                   params.GetInteractionModelRevision().Value());
            return false;
        }
        if (params.GetSpecificationVersion() != testCase.specVersion)
        {
            printf("Specification version mismatch: expected %" PRIu32 ", got %" PRIu32 "\n", testCase.specVersion,
                   params.GetSpecificationVersion().Value());
            return false;
        }
        if (params.GetMaxPathsPerInvoke() != testCase.maxPaths)
        {
            printf("Max paths mismatch: expected %u, got %u\n", testCase.maxPaths, params.GetMaxPathsPerInvoke());
            return false;
        }
    }

    return true;
}

TEST_F(TestPairingSession, TestDecodeSessionParameters)
{
    const DecodeSessionParamsTestCase testCases[] = {
        { .testCaseName       = "1. Empty parameters",
          .mrpConfig          = ReliableMessageProtocolConfig(System::Clock::Milliseconds32(0), System::Clock::Milliseconds32(0),
                                                              System::Clock::Milliseconds16(0)),
          .includeVersionInfo = false,
          .expectedErrorDecodeParams = CHIP_NO_ERROR },

        { .testCaseName = "2. Only MRP parameters",
          .mrpConfig    = ReliableMessageProtocolConfig(System::Clock::Milliseconds32(1200), System::Clock::Milliseconds32(2200),
                                                        System::Clock::Milliseconds16(450)),
          .includeVersionInfo        = false,
          .expectedErrorDecodeParams = CHIP_NO_ERROR },

        { .testCaseName = "3. All parameters",
          .mrpConfig    = ReliableMessageProtocolConfig(System::Clock::Milliseconds32(1600), System::Clock::Milliseconds32(2100),
                                                        System::Clock::Milliseconds16(1300)),
          .includeVersionInfo        = true,
          .dataModelRev              = 1,
          .interactionModelRev       = 2,
          .specVersion               = 1,
          .maxPaths                  = 10,
          .expectedErrorDecodeParams = CHIP_NO_ERROR },

        { .testCaseName = "4. Maximum Value of All parameters",
          .mrpConfig =
              ReliableMessageProtocolConfig(System::Clock::Milliseconds32(UINT32_MAX), // Test max value
                                            System::Clock::Milliseconds32(UINT32_MAX), System::Clock::Milliseconds16(UINT16_MAX)),
          .includeVersionInfo        = true,
          .dataModelRev              = UINT16_MAX,
          .interactionModelRev       = UINT16_MAX,
          .specVersion               = UINT32_MAX,
          .maxPaths                  = UINT16_MAX,
          .expectedErrorDecodeParams = CHIP_NO_ERROR },

        // Future-proofing: Ensure that TLV elements being added to the specification in the future do not trigger an error in
        // DecodeSessionParametersIfPresent.
        { .testCaseName = "5. All parameters AND Test for Future Proofing",
          .mrpConfig    = ReliableMessageProtocolConfig(System::Clock::Milliseconds32(1600), System::Clock::Milliseconds32(2100),
                                                        System::Clock::Milliseconds16(1300)),
          .includeVersionInfo              = true,
          .dataModelRev                    = 1,
          .interactionModelRev             = 2,
          .specVersion                     = 1,
          .maxPaths                        = 10,
          .expectedErrorDecodeParams       = CHIP_NO_ERROR,
          .expectedErrorOuterExitContainer = CHIP_NO_ERROR,
          .testFutureProofing              = true },

        // test case is expected to fail since the nested end container is not included in the encoded TLV packet. However, The
        // failure will occur at the outer ExitContainer since the inner ExitContainer will SkipToEndOfContainer
        { .testCaseName = "6. All parameters AND Exclude End Container",
          .mrpConfig    = ReliableMessageProtocolConfig(System::Clock::Milliseconds32(1500), System::Clock::Milliseconds32(2500),
                                                        System::Clock::Milliseconds16(1500)),
          .includeVersionInfo              = true,
          .dataModelRev                    = 1,
          .interactionModelRev             = 2,
          .specVersion                     = 1,
          .maxPaths                        = 10,
          .expectedErrorDecodeParams       = CHIP_NO_ERROR,
          .expectedErrorOuterExitContainer = CHIP_END_OF_TLV,
          .includeEndContainer             = false },

        { .testCaseName = "7. Only MRP Params AND Exclude End Container",
          .mrpConfig    = ReliableMessageProtocolConfig(System::Clock::Milliseconds32(1500), System::Clock::Milliseconds32(2500),
                                                        System::Clock::Milliseconds16(1500)),
          .includeVersionInfo              = false,
          .expectedErrorDecodeParams       = CHIP_NO_ERROR,
          .expectedErrorOuterExitContainer = CHIP_END_OF_TLV,
          .includeEndContainer             = false },

        // Testing the corner case where a TLV element is added to the specification in the future, yet no EndContainer is
        // included in the nested session-parameter-struct
        { .testCaseName = "8. All parameters AND Test Future Proofing AND Exclude End Container",
          .mrpConfig    = ReliableMessageProtocolConfig(System::Clock::Milliseconds32(1500), System::Clock::Milliseconds32(2500),
                                                        System::Clock::Milliseconds16(1500)),
          .includeVersionInfo              = true,
          .dataModelRev                    = 1,
          .interactionModelRev             = 2,
          .specVersion                     = 1,
          .maxPaths                        = 10,
          .expectedErrorDecodeParams       = CHIP_NO_ERROR,
          .expectedErrorOuterExitContainer = CHIP_END_OF_TLV,
          .testFutureProofing              = true,
          .includeEndContainer             = false },

    };

    bool testFailed = false;

    for (const auto & testCase : testCases)
    {
        System::PacketBufferHandle msg = System::PacketBufferHandle::New(64, 0);
        System::PacketBufferTLVWriter writer;
        writer.Init(msg.Retain());
        TLVType outerContainerType = kTLVType_NotSpecified;

        EXPECT_EQ(writer.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainerType), CHIP_NO_ERROR);
        EncodeSessionParamsHelper(testCase, writer);
        EXPECT_EQ(writer.EndContainer(outerContainerType), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&msg), CHIP_NO_ERROR);

        SessionParameters decodedParams;
        System::PacketBufferTLVReader reader;
        TLVType containerType = kTLVType_Structure;
        reader.Init(std::move(msg));
        EXPECT_EQ(reader.Next(containerType, AnonymousTag()), CHIP_NO_ERROR);
        EXPECT_EQ(reader.EnterContainer(containerType), CHIP_NO_ERROR);
        EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

        bool testCaseFailed = false;
        CHIP_ERROR err      = CHIP_NO_ERROR;

        err = DecodeSessionParametersIfPresent(ContextTag(1), reader, decodedParams);
        if (err != testCase.expectedErrorDecodeParams)
        {
            printf("\nDecodeSessionParametersIfPresent returned Unexpected error code: %" CHIP_ERROR_FORMAT, err.Format());
            printf("\nExpected Error Code: %" CHIP_ERROR_FORMAT "\n", testCase.expectedErrorDecodeParams.Format());

            testCaseFailed = true;
        }
        err = reader.Next();
        EXPECT_TRUE(err == CHIP_NO_ERROR || err == CHIP_END_OF_TLV);
        if (reader.ExitContainer(containerType) != testCase.expectedErrorOuterExitContainer)
        {
            printf("\nOuter ExitContainer() returned Unexpected error code: %" CHIP_ERROR_FORMAT, err.Format());
            printf("\nExpected Error Code: %" CHIP_ERROR_FORMAT "\n", testCase.expectedErrorOuterExitContainer.Format());
            testCaseFailed = true;
        }

        if (!testCaseFailed)
        {
            bool verifyResult = VerifyDecodedParameters(testCase, decodedParams);
            if (!verifyResult)
            {
                testCaseFailed = true;
            }
        }

        if (!testCaseFailed)
        {
            printf("\n[PASSED] TestCaseName%s\n\n", testCase.testCaseName);
        }
        else
        {
            printf("\n[FAILED] TestCaseName: %s\n\n", testCase.testCaseName);
            testFailed = true;
        }
    }
    EXPECT_FALSE(testFailed);
}

// Test Suite
