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
#include <access/examples/PermissiveAccessControlDelegate.h>
#include <app/AttributeAccessInterface.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/MessageDef/EventDataIB.h>
#include <app/tests/AppTestContext.h>
#include <app/util/basic-types.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/CHIPCounter.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <nlunit-test.h>
#include <protocols/interaction_model/Constants.h>

#include <type_traits>

namespace {
using namespace chip;
using namespace chip::Access;

chip::ClusterId kTestClusterId        = 1;
chip::ClusterId kTestDeniedClusterId1 = 1000;
chip::ClusterId kTestDeniedClusterId2 = 3;
chip::EndpointId kTestEndpointId      = 4;

class TestAccessControlDelegate : public AccessControl::Delegate
{
public:
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const chip::Access::RequestPath & requestPath,
                     Privilege requestPrivilege) override
    {
        if (requestPath.cluster == kTestDeniedClusterId2)
        {
            return CHIP_ERROR_ACCESS_DENIED;
        }
        return CHIP_NO_ERROR;
    }
};

AccessControl::Delegate * GetTestAccessControlDelegate()
{
    static TestAccessControlDelegate accessControlDelegate;
    return &accessControlDelegate;
}

class TestDeviceTypeResolver : public AccessControl::DeviceTypeResolver
{
public:
    bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint) override { return false; }
} gDeviceTypeResolver;

class TestAccessContext : public chip::Test::AppContext
{
public:
    static int Initialize(void * context)
    {
        if (AppContext::Initialize(context) != SUCCESS)
            return FAILURE;
        Access::GetAccessControl().Finish();
        Access::GetAccessControl().Init(GetTestAccessControlDelegate(), gDeviceTypeResolver);
        return SUCCESS;
    }

    static int Finalize(void * context)
    {
        if (AppContext::Finalize(context) != SUCCESS)
            return FAILURE;
        return SUCCESS;
    }

private:
    chip::MonotonicallyIncreasingCounter<chip::EventNumber> mEventCounter;
};

class MockInteractionModelApp : public chip::app::ReadClient::Callback
{
public:
    void OnAttributeData(const chip::app::ConcreteDataAttributePath & aPath, chip::TLV::TLVReader * apData,
                         const chip::app::StatusIB & status) override
    {
        mGotReport          = true;
        mLastStatusReceived = status;
    }

    void OnError(CHIP_ERROR aError) override { mError = aError; }

    void OnDone(chip::app::ReadClient *) override {}

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override
    {
        if (aReadPrepareParams.mpAttributePathParamsList != nullptr)
        {
            delete[] aReadPrepareParams.mpAttributePathParamsList;
        }

        if (aReadPrepareParams.mpDataVersionFilterList != nullptr)
        {
            delete[] aReadPrepareParams.mpDataVersionFilterList;
        }
    }

    bool mGotReport = false;
    chip::app::StatusIB mLastStatusReceived;
    CHIP_ERROR mError = CHIP_NO_ERROR;
};
} // namespace

namespace chip {
namespace app {

bool ConcreteAttributePathExists(const ConcreteAttributePath & aPath)
{
    return aPath.mClusterId != kTestDeniedClusterId1;
}

class TestAclAttribute
{
public:
    static void TestACLDeniedAttribute(nlTestSuite * apSuite, void * apContext);
};

// Read Client sends a malformed subscribe request, interaction model engine fails to parse the request and generates a status
// report to client, and client is closed.
void TestAclAttribute::TestACLDeniedAttribute(nlTestSuite * apSuite, void * apContext)
{
    TestAccessContext & ctx = *static_cast<TestAccessContext *>(apContext);
    CHIP_ERROR err          = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        chip::app::AttributePathParams attributePathParams[2];
        attributePathParams[0].mEndpointId  = kTestEndpointId;
        attributePathParams[0].mClusterId   = kTestDeniedClusterId1;
        attributePathParams[0].mAttributeId = 1;

        attributePathParams[1].mEndpointId  = kTestEndpointId;
        attributePathParams[1].mClusterId   = kTestDeniedClusterId1;
        attributePathParams[1].mAttributeId = 2;

        ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
        readPrepareParams.mpAttributePathParamsList    = attributePathParams;
        readPrepareParams.mAttributePathParamsListSize = 2;

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, delegate.mError == CHIP_IM_GLOBAL_STATUS(InvalidAction));
        NL_TEST_ASSERT(apSuite, !delegate.mGotReport);
        delegate.mError     = CHIP_NO_ERROR;
        delegate.mGotReport = false;
    }

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        chip::app::AttributePathParams attributePathParams[2];

        attributePathParams[0].mClusterId   = kTestDeniedClusterId2;
        attributePathParams[0].mAttributeId = 1;

        attributePathParams[1].mClusterId   = kTestDeniedClusterId2;
        attributePathParams[1].mAttributeId = 2;

        ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
        readPrepareParams.mpAttributePathParamsList    = attributePathParams;
        readPrepareParams.mAttributePathParamsListSize = 2;

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, delegate.mError == CHIP_IM_GLOBAL_STATUS(InvalidAction));
        NL_TEST_ASSERT(apSuite, !delegate.mGotReport);
        delegate.mError     = CHIP_NO_ERROR;
        delegate.mGotReport = false;
    }

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        chip::app::AttributePathParams attributePathParams[2];
        attributePathParams[0].mEndpointId  = kTestEndpointId;
        attributePathParams[0].mClusterId   = kTestDeniedClusterId1;
        attributePathParams[0].mAttributeId = 1;

        attributePathParams[1].mEndpointId  = kTestEndpointId;
        attributePathParams[1].mClusterId   = kTestClusterId;
        attributePathParams[1].mAttributeId = 2;

        ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
        readPrepareParams.mpAttributePathParamsList    = attributePathParams;
        readPrepareParams.mAttributePathParamsListSize = 2;

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, delegate.mError == CHIP_NO_ERROR);
        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);
        delegate.mError     = CHIP_NO_ERROR;
        delegate.mGotReport = false;
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
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
    NL_TEST_DEF("TestACLDeniedAttribute", chip::app::TestAclAttribute::TestACLDeniedAttribute),
    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestAclAttribute",
    &sTests[0],
    TestAccessContext::Initialize,
    TestAccessContext::Finalize
};
// clang-format on

} // namespace

int TestAclAttribute()
{
    return chip::ExecuteTestsWithContext<TestAccessContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestAclAttribute)
