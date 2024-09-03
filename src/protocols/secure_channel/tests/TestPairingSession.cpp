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

    CHIP_ERROR DecodeMRPParametersIfPresent(TLV::Tag expectedTag, System::PacketBufferTLVReader & tlvReader)
    {
        return PairingSession::DecodeMRPParametersIfPresent(expectedTag, tlvReader);
    }
};

TEST_F(TestPairingSession, PairingSessionEncodeDecodeMRPParams)
{
    ReliableMessageProtocolConfig config(Milliseconds32(100), Milliseconds32(200), Milliseconds16(4000));

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(64, 0);
    System::PacketBufferTLVWriter writer;
    writer.Init(buf.Retain());

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType), CHIP_NO_ERROR);

    EXPECT_EQ(PairingSession::EncodeSessionParameters(TLV::ContextTag(1), config, writer), CHIP_NO_ERROR);

    EXPECT_EQ(writer.EndContainer(outerContainerType), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(&buf), CHIP_NO_ERROR);

    System::PacketBufferTLVReader reader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    reader.Init(std::move(buf));
    EXPECT_EQ(reader.Next(containerType, TLV::AnonymousTag()), CHIP_NO_ERROR);
    EXPECT_EQ(reader.EnterContainer(containerType), CHIP_NO_ERROR);

    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(DecodeMRPParametersIfPresent(TLV::ContextTag(1), reader), CHIP_NO_ERROR);

    EXPECT_EQ(GetRemoteMRPConfig(), config);
}

TEST_F(TestPairingSession, PairingSessionTryDecodeMissingMRPParams)
{
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(64, 0);
    System::PacketBufferTLVWriter writer;
    writer.Init(buf.Retain());

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(1), static_cast<uint16_t>(0x1234)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(outerContainerType), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(&buf), CHIP_NO_ERROR);

    System::PacketBufferTLVReader reader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    reader.Init(std::move(buf));
    EXPECT_EQ(reader.Next(containerType, TLV::AnonymousTag()), CHIP_NO_ERROR);
    EXPECT_EQ(reader.EnterContainer(containerType), CHIP_NO_ERROR);
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(DecodeMRPParametersIfPresent(TLV::ContextTag(2), reader), CHIP_NO_ERROR);

    EXPECT_EQ(GetRemoteMRPConfig(), GetDefaultMRPConfig());
}

// Test Suite
