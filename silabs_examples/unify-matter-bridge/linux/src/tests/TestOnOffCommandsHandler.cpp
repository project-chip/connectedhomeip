// Unify bridge components
#include "command_translator.hpp"
#include "matter_device_translator.hpp"
#include "matter_node_state_monitor.hpp"

// Chip components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Third party library
#include <iostream>
#include <string>
#include <nlunit-test.h>

using namespace unify::matter_bridge;

static UnifyEmberInterface ember_interface = UnifyEmberInterface();

class MockMatterNodeStateMonitor : public matter_node_state_monitor
{
public:
    MockMatterNodeStateMonitor() : matter_node_state_monitor(device_translator(), ember_interface), test_bridge_ep(matter_endpoint_context()) {}

    const struct bridged_endpoint * bridged_endpoint(chip::EndpointId endpoint) const override
    {
        ++nNumBridgedMethodCall;
        if (endpoint >= 2)
        {
            test_bridge_ep.matter_endpoint = endpoint;
            test_bridge_ep.unify_endpoint = 4;
            test_bridge_ep.unify_unid = "zw-0x0002";
            return &test_bridge_ep;
        }
        else
        {
            return nullptr;
        }
    }

    mutable int nNumBridgedMethodCall = 0;
    mutable struct bridged_endpoint test_bridge_ep;
};

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

class MockUicMqttPublish : public UicMqtt
{
public:
    void Publish(std::string topic, std::string message, bool retain) override
    {
        publish_topic = topic;
        ++nNumerUicMqttPublishCall;
    }
    int nNumerUicMqttPublishCall = 0;
    std::string publish_topic;
};

void TestOnOffClusterCommandHandler(nlTestSuite * inSuite, void * aContext)
{
    MockMatterNodeStateMonitor test_matter_node_state_monitor;
    MockUicMqttPublish mqtt_publish_test;
    OnOffClusterCommandHandler on_cmd_handler(test_matter_node_state_monitor, mqtt_publish_test);

    chip::EndpointId endpoint = 2;
    chip::app::ConcreteCommandPath test_command_path =
        chip::app::ConcreteCommandPath(endpoint, chip::app::Clusters::OnOff::Id, chip::app::Clusters::OnOff::Commands::On::Id);
    MockCommandHandlerCallback mockCommandHandlerDelegate;
    chip::app::CommandHandler test_commandHandler(&mockCommandHandlerDelegate);
    chip::TLV::TLVReader test_tlv_reader;
    chip::app::CommandHandlerInterface::HandlerContext ctxt =
        chip::app::CommandHandlerInterface::HandlerContext(test_commandHandler, test_command_path, test_tlv_reader);

    on_cmd_handler.InvokeCommand(ctxt);

    NL_TEST_ASSERT(inSuite, test_matter_node_state_monitor.nNumBridgedMethodCall == 2);
    NL_TEST_ASSERT(inSuite, mqtt_publish_test.nNumerUicMqttPublishCall == 1);
    NL_TEST_ASSERT(inSuite, mqtt_publish_test.publish_topic == "ucl/by-unid/zw-0x0002/ep4/OnOff/Commands/On" );

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