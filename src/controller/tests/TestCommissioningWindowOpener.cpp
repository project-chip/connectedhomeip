/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <gtest/gtest.h>

#include <controller/CommissioningWindowOpener.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;

namespace {

class MockDeviceController : public Controller::DeviceController
{
public:
    CHIP_ERROR
    GetConnectedDevice(NodeId peerNodeId, Callback::Callback<OnDeviceConnected> * onConnection,
                       Callback::Callback<OnDeviceConnectionFailure> * onFailure,
                       TransportPayloadCapability transportPayloadCapability = TransportPayloadCapability::kMRPPayload) override
    {
        return CHIP_NO_ERROR;
    }
};

// Valid crypto values from src/protocols/secure_channel/tests/TestPASESession.cpp
constexpr uint32_t sTestSpake2p01_PinCode                                     = 20202021;
constexpr uint32_t sTestSpake2p01_IterationCount                              = 1000;
constexpr uint8_t sTestSpake2p01_Salt[]                                       = { 0x53, 0x50, 0x41, 0x4B, 0x45, 0x32, 0x50, 0x20,
                                                                                  0x4B, 0x65, 0x79, 0x20, 0x53, 0x61, 0x6C, 0x74 };
constexpr Crypto::Spake2pVerifierSerialized sTestSpake2p01_SerializedVerifier = {
    0xB9, 0x61, 0x70, 0xAA, 0xE8, 0x03, 0x34, 0x68, 0x84, 0x72, 0x4F, 0xE9, 0xA3, 0xB2, 0x87, 0xC3, 0x03, 0x30, 0xC2, 0xA6,
    0x60, 0x37, 0x5D, 0x17, 0xBB, 0x20, 0x5A, 0x8C, 0xF1, 0xAE, 0xCB, 0x35, 0x04, 0x57, 0xF8, 0xAB, 0x79, 0xEE, 0x25, 0x3A,
    0xB6, 0xA8, 0xE4, 0x6B, 0xB0, 0x9E, 0x54, 0x3A, 0xE4, 0x22, 0x73, 0x6D, 0xE5, 0x01, 0xE3, 0xDB, 0x37, 0xD4, 0x41, 0xFE,
    0x34, 0x49, 0x20, 0xD0, 0x95, 0x48, 0xE4, 0xC1, 0x82, 0x40, 0x63, 0x0C, 0x4F, 0xF4, 0x91, 0x3C, 0x53, 0x51, 0x38, 0x39,
    0xB7, 0xC0, 0x7F, 0xCC, 0x06, 0x27, 0xA1, 0xB8, 0x57, 0x3A, 0x14, 0x9F, 0xCD, 0x1F, 0xA4, 0x66, 0xCF
};

static void OCWPasscodeCallback(void * context, NodeId deviceId, CHIP_ERROR status, SetupPayload payload) {}
static void OCWVerifierCallback(void * context, NodeId deviceId, CHIP_ERROR status) {}

class TestCommissioningWindowOpener : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    // Initialize with a null pointer for now, replace with a valid controller pointer if available
    MockDeviceController mockController;
    Controller::CommissioningWindowOpener opener = Controller::CommissioningWindowOpener(&mockController);
};

