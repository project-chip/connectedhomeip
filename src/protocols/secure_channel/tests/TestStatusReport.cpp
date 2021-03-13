/**
 *    @file
 *      Tests for the StatusReport class.
 */

#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <support/BufferWriter.h>
#include <support/CHIPMem.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemPacketBuffer.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::Protocols;
using namespace chip::Protocols::SecureChannel;

void TestStatusReport_NoData(nlTestSuite * inSuite, void * inContext)
{
    uint16_t generalCode  = static_cast<uint16_t>(GeneralStatusCode::Success);
    uint32_t protocolId   = kProtocol_SecureChannel;
    uint16_t protocolCode = kProtocolCodeSuccess;

    StatusReport testReport(generalCode, protocolId, protocolCode);

    size_t msgSize = testReport.Size();
    Encoding::LittleEndian::PacketBufferWriter bbuf(System::PacketBufferHandle::New(msgSize));
    testReport.WriteToBuffer(bbuf);

    System::PacketBufferHandle msgBuf = bbuf.Finalize();
    NL_TEST_ASSERT(inSuite, !msgBuf.IsNull());

    StatusReport report2;
    CHIP_ERROR err = report2.Parse(std::move(msgBuf));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, report2.GetGeneralCode() == generalCode);
    NL_TEST_ASSERT(inSuite, report2.GetProtocolId() == protocolId);
    NL_TEST_ASSERT(inSuite, report2.GetProtocolCode() == protocolCode);

    uint8_t * testData = nullptr;
    uint16_t dataLen   = 0;
    report2.GetProtocolData(&testData, dataLen);
    NL_TEST_ASSERT(inSuite, testData == nullptr);
    NL_TEST_ASSERT(inSuite, dataLen == 0);
}

void TestStatusReport_WithData(nlTestSuite * inSuite, void * inContext)
{
    uint16_t generalCode   = static_cast<uint16_t>(GeneralStatusCode::Failure);
    uint32_t protocolId    = kProtocol_SecureChannel;
    uint16_t protocolCode  = static_cast<uint16_t>(StatusCode::InvalidFabricConfig);
    uint8_t data[6]        = { 42, 19, 3, 1, 3, 0 };
    const uint16_t dataLen = 6;

    StatusReport testReport(generalCode, protocolId, protocolCode, static_cast<uint8_t *>(data), dataLen);

    size_t msgSize = testReport.Size();
    Encoding::LittleEndian::PacketBufferWriter bbuf(System::PacketBufferHandle::New(msgSize));
    testReport.WriteToBuffer(bbuf);

    System::PacketBufferHandle msgBuf = bbuf.Finalize();
    NL_TEST_ASSERT(inSuite, !msgBuf.IsNull());

    StatusReport report2;
    CHIP_ERROR err = report2.Parse(std::move(msgBuf));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, report2.GetGeneralCode() == generalCode);
    NL_TEST_ASSERT(inSuite, report2.GetProtocolId() == protocolId);
    NL_TEST_ASSERT(inSuite, report2.GetProtocolCode() == protocolCode);

    uint8_t * rcvData   = nullptr;
    uint16_t rcvDataLen = 0;
    report2.GetProtocolData(&rcvData, rcvDataLen);
    if (rcvData == nullptr)
    {
        NL_TEST_ASSERT(inSuite, false);
        return;
    }
    NL_TEST_ASSERT(inSuite, !memcmp(rcvData, data, rcvDataLen));
    NL_TEST_ASSERT(inSuite, rcvDataLen = dataLen);
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
