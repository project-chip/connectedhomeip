// Unify bridge components
#include "command_translator.hpp"
#include "matter_device_translator.hpp"

// Chip components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Mocks
#include "MockMatterDataStorage.hpp"
#include "MockNodeStateMonitor.hpp"
#include "MockUnifyMqtt.hpp"

// Third party library
#include <iostream>
#include <nlunit-test.h>
#include <string>

using namespace unify::matter_bridge;

static UnifyEmberInterface ember_interface = UnifyEmberInterface();
static device_translator dev_translator    = device_translator();
class MockCommandHandlerCallback : public chip::app::CommandHandler::Callback
{
public:
    void OnDone(chip::app::CommandHandler & apCommandHandler) final { onFinalCalledTimes++; }
    void DispatchCommand(chip::app::CommandHandler & apCommandObj, const chip::app::ConcreteCommandPath & aCommandPath,
                         chip::TLV::TLVReader & apPayload) final
    {}
    chip::Protocols::InteractionModel::Status CommandExists(const chip::app::ConcreteCommandPath & aCommandPath)
    {
        return chip::Protocols::InteractionModel::Status::Success;
    }

    int onFinalCalledTimes = 0;
};

void TestOnOffClusterCommandHandler(nlTestSuite * inSuite, void * aContext)
{
    // 1
    // Execute command ON with onoff device
    Test::MockNodeStateMonitor test_matter_node_state_monitor(dev_translator, ember_interface);
    Test::MockUnifyMqtt mqtt_publish_test;
    Test::MockMatterDataStorage mock_data_storage;
    group_translator test_group_translator(mock_data_storage);
    OnOffClusterCommandHandler on_cmd_handler(test_matter_node_state_monitor, mqtt_publish_test, test_group_translator);

    // Create a node with cluster endpoints resembling On/Off.
    const std::string node_id = "zw-0x0002";
    const uint8_t endpoint    = 4;
    unify::node_state_monitor::node node_ember_1(node_id);
    auto & ep                        = node_ember_1.emplace_endpoint(endpoint);
    auto & onoff_cluster             = ep.emplace_cluster("OnOff");
    onoff_cluster.supported_commands = { "On", "Off", "Toggle" };
    onoff_cluster.attributes.emplace("OnOff");

    test_matter_node_state_monitor.call_on_unify_node_added(node_ember_1);
    auto bridged_ep = test_matter_node_state_monitor.bridged_endpoint(node_id, endpoint);
    NL_TEST_ASSERT(inSuite, bridged_ep != nullptr);

    chip::app::ConcreteCommandPath test_command_path = chip::app::ConcreteCommandPath(
        bridged_ep->matter_endpoint, chip::app::Clusters::OnOff::Id, chip::app::Clusters::OnOff::Commands::On::Id);
    MockCommandHandlerCallback mockCommandHandlerDelegate;
    chip::app::CommandHandler test_commandHandler(&mockCommandHandlerDelegate);
    chip::TLV::TLVReader test_tlv_reader;
    chip::app::CommandHandlerInterface::HandlerContext ctxt =
        chip::app::CommandHandlerInterface::HandlerContext(test_commandHandler, test_command_path, test_tlv_reader);

    on_cmd_handler.InvokeCommand(ctxt);

    NL_TEST_ASSERT(inSuite, mqtt_publish_test.nNumerUicMqttPublishCall == 1);
    NL_TEST_ASSERT(inSuite, mqtt_publish_test.publish_topic == "ucl/by-unid/zw-0x0002/ep4/OnOff/Commands/On");
    NL_TEST_ASSERT(inSuite, mqtt_publish_test.publish_payload.compare("{}") == 0);
}

class TestContext
{
public:
    nlTestSuite * mTestSuite;
};

static const nlTest sTests[] = { NL_TEST_DEF("TestOnOffClusterCommandHandler", TestOnOffClusterCommandHandler),
                                 NL_TEST_SENTINEL() };

static nlTestSuite TheCommandSuite = { "TestOnOffCommandsHandler", &sTests[0], nullptr, nullptr };

int TestOnOffCommandsHandler(void)
{
    return (chip::ExecuteTestsWithContext<TestContext>(&TheCommandSuite));
}

CHIP_REGISTER_TEST_SUITE(TestOnOffCommandsHandler)