TEST_F(TestCommissioningWindowOpener, OpenCommissioningWindowVerifier_Success)
{
    Callback::Callback<Controller::OnOpenCommissioningWindowWithVerifier> callback(OCWVerifierCallback, this);

    CHIP_ERROR err = opener.OpenCommissioningWindow(Controller::CommissioningWindowVerifierParams()
                                                        .SetNodeId(0x1234)
                                                        .SetTimeout(300)
                                                        .SetIteration(sTestSpake2p01_IterationCount)
                                                        .SetDiscriminator(3840)
                                                        .SetSalt(ByteSpan(sTestSpake2p01_Salt))
                                                        .SetVerifier(ByteSpan(sTestSpake2p01_SerializedVerifier))
                                                        .SetCallback(&callback));
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestCommissioningWindowOpener, OpenCommissioningWindowVerifier_Failure_InvalidSalt)
{
    Callback::Callback<Controller::OnOpenCommissioningWindowWithVerifier> callback(OCWVerifierCallback, this);

    CHIP_ERROR err = opener.OpenCommissioningWindow(Controller::CommissioningWindowVerifierParams()
                                                        .SetNodeId(0x1234)
                                                        .SetTimeout(300)
                                                        .SetIteration(sTestSpake2p01_IterationCount)
                                                        .SetDiscriminator(3840)
                                                        .SetSalt(ByteSpan())
                                                        .SetVerifier(ByteSpan(sTestSpake2p01_SerializedVerifier))
                                                        .SetCallback(&callback));
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestCommissioningWindowOpener, OpenCommissioningWindowVerifier_Failure_InvalidVerifier)
{
    Callback::Callback<Controller::OnOpenCommissioningWindowWithVerifier> callback(OCWVerifierCallback, this);

    CHIP_ERROR err = opener.OpenCommissioningWindow(Controller::CommissioningWindowVerifierParams()
                                                        .SetNodeId(0x1234)
                                                        .SetTimeout(300)
                                                        .SetIteration(sTestSpake2p01_IterationCount)
                                                        .SetDiscriminator(3840)
                                                        .SetSalt(ByteSpan(sTestSpake2p01_Salt))
                                                        .SetVerifier(ByteSpan())
                                                        .SetCallback(&callback));
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestCommissioningWindowOpener, OpenCommissioningWindowVerifier_Failure_InvalidIteration)
{
    Callback::Callback<Controller::OnOpenCommissioningWindowWithVerifier> callback(OCWVerifierCallback, this);

    CHIP_ERROR err = opener.OpenCommissioningWindow(Controller::CommissioningWindowVerifierParams()
                                                        .SetNodeId(0x1234)
                                                        .SetTimeout(300)
                                                        .SetIteration(0)
                                                        .SetDiscriminator(3840)
                                                        .SetSalt(ByteSpan(sTestSpake2p01_Salt))
                                                        .SetVerifier(ByteSpan(sTestSpake2p01_SerializedVerifier))
                                                        .SetCallback(&callback));
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestCommissioningWindowOpener, OpenCommissioningWindowPasscode_Success)
{
    SetupPayload ignored;
    Callback::Callback<Controller::OnOpenCommissioningWindow> callback(OCWPasscodeCallback, this);
    CHIP_ERROR err = opener.OpenCommissioningWindow(Controller::CommissioningWindowPasscodeParams()
                                                        .SetNodeId(0x1234)
                                                        .SetTimeout(300)
                                                        .SetIteration(sTestSpake2p01_IterationCount)
                                                        .SetDiscriminator(3840)
                                                        .SetSetupPIN(sTestSpake2p01_PinCode)
                                                        .SetReadVIDPIDAttributes(true)
                                                        .SetSalt(ByteSpan(sTestSpake2p01_Salt))
                                                        .SetCallback(&callback),
                                                    ignored);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestCommissioningWindowOpener, OpenCommissioningWindowPasscode_Success_NoPin)
{
    SetupPayload ignored;
    Callback::Callback<Controller::OnOpenCommissioningWindow> callback(OCWPasscodeCallback, this);
    CHIP_ERROR err = opener.OpenCommissioningWindow(Controller::CommissioningWindowPasscodeParams()
                                                        .SetNodeId(0x1234)
                                                        .SetTimeout(300)
                                                        .SetIteration(sTestSpake2p01_IterationCount)
                                                        .SetDiscriminator(3840)
                                                        .SetSalt(ByteSpan(sTestSpake2p01_Salt))
                                                        .SetCallback(&callback),
                                                    ignored);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestCommissioningWindowOpener, OpenCommissioningWindowPasscode_Success_NoSalt)
{
    SetupPayload ignored;
    Callback::Callback<Controller::OnOpenCommissioningWindow> callback(OCWPasscodeCallback, this);
    CHIP_ERROR err = opener.OpenCommissioningWindow(Controller::CommissioningWindowPasscodeParams()
                                                        .SetNodeId(0x1234)
                                                        .SetTimeout(300)
                                                        .SetIteration(sTestSpake2p01_IterationCount)
                                                        .SetDiscriminator(3840)
                                                        .SetSetupPIN(sTestSpake2p01_PinCode)
                                                        .SetCallback(&callback),
                                                    ignored);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestCommissioningWindowOpener, OpenCommissioningWindowPasscode_Failure_InvalidIteration)
{
    SetupPayload ignored;
    Callback::Callback<Controller::OnOpenCommissioningWindow> callback(OCWPasscodeCallback, this);
    CHIP_ERROR err = opener.OpenCommissioningWindow(Controller::CommissioningWindowPasscodeParams()
                                                        .SetNodeId(0x1234)
                                                        .SetTimeout(300)
                                                        .SetIteration(0)
                                                        .SetDiscriminator(3840)
                                                        .SetCallback(&callback),
                                                    ignored);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}

// Add more test cases as needed to cover different scenarios
} // namespace
