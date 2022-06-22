/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <system/SystemPacketBuffer.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::Protocols;
using namespace chip::Protocols::SecureChannel;

void TestStatusReport_NoData(nlTestSuite * inSuite, void * inContext)
{
    GeneralStatusCode generalCode = GeneralStatusCode::kSuccess;
    auto protocolId               = SecureChannel::Id;
    uint16_t protocolCode         = kProtocolCodeSuccess;

    StatusReport testReport(generalCode, protocolId, protocolCode);

    size_t msgSize = testReport.Size();
    Encoding::LittleEndian::PacketBufferWriter bbuf(System::PacketBufferHandle::New(msgSize));
    testReport.WriteToBuffer(bbuf);

    System::PacketBufferHandle msgBuf = bbuf.Finalize();
    NL_TEST_ASSERT(inSuite, !msgBuf.IsNull());

    StatusReport reportToParse;
    CHIP_ERROR err = reportToParse.Parse(std::move(msgBuf));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, reportToParse.GetGeneralCode() == generalCode);
    NL_TEST_ASSERT(inSuite, reportToParse.GetProtocolId() == protocolId);
    NL_TEST_ASSERT(inSuite, reportToParse.GetProtocolCode() == protocolCode);

    const System::PacketBufferHandle & data = reportToParse.GetProtocolData();
    NL_TEST_ASSERT(inSuite, data.IsNull());
}

void TestStatusReport_WithData(nlTestSuite * inSuite, void * inContext)
{
    GeneralStatusCode generalCode      = GeneralStatusCode::kFailure;
    auto protocolId                    = SecureChannel::Id;
    uint16_t protocolCode              = static_cast<uint16_t>(StatusCode::InvalidFabricConfig);
    uint8_t data[6]                    = { 42, 19, 3, 1, 3, 0 };
    const uint16_t dataLen             = 6;
    System::PacketBufferHandle dataBuf = System::PacketBufferHandle::NewWithData(data, dataLen);

    StatusReport testReport(generalCode, protocolId, protocolCode, std::move(dataBuf));

    size_t msgSize = testReport.Size();
    Encoding::LittleEndian::PacketBufferWriter bbuf(System::PacketBufferHandle::New(msgSize));
    testReport.WriteToBuffer(bbuf);

    System::PacketBufferHandle msgBuf = bbuf.Finalize();
    NL_TEST_ASSERT(inSuite, !msgBuf.IsNull());

    StatusReport reportToParse;
    CHIP_ERROR err = reportToParse.Parse(std::move(msgBuf));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, reportToParse.GetGeneralCode() == generalCode);
    NL_TEST_ASSERT(inSuite, reportToParse.GetProtocolId() == protocolId);
    NL_TEST_ASSERT(inSuite, reportToParse.GetProtocolCode() == protocolCode);

    const System::PacketBufferHandle & rcvData = reportToParse.GetProtocolData();
    if (rcvData.IsNull())
    {
        NL_TEST_ASSERT(inSuite, false);
        return;
    }
    NL_TEST_ASSERT(inSuite, rcvData->DataLength() == dataLen);
    NL_TEST_ASSERT(inSuite, !memcmp(rcvData->Start(), data, dataLen));
}

void TestBadStatusReport(nlTestSuite * inSuite, void * inContext)
{
    StatusReport report;
    System::PacketBufferHandle badMsg = System::PacketBufferHandle::New(10);
    CHIP_ERROR err                    = report.Parse(std::move(badMsg));
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    StatusReport report2;
    badMsg = nullptr;
    err    = report2.Parse(std::move(badMsg));
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestStatusReport_NoData", TestStatusReport_NoData),
    NL_TEST_DEF("TestStatusReport_WithData", TestStatusReport_WithData),
    NL_TEST_DEF("TestBadStatusReport", TestBadStatusReport),

    NL_TEST_SENTINEL()
};
// clang-format on

/**
 *  Set up the test suite.
 */
static int TestSetup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
static int TestTeardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-StatusReport",
    &sTests[0],
    TestSetup,
    TestTeardown,
};
// clang-format on

/**
 *  Main
 */
int TestStatusReport()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestStatusReport)
