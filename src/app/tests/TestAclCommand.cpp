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

namespace {
using namespace chip;
using namespace chip::Access;
using namespace chip::Test;

constexpr CommandId kTestUnsupportedCommandId = 77;
constexpr CommandId kTestCommandId            = 4;

const MockNodeConfig & TestMockNodeConfig()
{
    using namespace chip::app;
    using namespace chip::app::Clusters::Globals::Attributes;
    // clang-format off
    static const MockNodeConfig config({
        MockEndpointConfig(kTestEndpointId, {
            MockClusterConfig(kTestClusterId, {
                ClusterRevision::Id, FeatureMap::Id, 1, 2
            }),
        }),
        MockEndpointConfig(kTestDeniedEndpointId, {
            MockClusterConfig(kTestClusterId, {
                ClusterRevision::Id, FeatureMap::Id, 1, 2
            }),
        }),
    });
    // clang-format on
    return config;
}

class TestAccessControlDelegate : public AccessControl::Delegate
{
public:
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const chip::Access::RequestPath & requestPath,
                     Privilege requestPrivilege) override
    {
        if (requestPath.cluster == chip::Test::kTestDeniedClusterId2 || requestPath.endpoint == chip::Test::kTestDeniedEndpointId)
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

class MockCommandSenderCallback : public app::CommandSender::Callback
{
public:
    void OnError(const app::CommandSender * apCommandSender, CHIP_ERROR aError) override
    {
        ChipLogError(Controller, "OnError happens with %" CHIP_ERROR_FORMAT, aError.Format());
        mError = aError;
        onErrorCalledTimes++;
        mError = aError;
    }
    void OnDone(app::CommandSender * apCommandSender) override { onFinalCalledTimes++; }

    void ResetCounter()
    {
        onErrorCalledTimes = 0;
        onFinalCalledTimes = 0;
    }

    int onErrorCalledTimes = 0;
    int onFinalCalledTimes = 0;
    CHIP_ERROR mError      = CHIP_NO_ERROR;
} mockCommandSenderDelegate;

} // namespace

namespace chip {
namespace app {

class TestAclCommand : public Test::AppContext
{
public:
    void SetUp() override
    {
        AppContext::SetUp();

        Access::GetAccessControl().Finish();
        Access::GetAccessControl().Init(GetTestAccessControlDelegate(), gDeviceTypeResolver);
        mOldProvider = InteractionModelEngine::GetInstance()->SetDataModelProvider(&TestImCustomDataModel::Instance());
        chip::Test::SetMockNodeConfig(TestMockNodeConfig());
    }

    void TearDown() override
    {
        chip::Test::ResetMockNodeConfig();
        AppContext::TearDown();
        InteractionModelEngine::GetInstance()->SetDataModelProvider(mOldProvider);
    }

    static void AddInvokeRequestData(CommandSender * apCommandSender, CommandPathParams aCommandPathParams);

private:
    chip::app::DataModel::Provider * mOldProvider = nullptr;
};

void TestAclCommand::AddInvokeRequestData(CommandSender * apCommandSender, CommandPathParams aCommandPathParams)
{

    EXPECT_EQ(apCommandSender->PrepareCommand(aCommandPathParams), CHIP_NO_ERROR);

    chip::TLV::TLVWriter * writer = apCommandSender->GetCommandDataIBTLVWriter();

    EXPECT_EQ(writer->PutBoolean(chip::TLV::ContextTag(1), true), CHIP_NO_ERROR);
    EXPECT_EQ(apCommandSender->FinishCommand(), CHIP_NO_ERROR);
}

// If an endpoint isn't supported, but access to the request path is denied, we should receive UnsupportedAccess rather than
// UnsupportedEndpoint to prevent information leakage
TEST_F(TestAclCommand, AccessDeniedPrecedenceOverUnsupportedEndpoint)
{

    {
        mockCommandSenderDelegate.ResetCounter();
        app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

        auto commandPathParams = CommandPathParams(kTestUnsupportedEndpointId, 0, kTestDeniedClusterId2, kTestCommandId,
                                                   (chip::app::CommandPathFlags::kEndpointIdValid));
        AddInvokeRequestData(&commandSender, commandPathParams);
        EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
        EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 0u);

        DrainAndServiceIO();

        EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_IM_GLOBAL_STATUS(UnsupportedAccess));

        EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
        EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);

        EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 1u);
        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    {
        mockCommandSenderDelegate.ResetCounter();
        app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

        auto commandPathParams = CommandPathParams(kTestUnsupportedEndpointId, 0, kTestClusterId, kTestCommandId,
                                                   (chip::app::CommandPathFlags::kEndpointIdValid));
        AddInvokeRequestData(&commandSender, commandPathParams);
        EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
        EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 0u);

        DrainAndServiceIO();

        EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint));

        EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
        EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);

        EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 1u);
        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }
}

// If a Cluster isn't supported, but access to the request path is denied, we should receive UnsupportedAccess rather than
// UnsupportedCluster to prevent information leakage
TEST_F(TestAclCommand, AccessDeniedPrecedenceOverUnsupportedCluster)
{

    {
        mockCommandSenderDelegate.ResetCounter();
        app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

        auto commandPathParams = CommandPathParams(kTestDeniedEndpointId, 0, kTestUnsupportedClusterId, kTestCommandId,
                                                   (chip::app::CommandPathFlags::kEndpointIdValid));
        AddInvokeRequestData(&commandSender, commandPathParams);
        EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
        EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 0u);

        DrainAndServiceIO();

        EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_IM_GLOBAL_STATUS(UnsupportedAccess));

        EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
        EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);

        EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 1u);
        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    {
        mockCommandSenderDelegate.ResetCounter();
        app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

        auto commandPathParams = CommandPathParams(kTestEndpointId, 0, kTestUnsupportedClusterId, kTestCommandId,
                                                   (chip::app::CommandPathFlags::kEndpointIdValid));
        AddInvokeRequestData(&commandSender, commandPathParams);
        EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
        EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 0u);

        DrainAndServiceIO();

        EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_IM_GLOBAL_STATUS(UnsupportedCluster));

        EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
        EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);

        EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 1u);
        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }
}
// If a Command isn't supported, but access to the request path is denied, we should receive UnsupportedAccess rather than
// UnsupportedCommand to prevent information leakage
TEST_F(TestAclCommand, AccessDeniedPrecedenceOverUnsupportedCommand)
{

    {
        mockCommandSenderDelegate.ResetCounter();
        app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

        auto commandPathParams = CommandPathParams(kTestDeniedEndpointId, 0, kTestClusterId, kTestUnsupportedCommandId,
                                                   (chip::app::CommandPathFlags::kEndpointIdValid));
        AddInvokeRequestData(&commandSender, commandPathParams);
        EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
        EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 0u);

        DrainAndServiceIO();

        EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_IM_GLOBAL_STATUS(UnsupportedAccess));

        EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
        EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);

        EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 1u);
        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    {
        mockCommandSenderDelegate.ResetCounter();
        app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

        auto commandPathParams = CommandPathParams(kTestEndpointId, 0, kTestClusterId, kTestUnsupportedCommandId,
                                                   (chip::app::CommandPathFlags::kEndpointIdValid));
        AddInvokeRequestData(&commandSender, commandPathParams);
        EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
        EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 0u);

        DrainAndServiceIO();

        EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_IM_GLOBAL_STATUS(UnsupportedCommand));

        EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
        EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);

        EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 1u);
        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }
}

} // namespace app
} // namespace chip
