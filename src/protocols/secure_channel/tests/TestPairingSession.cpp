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
#include <nlunit-test.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <protocols/secure_channel/PairingSession.h>
#include <stdarg.h>
#include <system/SystemClock.h>
#include <system/TLVPacketBufferBackingStore.h>

using namespace chip;
using namespace chip::System::Clock;

class TestPairingSession : public PairingSession
{
public:
    Transport::SecureSession::Type GetSecureSessionType() const override { return Transport::SecureSession::Type::kPASE; }
    ScopedNodeId GetPeer() const override { return ScopedNodeId(); }
    ScopedNodeId GetLocalScopedNodeId() const override { return ScopedNodeId(); }
    CATValues GetPeerCATs() const override { return CATValues(); };

    void OnSessionReleased() override {}

    const ReliableMessageProtocolConfig & GetRemoteMRPConfig() const { return mRemoteMRPConfig; }

    CHIP_ERROR DeriveSecureSession(CryptoContext & session) const override { return CHIP_NO_ERROR; }

    CHIP_ERROR DecodeMRPParametersIfPresent(TLV::Tag expectedTag, System::PacketBufferTLVReader & tlvReader)
    {
        return PairingSession::DecodeMRPParametersIfPresent(expectedTag, tlvReader);
    }
};

void PairingSessionEncodeDecodeMRPParams(nlTestSuite * inSuite, void * inContext)
{
    TestPairingSession session;

    ReliableMessageProtocolConfig config(Milliseconds32(100), Milliseconds32(200));

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(64, 0);
    System::PacketBufferTLVWriter writer;
    writer.Init(buf.Retain());

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    NL_TEST_ASSERT(inSuite,
                   writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, PairingSession::EncodeMRPParameters(TLV::ContextTag(1), config, writer) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, writer.EndContainer(outerContainerType) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, writer.Finalize(&buf) == CHIP_NO_ERROR);

    System::PacketBufferTLVReader reader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    reader.Init(std::move(buf));
    NL_TEST_ASSERT(inSuite, reader.Next(containerType, TLV::AnonymousTag()) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, reader.EnterContainer(containerType) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, reader.Next() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, session.DecodeMRPParametersIfPresent(TLV::ContextTag(1), reader) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, session.GetRemoteMRPConfig() == config);
}

void PairingSessionTryDecodeMissingMRPParams(nlTestSuite * inSuite, void * inContext)
{
    TestPairingSession session;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(64, 0);
    System::PacketBufferTLVWriter writer;
    writer.Init(buf.Retain());

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    NL_TEST_ASSERT(inSuite,
                   writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, writer.Put(TLV::ContextTag(1), static_cast<uint16_t>(0x1234)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, writer.EndContainer(outerContainerType) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, writer.Finalize(&buf) == CHIP_NO_ERROR);

    System::PacketBufferTLVReader reader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    reader.Init(std::move(buf));
    NL_TEST_ASSERT(inSuite, reader.Next(containerType, TLV::AnonymousTag()) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, reader.EnterContainer(containerType) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, reader.Next() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, session.DecodeMRPParametersIfPresent(TLV::ContextTag(2), reader) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, session.GetRemoteMRPConfig() == GetDefaultMRPConfig());
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Encode and Decode MRP params", PairingSessionEncodeDecodeMRPParams),
    NL_TEST_DEF("Decode missing MRP params", PairingSessionTryDecodeMissingMRPParams),

    NL_TEST_SENTINEL()
};
// clang-format on

/**
 *  Set up the test suite.
 */
int TestPairingSession_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestPairingSession_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-PairingSession",
    &sTests[0],
    TestPairingSession_Setup,
    TestPairingSession_Teardown
};
// clang-format on

/**
 *  Main
 */
int TestPairingSessionInit()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestPairingSessionInit)
