// Unify bridge components

// Chip components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Third party library
#include <nlunit-test.h>

#include "attribute_translator.hpp"
#include "matter_bridge_cli.hpp"
#include <uic_stdin.hpp>

// mocks
#include "MockGroupTranslator.hpp"
#include "MockNodeStateMonitor.hpp"

using namespace unify::matter_bridge;

static UnifyEmberInterface ember_interface = UnifyEmberInterface();
static device_translator dev_translator    = device_translator();

/* uic stdin mocks start */
static command_map_t commands;
// The prompt string for the command line interface

void uic_stdin_add_commands(
  const std::map<std::string, std::pair<std::string, handler_func>>
    &append_commands)
{
  commands.insert(append_commands.begin(), append_commands.end());
}
sl_status_t uic_stdin_handle_command(const char *command)
{
  std::string command_str(command);
  std::string cmd(command_str.substr(0, command_str.find(' ')));
  auto iter = commands.find(cmd);
  if (iter != commands.end()) {
    handle_args_t args;
    // add <command> as first arg
    args.push_back(cmd);
    // If there is a space after the command, look for "," seperated args
    // e.g. <command> <arg1>,<arg2>,<arg3>
    size_t pos = 0;
    if ((pos = command_str.find(' ')) != std::string::npos) {
      // erase "<command> " from the command_str.
      command_str.erase(0, pos + 1);
      const std::string delimiter(",");
      // look for "," seperated args in command_str
      while ((pos = command_str.find(delimiter)) != std::string::npos) {
        args.push_back(command_str.substr(0, pos));
        // erase <arg> from command_str
        command_str.erase(0, pos + delimiter.length());
      }
      // add last argument as well
      args.push_back(command_str);
    }
    return iter->second.second(args);
  }

  return SL_STATUS_FAIL;
}
/* uic stdin mocks end */
static void TestCliSetMatterNodeStateMonitor(nlTestSuite * inSuite, void * aContext)
{
    Test::MockNodeStateMonitor test_matter_node_state_monitor(dev_translator, ember_interface);
    matter_data_storage m_matter_data_storage;
    Test::MockGroupTranslator mGroupTranslator(m_matter_data_storage);
    set_mapping_display_instance(test_matter_node_state_monitor, mGroupTranslator);
    NL_TEST_ASSERT(inSuite, (uic_stdin_handle_command("epmap") == SL_STATUS_OK));
    NL_TEST_ASSERT(inSuite, (uic_stdin_handle_command("groups_map") == SL_STATUS_OK));
}


static void TestCliInit(nlTestSuite * inSuite, void * aContext)
{
    NL_TEST_ASSERT(inSuite, (matter_bridge_cli_init() == SL_STATUS_OK));
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
    NL_TEST_DEF("TestCliSetMatterNodeStateMonitor", TestCliSetMatterNodeStateMonitor),
    NL_TEST_SENTINEL()
};


// clang-format off
static nlTestSuite kTheSuite =
{
    "TestCliInterface",
    &sTests[0],
    nullptr,
    nullptr
};

int TestCliInterface(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCliInterface)
