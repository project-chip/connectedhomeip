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
#include <app/util/af.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <nlunit-test.h>
#include <protocols/interaction_model/Constants.h>
#include <type_traits>

#include <vector>

namespace {
chip::EndpointId numEndpoints = 0;
}
extern uint16_t emberAfGetClusterServerEndpointIndex(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                     uint16_t fixedClusterServerEndpointCount)
{
    // Very simple mapping here, we're just going to return the endpoint that matches the given endpoint index because the test
    // uses the endpoints in order.
    if (endpoint >= numEndpoints)
    {
        return kEmberInvalidEndpointIndex;
    }
    return endpoint;
}

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

    // Buffer setup
    constexpr size_t buflen = 128;
    uint8_t buf[buflen];
    memset(buf, 0, buflen);

    // Create the builders
    TLV::TLVWriter tlvWriter;
    tlvWriter.Init(buf);

    AttributeReportIBs::Builder builder;
    builder.Init(&tlvWriter);

    ConcreteAttributePath path(endpoint, Clusters::PowerSource::Id, Clusters::PowerSource::Attributes::EndpointList::Id);
    ConcreteReadAttributePath readPath(path);
    chip::DataVersion dataVersion(0);
    AttributeValueEncoder aEncoder(builder, 0, path, dataVersion);

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

    // We're now in the attribute data IB, skip to the desired tag, we want TagNum = 2
    attrDataReader.Next();
    for (int i = 0; i < 3 && !(IsContextTag(attrDataReader.GetTag()) && TagNumFromTag(attrDataReader.GetTag()) == 2); ++i)
    {
        attrDataReader.Next();
    }
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

    if (powerSourceServer.GetNumSupportedEndpointLists() < 2 ||
        powerSourceServer.GetNumSupportedEndpointLists() > std::numeric_limits<uint16_t>::max())
    {
        // Test assumes at least two endpoints. This runs on linux, not worthwhile to run on platforms with fewer endpoints.
        return;
    }

    // *****************
    // Test setting, getting and reading through the attribute access interface
    // *****************
    EndpointId list0[5]    = { 1, 2, 3, 4, 5 };
    EndpointId list1[10]   = { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    EndpointId listRest[1] = { 2 };

    // we checked earlier that this fit
    // This test just uses endpoints in order, so we want to set endpoints from
    // 0 to numEndpoints - 1, and use this for overflow checking
    numEndpoints = static_cast<EndpointId>(powerSourceServer.GetNumSupportedEndpointLists());

    // Endpoint 0 - list of 5
    err = powerSourceServer.SetEndpointList(0, Span<EndpointId>(list0));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    const Span<EndpointId> * readBack = powerSourceServer.GetEndpointList(0);
    NL_TEST_EXIT_ON_FAILED_ASSERT(apSuite, readBack != nullptr);
    NL_TEST_ASSERT(apSuite, readBack->size() == 5);
    for (size_t i = 0; i < readBack->size(); ++i)
    {
        NL_TEST_ASSERT(apSuite, readBack->data()[i] == list0[i]);
    }

    // Endpoint 1 - list of 10
    err = powerSourceServer.SetEndpointList(1, Span<EndpointId>(list1));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    readBack = powerSourceServer.GetEndpointList(1);
    NL_TEST_EXIT_ON_FAILED_ASSERT(apSuite, readBack != nullptr);
    NL_TEST_ASSERT(apSuite, readBack->size() == 10);
    for (size_t i = 0; i < readBack->size(); ++i)
    {
        NL_TEST_ASSERT(apSuite, readBack->data()[i] == list1[i]);
    }

    // Remaining endpoints - list of 1
    for (EndpointId ep = 2; ep < numEndpoints; ++ep)
    {
        err = powerSourceServer.SetEndpointList(ep, Span<EndpointId>(listRest));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        readBack = powerSourceServer.GetEndpointList(ep);
        NL_TEST_EXIT_ON_FAILED_ASSERT(apSuite, readBack != nullptr);
        NL_TEST_ASSERT(apSuite, readBack->size() == 1);
        if (readBack->size() == 1)
        {
            NL_TEST_ASSERT(apSuite, readBack->data()[0] == listRest[0]);
        }
    }

    // *****************
    // Check for out of memory error when setting too many endpoints
    // *****************
    // pick a random endpoint number for the power cluster - it doesn't matter, we don't have space anyway.
    err = powerSourceServer.SetEndpointList(55, Span<EndpointId>(listRest));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NO_MEMORY);

    // *****************
    // Recheck getting and reading after OOM
    // *****************
    // EP0
    readBack = powerSourceServer.GetEndpointList(0);
    NL_TEST_EXIT_ON_FAILED_ASSERT(apSuite, readBack != nullptr);
    NL_TEST_ASSERT(apSuite, readBack->size() == 5);
    for (size_t i = 0; i < readBack->size(); ++i)
    {
        NL_TEST_ASSERT(apSuite, readBack->data()[i] == list0[i]);
    }

    // EP1
    readBack = powerSourceServer.GetEndpointList(1);
    NL_TEST_EXIT_ON_FAILED_ASSERT(apSuite, readBack != nullptr);
    NL_TEST_ASSERT(apSuite, readBack->size() == 10);
    for (size_t i = 0; i < readBack->size(); ++i)
    {
        NL_TEST_ASSERT(apSuite, readBack->data()[i] == list1[i]);
    }

    // Remaining endpoints
    for (EndpointId ep = 2; ep < numEndpoints; ++ep)
    {
        readBack = powerSourceServer.GetEndpointList(ep);
        NL_TEST_EXIT_ON_FAILED_ASSERT(apSuite, readBack != nullptr);
        NL_TEST_ASSERT(apSuite, readBack->size() == 1);
        if (readBack->size() == 1)
        {
            NL_TEST_ASSERT(apSuite, readBack->data()[0] == listRest[0]);
        }
    }

    // *****************
    // Test overwriting
    // *****************
    // Overwrite a list
    err = powerSourceServer.SetEndpointList(1, Span<EndpointId>(listRest));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    readBack = powerSourceServer.GetEndpointList(1);
    NL_TEST_ASSERT(apSuite, readBack->size() == 1);
    if (readBack->size() == 1)
    {
        NL_TEST_ASSERT(apSuite, readBack->data()[0] == listRest[0]);
    }

    // Ensure only the overwritten list was changed, using read interface
    for (EndpointId ep = 0; ep < numEndpoints + 1; ++ep)
    {
        std::vector<EndpointId> vec = ReadEndpointsThroughAttributeReader(apSuite, ep);
        if (ep == 0)
        {
            NL_TEST_ASSERT(apSuite, vec.size() == 5);
            for (size_t j = 0; j < vec.size(); ++j)
            {
                NL_TEST_ASSERT(apSuite, vec[j] == list0[j]);
            }
        }
        else if (ep == numEndpoints)
        {
            NL_TEST_ASSERT(apSuite, vec.size() == 0);
        }
        else
        {
            NL_TEST_ASSERT(apSuite, vec.size() == 1);
            if (vec.size() == 1)
            {
                NL_TEST_ASSERT(apSuite, vec[0] == listRest[0]);
            }
        }
    }

    // *****************
    // Test removal
    // *****************
    for (EndpointId ep = 0; ep < numEndpoints; ++ep)
    {
        err = powerSourceServer.SetEndpointList(ep, Span<EndpointId>());
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        readBack = powerSourceServer.GetEndpointList(ep);
        NL_TEST_ASSERT(apSuite, readBack == nullptr);
    }

    // Check through the read interface
    for (EndpointId ep = 0; ep < numEndpoints + 1; ++ep)
    {
        std::vector<EndpointId> vec = ReadEndpointsThroughAttributeReader(apSuite, ep);
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

/**
 *  Set up the test suite.
 */
int TestPowerSourceClusterContext_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestPowerSourceClusterContext_Teardown(void * inContext)
{
    chip::app::Clusters::PowerSourceServer::Instance().Shutdown();
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

// clang-format off
nlTestSuite sSuite =
{
    "TestPowerSourceCluster",
    &sTests[0],
    TestPowerSourceClusterContext_Setup,
    TestPowerSourceClusterContext_Teardown
};
// clang-format on

} // namespace

int TestPowerSource()
{
    nlTestRunner(&sSuite, nullptr);
    return nlTestRunnerStats(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPowerSource)
