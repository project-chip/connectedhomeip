// Chip components
#include <app/MessageDef/AttributeDataIB.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Third party library
#include <iostream>
#include <nlunit-test.h>
#include <string>

#include "attribute_translator.hpp"
// Mocks
#include "MockNodeStateMonitor.hpp"
#include "MockUnifyMqtt.hpp"

using namespace unify::matter_bridge;

#define TEST_LOG_TAG "AttributeTranslatorTest"

static UnifyEmberInterface ember_interface = UnifyEmberInterface();
static device_translator dev_translator    = device_translator();

chip::app::AttributeValueEncoder setupEncoder(chip::EndpointId endpoint, chip::app::ConcreteAttributePath & path,
                                              chip::DataVersion & dataVersion)
{
    const chip::app::AttributeValueEncoder::AttributeEncodeState & aState =
        chip::app::AttributeValueEncoder::AttributeEncodeState();
    chip::app::AttributeReportIBs::Builder builder;
    chip::FabricIndex fabricIndex = static_cast<uint8_t>(endpoint);
    chip::app::AttributeValueEncoder encoder(builder, fabricIndex, path, dataVersion, true, aState);
    uint8_t buf[1024];
    chip::TLV::TLVWriter writer;
    writer.Init(buf);
    chip::TLV::TLVType ignored;
    writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, ignored);
    builder.Init(&writer, 1);
    return encoder;
}

static void TestClusterTranslatorRevision(nlTestSuite * inSuite, void * aContext)
{
    Test::MockNodeStateMonitor test_matter_node_state_monitor(dev_translator, ember_interface);
    Test::MockUnifyMqtt mqtt_publish_test;
    // testing Identify Cluster revision
    IdentifyAttributeAccess test_identify_attribute_handler(test_matter_node_state_monitor, mqtt_publish_test);
    const uint16_t endpoint                             = 4;
    chip::app::ConcreteReadAttributePath test_attr_path = chip::app::ConcreteReadAttributePath(
        endpoint, chip::app::Clusters::Identify::Id, chip::app::Clusters::Identify::Attributes::ClusterRevision::Id);

    chip::DataVersion dataVersion            = 0;
    chip::app::AttributeValueEncoder encoder = setupEncoder(endpoint, test_attr_path, dataVersion);
    auto err                                 = test_identify_attribute_handler.Read(test_attr_path, encoder);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

class TestContext
{
public:
    nlTestSuite * mTestSuite;
};

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF("ClusterTranslator::TestClusterTranslatorRevision", TestClusterTranslatorRevision),
                                 NL_TEST_SENTINEL() };

static nlTestSuite kTheSuite = { "ClusterTranslatorTests", &sTests[0], nullptr, nullptr };

int TestClusterTranslatorSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestClusterTranslatorSuite)
