/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;

void TestInit(nlTestSuite * inSuite, void * inContext)
{
    Thread::OperationalDataset & dataset = *static_cast<Thread::OperationalDataset *>(inContext);

    NL_TEST_ASSERT(inSuite, dataset.Init(ByteSpan(nullptr, 255)) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, dataset.Init(ByteSpan(nullptr, 0)) == CHIP_NO_ERROR);

    {
        uint8_t data[] = { 0x01, 0x02, 0x03 };

        NL_TEST_ASSERT(inSuite, dataset.Init(ByteSpan(data)) == CHIP_ERROR_INVALID_ARGUMENT);
    }

    {
        uint8_t data[] = { 0x01 };

        NL_TEST_ASSERT(inSuite, dataset.Init(ByteSpan(data)) == CHIP_ERROR_INVALID_ARGUMENT);
    }
}

void TestActiveTimestamp(nlTestSuite * inSuite, void * inContext)
{
    static constexpr uint64_t kActiveTimestampValue = 1;

    Thread::OperationalDataset & dataset = *static_cast<Thread::OperationalDataset *>(inContext);

    uint64_t activeTimestamp = 0;

    NL_TEST_ASSERT(inSuite, dataset.SetActiveTimestamp(kActiveTimestampValue) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, dataset.GetActiveTimestamp(activeTimestamp) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, activeTimestamp == kActiveTimestampValue);
}

void TestChannel(nlTestSuite * inSuite, void * inContext)
{
    static constexpr uint16_t kChannelValue = 15;

    Thread::OperationalDataset & dataset = *static_cast<Thread::OperationalDataset *>(inContext);

    uint16_t channel = 0;

    NL_TEST_ASSERT(inSuite, dataset.SetChannel(kChannelValue) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, dataset.GetChannel(channel) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, channel == kChannelValue);
}

void TestExtendedPanId(nlTestSuite * inSuite, void * inContext)
{
    static constexpr uint8_t kExtendedPanId[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };

    Thread::OperationalDataset & dataset = *static_cast<Thread::OperationalDataset *>(inContext);

    uint8_t extendedPanId[Thread::kSizeExtendedPanId] = { 0 };

    NL_TEST_ASSERT(inSuite, dataset.SetExtendedPanId(kExtendedPanId) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, dataset.GetExtendedPanId(extendedPanId) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(extendedPanId, kExtendedPanId, sizeof(kExtendedPanId)) == 0);
}

void TestMasterKey(nlTestSuite * inSuite, void * inContext)
{
    static constexpr uint8_t kMasterKey[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                              0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

    Thread::OperationalDataset & dataset = *static_cast<Thread::OperationalDataset *>(inContext);

    uint8_t masterKey[Thread::kSizeMasterKey] = { 0 };

    NL_TEST_ASSERT(inSuite, dataset.SetMasterKey(kMasterKey) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, dataset.GetMasterKey(masterKey) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(masterKey, kMasterKey, sizeof(kMasterKey)) == 0);
}

void TestMeshLocalPrefix(nlTestSuite * inSuite, void * inContext)
{
    static constexpr uint8_t kMeshLocalPrefix[] = { 0xfd, 0xde, 0xad, 0x00, 0xbe, 0xef, 0x00, 0x00 };

    Thread::OperationalDataset & dataset = *static_cast<Thread::OperationalDataset *>(inContext);

    uint8_t meshLocalPrefix[Thread::kSizeMeshLocalPrefix] = { 0 };

    NL_TEST_ASSERT(inSuite, dataset.SetMeshLocalPrefix(kMeshLocalPrefix) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, dataset.GetMeshLocalPrefix(meshLocalPrefix) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(meshLocalPrefix, kMeshLocalPrefix, sizeof(kMeshLocalPrefix)) == 0);
}

void TestNetworkName(nlTestSuite * inSuite, void * inContext)
{
    static constexpr char kNetworkName[] = "ThreadNetwork";

    Thread::OperationalDataset & dataset = *static_cast<Thread::OperationalDataset *>(inContext);

    char networkName[Thread::kSizeNetworkName + 1] = { 0 };

    NL_TEST_ASSERT(inSuite, dataset.SetNetworkName(kNetworkName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, dataset.GetNetworkName(networkName) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(networkName, kNetworkName) == 0);

    NL_TEST_ASSERT(inSuite, dataset.SetNetworkName("0123456789abcdef") == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, dataset.SetNetworkName("0123456789abcdefg") == CHIP_ERROR_INVALID_STRING_LENGTH);
    NL_TEST_ASSERT(inSuite, dataset.SetNetworkName("") == CHIP_ERROR_INVALID_STRING_LENGTH);
}

void TestPanId(nlTestSuite * inSuite, void * inContext)
{
    static constexpr uint16_t kPanIdValue = 0x1234;

    Thread::OperationalDataset & dataset = *static_cast<Thread::OperationalDataset *>(inContext);

    uint16_t panid = 0;

    NL_TEST_ASSERT(inSuite, dataset.SetPanId(kPanIdValue) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, dataset.GetPanId(panid) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, panid == kPanIdValue);
}

void TestPSKc(nlTestSuite * inSuite, void * inContext)
{
    static constexpr uint8_t kPSKc[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                         0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

    Thread::OperationalDataset & dataset = *static_cast<Thread::OperationalDataset *>(inContext);

    uint8_t pskc[Thread::kSizePSKc] = { 0 };

    NL_TEST_ASSERT(inSuite, dataset.SetPSKc(kPSKc) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, dataset.GetPSKc(pskc) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(pskc, kPSKc, sizeof(kPSKc)) == 0);
}

void TestUnsetMasterKey(nlTestSuite * inSuite, void * inContext)
{
    Thread::OperationalDataset & dataset = *static_cast<Thread::OperationalDataset *>(inContext);

    uint8_t masterKey[Thread::kSizeMasterKey] = { 0 };

    NL_TEST_ASSERT(inSuite, dataset.GetMasterKey(masterKey) == CHIP_NO_ERROR);
    dataset.UnsetMasterKey();
    NL_TEST_ASSERT(inSuite, dataset.GetMasterKey(masterKey) == CHIP_ERROR_TLV_TAG_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, dataset.SetMasterKey(masterKey) == CHIP_NO_ERROR);
}

void TestUnsetPSKc(nlTestSuite * inSuite, void * inContext)
{
    Thread::OperationalDataset & dataset = *static_cast<Thread::OperationalDataset *>(inContext);

    uint8_t pskc[Thread::kSizePSKc] = { 0 };

    NL_TEST_ASSERT(inSuite, dataset.GetPSKc(pskc) == CHIP_NO_ERROR);
    dataset.UnsetPSKc();
    NL_TEST_ASSERT(inSuite, dataset.GetPSKc(pskc) == CHIP_ERROR_TLV_TAG_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, dataset.SetPSKc(pskc) == CHIP_NO_ERROR);
}

void TestClear(nlTestSuite * inSuite, void * inContext)
{
    Thread::OperationalDataset & dataset = *static_cast<Thread::OperationalDataset *>(inContext);

    {
        uint64_t activeTimestamp;
        NL_TEST_ASSERT(inSuite, dataset.GetActiveTimestamp(activeTimestamp) == CHIP_NO_ERROR);
    }

    {
        uint16_t channel;
        NL_TEST_ASSERT(inSuite, dataset.GetChannel(channel) == CHIP_NO_ERROR);
    }

    {
        uint8_t extendedPanId[Thread::kSizeExtendedPanId] = { 0 };
        NL_TEST_ASSERT(inSuite, dataset.GetExtendedPanId(extendedPanId) == CHIP_NO_ERROR);
    }

    {
        uint8_t masterKey[Thread::kSizeMasterKey] = { 0 };
        NL_TEST_ASSERT(inSuite, dataset.GetMasterKey(masterKey) == CHIP_NO_ERROR);
    }

    {
        uint8_t meshLocalPrefix[Thread::kSizeMeshLocalPrefix] = { 0 };
        NL_TEST_ASSERT(inSuite, dataset.GetMeshLocalPrefix(meshLocalPrefix) == CHIP_NO_ERROR);
    }

    {
        char networkName[Thread::kSizeNetworkName + 1] = { 0 };
        NL_TEST_ASSERT(inSuite, dataset.GetNetworkName(networkName) == CHIP_NO_ERROR);
    }

    {
        uint16_t panid;
        NL_TEST_ASSERT(inSuite, dataset.GetPanId(panid) == CHIP_NO_ERROR);
    }

    {
        uint8_t pskc[Thread::kSizePSKc] = { 0 };
        NL_TEST_ASSERT(inSuite, dataset.GetPSKc(pskc) == CHIP_NO_ERROR);
    }

    dataset.Clear();

    {
        uint64_t activeTimestamp;
        NL_TEST_ASSERT(inSuite, dataset.GetActiveTimestamp(activeTimestamp) == CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }

    {
        uint16_t channel;
        NL_TEST_ASSERT(inSuite, dataset.GetChannel(channel) == CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }

    {
        uint8_t extendedPanId[Thread::kSizeExtendedPanId] = { 0 };
        NL_TEST_ASSERT(inSuite, dataset.GetExtendedPanId(extendedPanId) == CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }

    {
        uint8_t masterKey[Thread::kSizeMasterKey] = { 0 };
        NL_TEST_ASSERT(inSuite, dataset.GetMasterKey(masterKey) == CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }

    {
        uint8_t meshLocalPrefix[Thread::kSizeMeshLocalPrefix] = { 0 };
        NL_TEST_ASSERT(inSuite, dataset.GetMeshLocalPrefix(meshLocalPrefix) == CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }

    {
        char networkName[Thread::kSizeNetworkName + 1] = { 0 };
        NL_TEST_ASSERT(inSuite, dataset.GetNetworkName(networkName) == CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }

    {
        uint16_t panid;
        NL_TEST_ASSERT(inSuite, dataset.GetPanId(panid) == CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }

    {
        uint8_t pskc[Thread::kSizePSKc] = { 0 };
        NL_TEST_ASSERT(inSuite, dataset.GetPSKc(pskc) == CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestInit", TestInit),                       //
    NL_TEST_DEF("TestActiveTimestamp", TestActiveTimestamp), //
    NL_TEST_DEF("TestChannel", TestChannel),                 //
    NL_TEST_DEF("TestExtendedPanId", TestExtendedPanId),     //
    NL_TEST_DEF("TestMasterKey", TestMasterKey),             //
    NL_TEST_DEF("TestMeshLocalPrefix", TestMeshLocalPrefix), //
    NL_TEST_DEF("TestNetworkName", TestNetworkName),         //
    NL_TEST_DEF("TestPanId", TestPanId),                     //
    NL_TEST_DEF("TestPSKc", TestPSKc),                       //
    NL_TEST_DEF("TestUnsetMasterKey", TestUnsetMasterKey),   //
    NL_TEST_DEF("TestUnsetPSKc", TestUnsetPSKc),             //
    NL_TEST_DEF("TestClear", TestClear),                     //
    NL_TEST_SENTINEL()                                       //
};

} // namespace

int TestThreadOperationalDatasetBuilder(void)
{
    nlTestSuite theSuite = { "ThreadOperationalDataset", sTests, nullptr, nullptr };
    Thread::OperationalDataset dataset{};

    nlTestRunner(&theSuite, &dataset);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestThreadOperationalDatasetBuilder)
