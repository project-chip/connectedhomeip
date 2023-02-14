#include "ClusterTestContext.h"
#include "TestGroupCommandTranslatorHelper.h"
#include "group_command_translator.hpp"
#include "matter_device_translator.hpp"
// Matter components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Third party library
#include <nlunit-test.h>

// mocks
#include "MockGroupTranslator.hpp"
#include "MockNodeStateMonitor.hpp"
#include "MockUnifyMqtt.hpp"

//
#include <credentials/GroupDataProviderImpl.h>
#include <lib/support/TestGroupData.h>
#include <lib/support/TestPersistentStorageDelegate.h>

using namespace unify::matter_bridge;
using namespace chip::app;

#define TEST_LOG_TAG "TestGroupCommandTranslator"

constexpr uint16_t kMaxGroupsPerFabric    = 5;
constexpr uint16_t kMaxGroupKeysPerFabric = 8;

chip::TestPersistentStorageDelegate gTestStorage;
chip::Credentials::GroupDataProviderImpl gGroupsProvider(kMaxGroupsPerFabric, kMaxGroupKeysPerFabric);

using TestContext = Test::ClusterContext<GroupClusterAttributeTranslatorHelper, GroupClusterCommandHandler>;

static int Initialize(void * context)
{
    if (TestContext::Initialize(context) != SUCCESS)
        return FAILURE;

    auto * ctx         = static_cast<TestContext *>(context);
    auto & ep          = ctx->get_endpoint();
    auto & cluster     = ep.emplace_cluster("Groups");
    cluster.attributes = {
        "NameSupport",
    };
    cluster.supported_commands = {
        "AddGroup",
        "RemoveGroup",
        "RemoveAllGroups",
    };
    // Adding additional cluster support to make sure the test dynamic endpoint has device type
    auto & cluster_identify             = ep.emplace_cluster("Identify");
    cluster_identify.supported_commands = { "Identify" };
    cluster_identify.attributes         = { "IdentifyTime" };
    auto & cluster_scene                = ep.emplace_cluster("Scenes");
    cluster_scene.supported_commands    = { "AddScene" };
    cluster_scene.supported_commands    = { "SceneCount" };
    return ctx->register_endpoint(ep);
}

static void TestAddGroupCommand(nlTestSuite * inSuite, void * aContext)
{
    TestContext & ctx = *static_cast<TestContext *>(aContext);
    gTestStorage.ClearStorage();
    gGroupsProvider.SetStorageDelegate(&gTestStorage);
    gGroupsProvider.Init();
    chip::Credentials::SetGroupDataProvider(&gGroupsProvider);
    Clusters::Groups::Commands::AddGroup::Type request;
    request.groupID   = 1;
    request.groupName = chip::CharSpan::fromCharString("test_group_1");
    Clusters::Groups::Commands::AddGroupResponse::DecodableType response;
    CHIP_ERROR err = ctx.command_test<Clusters::Groups::Commands::AddGroup::Type>(
        inSuite, "ucl/by-unid/zw-0x0002/ep2/Groups/Commands/AddGroup", R"({"GroupId":1,"GroupName":"test_group_1"})", request,
        response);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void TestRemoveGroupCommand(nlTestSuite * inSuite, void * aContext)
{
    TestContext & ctx = *static_cast<TestContext *>(aContext);
    Clusters::Groups::Commands::RemoveGroup::Type request_remove;
    request_remove.groupID = 1;
    Clusters::Groups::Commands::RemoveGroupResponse::DecodableType response_remove;
    CHIP_ERROR err = ctx.command_test<Clusters::Groups::Commands::RemoveGroup::Type>(
        inSuite, "ucl/by-unid/zw-0x0002/ep2/Groups/Commands/RemoveGroup", R"({"GroupId":1})", request_remove, response_remove);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void TestRemoveAllGroupsCommand(nlTestSuite * inSuite, void * aContext)
{
    TestContext & ctx = *static_cast<TestContext *>(aContext);
    Clusters::Groups::Commands::AddGroup::Type request;
    request.groupID   = 2;
    request.groupName = chip::CharSpan::fromCharString("test_group_2");
    Clusters::Groups::Commands::AddGroupResponse::DecodableType response;
    CHIP_ERROR err = ctx.command_test<Clusters::Groups::Commands::AddGroup::Type>(
        inSuite, "ucl/by-unid/zw-0x0002/ep2/Groups/Commands/AddGroup", R"({"GroupId":1,"GroupName":"test_group_2"})", request,
        response);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    Clusters::Groups::Commands::RemoveAllGroups::Type request_remove;
    err = ctx.command_test<Clusters::Groups::Commands::RemoveAllGroups::Type>(
        inSuite, "ucl/by-unid/zw-0x0002/ep2/Groups/Commands/RemoveAllGroups", R"({})", request_remove);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF("Groups::TestAddGroupCommand", TestAddGroupCommand),
                                 NL_TEST_DEF("Groups::TestRemoveGroupCommand", TestRemoveGroupCommand),
                                 NL_TEST_DEF("Groups::TestRemoveAllGroupsCommand", TestRemoveAllGroupsCommand),
                                 NL_TEST_SENTINEL() };

static nlTestSuite kTheSuite = { "TestGroupCommandTranslator", &sTests[0], Initialize, TestContext::Finalize };

int TestGroupCommandTranslatorSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestGroupCommandTranslatorSuite)
