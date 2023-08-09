// Unify bridge components

// Chip components
#include <AppMain.h>
#include <app/server/Server.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Third party library
#include <nlunit-test.h>

#include <filesystem>

#include "UnifyMPCContext.h"
#include "mpc_cli_mock.h"
#include <uic_stdin.hpp>

using namespace chip;

int Initialize(void * context)
{
    const char * __argv__[] = { "matter_bridge", nullptr };
    int __argc__            = sizeof(__argv__) / sizeof(const char *) - 1;

    // Makes sure our required state folders exists
    std::filesystem::create_directories(FATCONFDIR);
    std::filesystem::create_directories(SYSCONFDIR);
    std::filesystem::create_directories(LOCALSTATEDIR);

    ChipLinuxAppInit(__argc__, const_cast<char **>(__argv__));

    CommonCaseDeviceServerInitParams params;
    params.InitializeStaticResourcesBeforeServerInit();
    params.operationalServicePort        = CHIP_PORT;
    params.userDirectedCommissioningPort = CHIP_UDC_PORT;
    Server::GetInstance().Init(params);
    return 0;
}
int Finalize(void * contxt)
{
    Server::GetInstance().Shutdown();
    DeviceLayer::PlatformMgr().Shutdown();
    return 0;
}

static void TestCliMatterCommission(nlTestSuite * inSuite, void * aContext)
{

    NL_TEST_ASSERT(inSuite, (mpc_stdin_handle_command("commission") == SL_STATUS_OK));
    NL_TEST_ASSERT(inSuite, (Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen()));
}

static void TestCliInit(nlTestSuite * inSuite, void * aContext)
{
    NL_TEST_ASSERT(inSuite, (mpc_cli_init() == SL_STATUS_OK));
}

class TestContext
{
public:
    nlTestSuite * mTestSuite;
    uint32_t mNumTimersHandled;
};

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestCli",             TestCliInit),
    NL_TEST_DEF("TestCliMatterCommission", TestCliMatterCommission),
    NL_TEST_SENTINEL()
};


// clang-format off
static nlTestSuite kTheSuite =
{
    "TestCliInterface",
    &sTests[0],
    Initialize,
    Finalize
};

int TestCliInterface(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCliInterface)
