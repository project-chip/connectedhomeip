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

#include "lib/support/CHIPMem.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/power-source-server/power-source-server.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <nlunit-test.h>
#include <protocols/interaction_model/Constants.h>
#include <type_traits>

#include <vector>

namespace chip {
namespace app {

class TestPowerSourceCluster
{
public:
    static void TestEndpointList(nlTestSuite * apSuite, void * apContext);
};

std::vector<EndpointId> ReadEndpointsThroughAttributeReader(nlTestSuite * apSuite, EndpointId endpoint)
{
    Clusters::PowerSourceAttrAccess & attrAccess = Clusters::TestOnlyGetPowerSourceAttrAccess();
    CHIP_ERROR err                               = CHIP_NO_ERROR;

    // Write TLV through the attribute access interface into the buffer
    constexpr size_t buflen = 128;
    uint8_t buf[buflen];
    memset(buf, 0, buflen);
    AttributeReportIBs::Builder builder;
    TLV::TLVWriter tlvWriter;
    tlvWriter.Init(buf);
    builder.Init(&tlvWriter);
    ConcreteAttributePath path(endpoint, Clusters::PowerSource::Id, Clusters::PowerSource::Attributes::EndpointList::Id);
    chip::DataVersion dataVersion(0);
    AttributeValueEncoder aEncoder(builder, 0, path, dataVersion);
    ConcreteReadAttributePath readPath(path);
    err = attrAccess.Read(readPath, aEncoder);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // Read out from the buffer. This comes back as a nested struct
    // AttributeReportIBs is a list of
    // AttributeReportIB structs containing
    // AttributeDataIB struct, which holds DataVersion (tag 0), AttributePathIB (tag 1) and Data (tag 2)

    TLV::TLVReader reader;
    reader.Init(buf);

    TLV::TLVReader attrReportsReader;
    TLV::TLVReader attrReportReader;
    TLV::TLVReader attrDataReader;

    reader.Next();
    reader.OpenContainer(attrReportsReader);

    attrReportsReader.Next();
    attrReportsReader.OpenContainer(attrReportReader);

    attrReportReader.Next();
    attrReportReader.OpenContainer(attrDataReader);

    // We're now in the attribute data IB, skip the first two tags, we want tag 2
    attrDataReader.Next();
    attrDataReader.Next();
    attrDataReader.Next();
    NL_TEST_ASSERT(apSuite, IsContextTag(attrDataReader.GetTag()));
    NL_TEST_ASSERT(apSuite, TagNumFromTag(attrDataReader.GetTag()) == 2);

    // OK, we should be in the right spot now, let's decode the list.
    Clusters::PowerSource::Attributes::EndpointList::TypeInfo::DecodableType list;
    err = list.Decode(attrDataReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    std::vector<EndpointId> ret;
    auto iter = list.begin();
    while (iter.Next())
    {
        ret.push_back(iter.GetValue());
    }
    return ret;
}

void TestPowerSourceCluster::TestEndpointList(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Clusters::PowerSourceServer & powerSourceServer = Clusters::PowerSourceServer::Instance();

    // test that when we read everything we get an empty list as nothing has been set up yet
    for (EndpointId i = 0; i < 11; ++i)
    {
        std::vector<EndpointId> vec = ReadEndpointsThroughAttributeReader(apSuite, i);
        NL_TEST_ASSERT(apSuite, vec.size() == 0);
    }

    // Test setting and getting
    EndpointId list0[5]    = { 1, 2, 3, 4, 5 };
    EndpointId list1[10]   = { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    EndpointId listRest[1] = { 2 };

    err = powerSourceServer.SetEndpointList(0, Span<EndpointId>(list0));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    const Span<EndpointId> * readBack = powerSourceServer.GetEndpointList(0);
    NL_TEST_EXIT_ON_FAILED_ASSERT(apSuite, readBack != nullptr);
    NL_TEST_ASSERT(apSuite, readBack->size() == 5);
    for (size_t i = 0; i < readBack->size(); ++i)
    {
        NL_TEST_ASSERT(apSuite, readBack->data()[i] == list0[i]);
    }

    err = powerSourceServer.SetEndpointList(1, Span<EndpointId>(list1));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    readBack = powerSourceServer.GetEndpointList(1);
    NL_TEST_EXIT_ON_FAILED_ASSERT(apSuite, readBack != nullptr);
    NL_TEST_ASSERT(apSuite, readBack->size() == 10);
    for (size_t i = 0; i < readBack->size(); ++i)
    {
        NL_TEST_ASSERT(apSuite, readBack->data()[i] == list1[i]);
    }

    // Fill up the list
    for (EndpointId i = 2; i < 10; ++i)
    {
        err = powerSourceServer.SetEndpointList(i, Span<EndpointId>(listRest));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        readBack = powerSourceServer.GetEndpointList(i);
        NL_TEST_EXIT_ON_FAILED_ASSERT(apSuite, readBack != nullptr);
        NL_TEST_ASSERT(apSuite, readBack->size() == 1);
        NL_TEST_ASSERT(apSuite, readBack->data()[0] == listRest[0]);
    }

    // Add one more, make sure we get an out of memory error
    err = powerSourceServer.SetEndpointList(11, Span<EndpointId>(listRest));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NO_MEMORY);

    // Make sure all the stuff we had before is still there
    readBack = powerSourceServer.GetEndpointList(0);
    NL_TEST_EXIT_ON_FAILED_ASSERT(apSuite, readBack != nullptr);
    NL_TEST_ASSERT(apSuite, readBack->size() == 5);
    for (size_t i = 0; i < readBack->size(); ++i)
    {
        NL_TEST_ASSERT(apSuite, readBack->data()[i] == list0[i]);
    }

    readBack = powerSourceServer.GetEndpointList(1);
    NL_TEST_EXIT_ON_FAILED_ASSERT(apSuite, readBack != nullptr);
    NL_TEST_ASSERT(apSuite, readBack->size() == 10);
    for (size_t i = 0; i < readBack->size(); ++i)
    {
        NL_TEST_ASSERT(apSuite, readBack->data()[i] == list1[i]);
    }

    for (EndpointId i = 2; i < 10; ++i)
    {
        readBack = powerSourceServer.GetEndpointList(i);
        NL_TEST_EXIT_ON_FAILED_ASSERT(apSuite, readBack != nullptr);
        NL_TEST_ASSERT(apSuite, readBack->size() == 1);
        NL_TEST_ASSERT(apSuite, readBack->data()[0] == listRest[0]);
    }

    // Try overwriting a list
    err = powerSourceServer.SetEndpointList(1, Span<EndpointId>(listRest));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    readBack = powerSourceServer.GetEndpointList(1);
    NL_TEST_ASSERT(apSuite, readBack->size() == 1);
    NL_TEST_ASSERT(apSuite, readBack->data()[0] == listRest[0]);

    // Now let's try it through the read interface
    for (EndpointId i = 0; i < 11; ++i)
    {
        std::vector<EndpointId> vec = ReadEndpointsThroughAttributeReader(apSuite, i);
        if (i == 0)
        {
            NL_TEST_ASSERT(apSuite, vec.size() == 5);
            for (size_t j = 0; j < vec.size(); ++j)
            {
                NL_TEST_ASSERT(apSuite, vec[j] == list0[j]);
            }
        }
        else if (i == 10)
        {
            NL_TEST_ASSERT(apSuite, vec.size() == 0);
        }
        else
        {
            NL_TEST_ASSERT(apSuite, vec.size() == 1);
            NL_TEST_ASSERT(apSuite, vec[0] == listRest[0]);
        }
    }

    // Now see if we can remove everything
    for (EndpointId i = 0; i < 10; ++i)
    {
        err = powerSourceServer.SetEndpointList(i, Span<EndpointId>());
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        readBack = powerSourceServer.GetEndpointList(i);
        NL_TEST_ASSERT(apSuite, readBack == nullptr);
    }

    // Check through the read interface
    for (EndpointId i = 0; i < 11; ++i)
    {
        std::vector<EndpointId> vec = ReadEndpointsThroughAttributeReader(apSuite, i);
        NL_TEST_ASSERT(apSuite, vec.size() == 0);
    }
}

} // namespace app
} // namespace chip

namespace {

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestEndpointList", chip::app::TestPowerSourceCluster::TestEndpointList),
    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestPowerSourceCluster",
    &sTests[0],
    nullptr,
    nullptr
};
// clang-format on

} // namespace

int TestPowerSource()
{
    return chip::ExecuteTestsWithoutContext(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPowerSource)
