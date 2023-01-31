// Unify bridge components
#include "attribute_translator.hpp"
#include "matter_device_translator.hpp"
#include "unify_accessors.hpp"

// Unify SDK components
#include "sl_log.h"

// Chip components
#include <app/AttributeAccessInterface.h>
#include <app/MessageDef/AttributeDataIB.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Mocks
#include "MockNodeStateMonitor.hpp"
#include "MockUnifyMqtt.hpp"
// Third party library
#include <iostream>
#include <nlunit-test.h>
#include <string>

using namespace unify::matter_bridge;

#define TEST_LOG_TAG "NodeStateMonitorTest"

static UnifyEmberInterface ember_interface = UnifyEmberInterface();
static device_translator dev_translator    = device_translator(false);
static ClusterEmulator emulator            = ClusterEmulator();

chip::app::AttributeValueEncoder setupEncoder(chip::EndpointId endpoint, chip::app::ConcreteAttributePath & path,
                                              chip::DataVersion & dataVersion)
{
    const chip::app::AttributeValueEncoder::AttributeEncodeState & aState =
        chip::app::AttributeValueEncoder::AttributeEncodeState();
    chip::app::AttributeReportIBs::Builder builder;
    chip::FabricIndex fabricIndex = static_cast<uint8_t>(endpoint);
    chip::app::AttributeValueEncoder encoder(builder, fabricIndex, path, dataVersion, true, aState);

    return encoder;
}

void TestTemperatureMeasurementAttributes(nlTestSuite * inSuite, void * aContext)
{

    // 1
    Test::MockNodeStateMonitor test_matter_node_state_monitor(dev_translator, emulator, ember_interface);
    Test::MockUnifyMqtt test_unify_mqtt;

    unify::matter_bridge::TemperatureMeasurementAttributeAccess test_temperature_measurement_attributes(
        test_matter_node_state_monitor, test_unify_mqtt, dev_translator);

    NL_TEST_ASSERT(inSuite, test_unify_mqtt.nNumerUicMqttSubscribeCall == 0);
    NL_TEST_ASSERT(inSuite, test_unify_mqtt.subscribe_topic == "");

    // 2
    // Setup attribute listening for the temperature measurement cluster

    // Setup unify TemperatureMeasurement device/cluster on the matter node state monitor
    const std::string node_id = "temperature-measurement-id-1";
    unify::node_state_monitor::node node_temp_1(node_id);
    uint16_t endpoint   = 1;
    auto & ep           = node_temp_1.emplace_endpoint(endpoint);
    auto & temp_cluster = ep.emplace_cluster("TemperatureMeasurement");
    temp_cluster.attributes.emplace("MeasuredValue");
    test_matter_node_state_monitor.call_on_unify_node_added(node_temp_1);

    NL_TEST_ASSERT(inSuite, test_unify_mqtt.nNumerUicMqttSubscribeCall == 1);
    NL_TEST_ASSERT(
        inSuite, test_unify_mqtt.subscribe_topic == "ucl/by-unid/" + node_id + "/ep1/TemperatureMeasurement/Attributes/+/Reported");

    // 3
    // Test sending a report to the measured value attribute and expect it to be written to the matter fabric
    std::string measured_value_topic = "ucl/by-unid/" + node_id + "/ep1/TemperatureMeasurement/Attributes/MeasuredValue/Reported";
    std::string payload              = R"({ "value": 55 })";
    test_unify_mqtt.subscribeCB(measured_value_topic.c_str(), payload.c_str(), payload.size(),
                                &test_temperature_measurement_attributes);

    // Read the attribute value
    auto unify_node = test_matter_node_state_monitor.bridged_endpoint(node_id, endpoint);

    auto test_correct_attribute_id_path =
        chip::app::ConcreteAttributePath(unify_node->matter_endpoint, chip::app::Clusters::TemperatureMeasurement::Id,
                                         chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Id);
    chip::app::DataModel::Nullable<int16_t> test_attribute_value;
    auto result = unify::matter_bridge::TemperatureMeasurement::Attributes::MeasuredValue::Get(test_correct_attribute_id_path,
                                                                                               test_attribute_value);

    NL_TEST_ASSERT(inSuite, result == EMBER_ZCL_STATUS_SUCCESS);
    NL_TEST_ASSERT(inSuite, test_attribute_value.Value() == 55);
}

void TestTemperatureMeasurementReadFailures(nlTestSuite * inSuite, void * aContext)
{

    // Setup
    Test::MockNodeStateMonitor test_matter_node_state_monitor(dev_translator, emulator, ember_interface);
    Test::MockUnifyMqtt test_unify_mqtt;

    unify::matter_bridge::TemperatureMeasurementAttributeAccess test_temperature_measurement_attributes(
        test_matter_node_state_monitor, test_unify_mqtt, dev_translator);

    // 1
    // Reading Incorrect cluster ID attribute
    chip::EndpointId endpoint         = 2;
    auto test_wrong_attribute_id_path = chip::app::ConcreteAttributePath(
        endpoint, chip::app::Clusters::OnOff::Id, chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Id);

    chip::DataVersion dataVersion            = 0;
    chip::app::AttributeValueEncoder encoder = setupEncoder(endpoint, test_wrong_attribute_id_path, dataVersion);
    auto err                                 = test_temperature_measurement_attributes.Read(test_wrong_attribute_id_path, encoder);

    sl_log_debug(TEST_LOG_TAG, "The Error after reading correct attribute id is %d", err);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    // 2
    // Reading unwritten attribute
    auto test_correct_attribute_id_path =
        chip::app::ConcreteAttributePath(endpoint, chip::app::Clusters::TemperatureMeasurement::Id,
                                         chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Id);

    chip::app::AttributeValueEncoder encoder_2 = setupEncoder(endpoint, test_correct_attribute_id_path, dataVersion);
    auto err_2 = test_temperature_measurement_attributes.Read(test_correct_attribute_id_path, encoder_2);

    sl_log_debug(TEST_LOG_TAG, "The Error after reading correct attribute id is %d", err_2);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

class TestContext
{
public:
    nlTestSuite * mTestSuite;
};

static const nlTest sTests[] = { NL_TEST_DEF("TestTemperatureMeasurementAttributes", TestTemperatureMeasurementAttributes),
                                 NL_TEST_DEF("TestTemperatureMeasurementReadFailures", TestTemperatureMeasurementReadFailures),
                                 NL_TEST_SENTINEL() };

static nlTestSuite TheCommandSuite = { "TestTemperatureMeasurement", &sTests[0], nullptr, nullptr };

int TestTemperatureMeasurement(void)
{
    return (chip::ExecuteTestsWithContext<TestContext>(&TheCommandSuite));
}

CHIP_REGISTER_TEST_SUITE(TestTemperatureMeasurement)
