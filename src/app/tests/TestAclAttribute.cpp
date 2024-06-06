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

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

#include <access/examples/PermissiveAccessControlDelegate.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteEventPath.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/MessageDef/EventDataIB.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <app/tests/AppTestContext.h>
#include <app/tests/test-interaction-model-api.h>
#include <app/util/basic-types.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CHIPCounter.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <protocols/interaction_model/Constants.h>

#include <type_traits>

namespace {
using namespace chip;
using namespace chip::Access;

class TestAccessControlDelegate : public AccessControl::Delegate
{
public:
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const chip::Access::RequestPath & requestPath,
                     Privilege requestPrivilege) override
    {
        if (requestPath.cluster == chip::Test::kTestDeniedClusterId2)
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

class TestAclAttribute : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {

        mpTestContext = new chip::Test::AppContext;

        mpTestContext->SetUpTestSuite();
    }
    static void TearDownTestSuite()
    {
        mpTestContext->TearDownTestSuite();
        if (mpTestContext != nullptr)
        {
            delete mpTestContext;
        }
    }

    void SetUp() override
    {

        if (mpTestContext != nullptr)
        {
            mpTestContext->SetUp();

            Access::GetAccessControl().Finish();
            Access::GetAccessControl().Init(GetTestAccessControlDelegate(), gDeviceTypeResolver);
        }
    }
    void TearDown() override
    {
        if (mpTestContext != nullptr)
        {
            mpTestContext->TearDown();
        }
    }
    static chip::Test::AppContext * mpTestContext;
};

chip::Test::AppContext * TestAclAttribute::mpTestContext = nullptr;

// Read Client sends a malformed subscribe request, interaction model engine fails to parse the request and generates a status
// report to client, and client is closed.
TEST_F(TestAclAttribute, TestACLDeniedAttribute)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = mpTestContext->GetExchangeManager().GetReliableMessageMgr();
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&mpTestContext->GetExchangeManager(), &mpTestContext->GetFabricTable(),
                                 app::reporting::GetDefaultReportScheduler());
    EXPECT_EQ(err, CHIP_NO_ERROR);

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &mpTestContext->GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        chip::app::AttributePathParams attributePathParams[2];
        attributePathParams[0].mEndpointId  = chip::Test::kTestEndpointId;
        attributePathParams[0].mClusterId   = chip::Test::kTestDeniedClusterId1;
        attributePathParams[0].mAttributeId = 1;

        attributePathParams[1].mEndpointId  = chip::Test::kTestEndpointId;
        attributePathParams[1].mClusterId   = chip::Test::kTestDeniedClusterId1;
        attributePathParams[1].mAttributeId = 2;

        ReadPrepareParams readPrepareParams(mpTestContext->GetSessionBobToAlice());
        readPrepareParams.mpAttributePathParamsList    = attributePathParams;
        readPrepareParams.mAttributePathParamsListSize = 2;

        err = readClient.SendRequest(readPrepareParams);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        mpTestContext->DrainAndServiceIO();
        EXPECT_EQ(delegate.mError, CHIP_IM_GLOBAL_STATUS(InvalidAction));
        EXPECT_FALSE(delegate.mGotReport);
        delegate.mError     = CHIP_NO_ERROR;
        delegate.mGotReport = false;
    }

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &mpTestContext->GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        chip::app::AttributePathParams attributePathParams[2];

        attributePathParams[0].mClusterId   = chip::Test::kTestDeniedClusterId2;
        attributePathParams[0].mAttributeId = 1;

        attributePathParams[1].mClusterId   = chip::Test::kTestDeniedClusterId2;
        attributePathParams[1].mAttributeId = 2;

        ReadPrepareParams readPrepareParams(mpTestContext->GetSessionBobToAlice());
        readPrepareParams.mpAttributePathParamsList    = attributePathParams;
        readPrepareParams.mAttributePathParamsListSize = 2;

        err = readClient.SendRequest(readPrepareParams);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        mpTestContext->DrainAndServiceIO();
        EXPECT_EQ(delegate.mError, CHIP_IM_GLOBAL_STATUS(InvalidAction));
        EXPECT_FALSE(delegate.mGotReport);
        delegate.mError     = CHIP_NO_ERROR;
        delegate.mGotReport = false;
    }

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &mpTestContext->GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        chip::app::AttributePathParams attributePathParams[2];
        attributePathParams[0].mEndpointId  = chip::Test::kTestEndpointId;
        attributePathParams[0].mClusterId   = chip::Test::kTestDeniedClusterId1;
        attributePathParams[0].mAttributeId = 1;

        attributePathParams[1].mEndpointId  = chip::Test::kTestEndpointId;
        attributePathParams[1].mClusterId   = chip::Test::kTestClusterId;
        attributePathParams[1].mAttributeId = 2;

        ReadPrepareParams readPrepareParams(mpTestContext->GetSessionBobToAlice());
        readPrepareParams.mpAttributePathParamsList    = attributePathParams;
        readPrepareParams.mAttributePathParamsListSize = 2;

        err = readClient.SendRequest(readPrepareParams);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        mpTestContext->DrainAndServiceIO();
        EXPECT_EQ(delegate.mError, CHIP_NO_ERROR);
        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);
        delegate.mError     = CHIP_NO_ERROR;
        delegate.mGotReport = false;
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(mpTestContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}
} // namespace app
} // namespace chip
