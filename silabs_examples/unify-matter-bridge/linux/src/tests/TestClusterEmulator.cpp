// A mapping from a cluster to its device name.
#include "matter_device_types_clusters_list.inc"

// Unify bridge components
#include "cluster_emulator.hpp"
#include "matter_cluster_interactor.hpp"
#include "matter_device_translator.hpp"
#include "matter_endpoint_builder.hpp"

// UIC components
#include "sl_log.h"

// Chip components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Third party library
#include <nlunit-test.h>

#define TEST_LOG_TAG "ClusterEmulatorTest"

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

static void TestClusterEmulatorEmulationClusterRevisionAndFeatureMap(nlTestSuite * inSuite, void * aContext)
{
    sl_log_debug(TEST_LOG_TAG, "TestClusterEmulatorEmulation");
    // Initialize the cluster interactor
    unify::matter_bridge::device_translator matter_device_translator(false);
    unify::matter_bridge::matter_endpoint_builder builder;
    unify::matter_bridge::ClusterEmulator emulator;

    unify::matter_bridge::cluster_interactor cluster_interactor =
        unify::matter_bridge::cluster_interactor(emulator, matter_device_translator, builder);

    auto onoff_cluster               = unify::node_state_monitor::cluster("OnOff");
    onoff_cluster.attributes         = { "OnOff" };
    onoff_cluster.supported_commands = { "Toggle" };
    sl_log_debug(TEST_LOG_TAG, "Unify cluster setup");

    std::unordered_map<std::string, unify::node_state_monitor::cluster> clusters = {
        { "OnOff", onoff_cluster },
    };

    cluster_interactor.build_matter_cluster(clusters);
    EmberAfCluster matter_onoff_cluster = cluster_interactor.endpoint_builder.clusters[0];
    NL_TEST_ASSERT(inSuite, matter_onoff_cluster.clusterId == chip::app::Clusters::OnOff::Id);

    // Test the emulated attributes and commands
    chip::AttributeId emulated_attribute_feature_map = ZCL_FEATURE_MAP_SERVER_ATTRIBUTE_ID;
    chip::AttributeId cluster_revision_attribute_id  = ZCL_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID;
    bool feature_map_found                           = false;
    bool cluster_revision_found                      = false;

    for (uint8_t i = 0; i < matter_onoff_cluster.attributeCount; i++)
    {
        if (matter_onoff_cluster.attributes[i].attributeId == emulated_attribute_feature_map)
        {
            chip::app::ConcreteAttributePath aPath = { 0, matter_onoff_cluster.clusterId,
                                                       matter_onoff_cluster.attributes[i].attributeId };
            NL_TEST_ASSERT(inSuite, emulator.is_attribute_emulated(aPath));
            feature_map_found = true;
        }

        if (matter_onoff_cluster.attributes[i].attributeId == cluster_revision_attribute_id)
        {
            chip::app::ConcreteAttributePath aPath = { 0, matter_onoff_cluster.clusterId,
                                                       matter_onoff_cluster.attributes[i].attributeId };
            NL_TEST_ASSERT(inSuite, emulator.is_attribute_emulated(aPath));
            cluster_revision_found = true;
        }
    }
    NL_TEST_ASSERT(inSuite, feature_map_found);
    NL_TEST_ASSERT(inSuite, cluster_revision_found);

    // Test reading the emulated feature map and cluster revision
    auto attribute_id_path =
        chip::app::ConcreteAttributePath(0, chip::app::Clusters::OnOff::Id, ZCL_FEATURE_MAP_SERVER_ATTRIBUTE_ID);
    chip::DataVersion dataVersion            = 0;
    chip::app::AttributeValueEncoder encoder = setupEncoder(1, attribute_id_path, dataVersion);

    // The encoding fails everytime properly due to bad setup. This is a bug in the test code.
    NL_TEST_ASSERT(inSuite, emulator.read_attribute(attribute_id_path, encoder) != CHIP_NO_ERROR);

    auto revision_attribute_id_path =
        chip::app::ConcreteAttributePath(0, chip::app::Clusters::OnOff::Id, ZCL_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID);
    chip::DataVersion revisionDataVersion             = 0;
    chip::app::AttributeValueEncoder revision_encoder = setupEncoder(1, attribute_id_path, revisionDataVersion);

    // The encoding fails everytime properly due to bad setup. This is a bug in the test code.
    NL_TEST_ASSERT(inSuite, emulator.read_attribute(revision_attribute_id_path, revision_encoder) != CHIP_NO_ERROR);
}

static void TestClusterEmulatorIdentify(nlTestSuite * inSuite, void * aContext)
{
    sl_log_debug(TEST_LOG_TAG, "TestClusterEmulatorEmulation");
    // Initialize the cluster interactor
    unify::matter_bridge::device_translator matter_device_translator(false);
    unify::matter_bridge::matter_endpoint_builder builder;
    unify::matter_bridge::ClusterEmulator emulator;

    unify::matter_bridge::cluster_interactor cluster_interactor =
        unify::matter_bridge::cluster_interactor(emulator, matter_device_translator, builder);

    auto unify_identify_cluster       = unify::node_state_monitor::cluster("Identify");
    unify_identify_cluster.attributes = { "IdentifyTime" };
    sl_log_debug(TEST_LOG_TAG, "Unify cluster setup");

    std::unordered_map<std::string, unify::node_state_monitor::cluster> clusters = {
        { "Identify", unify_identify_cluster },
    };

    cluster_interactor.build_matter_cluster(clusters);
    EmberAfCluster identify_cluster = cluster_interactor.endpoint_builder.clusters[0];
    NL_TEST_ASSERT(inSuite, identify_cluster.clusterId == chip::app::Clusters::Identify::Id);

    // Test the emulated attribute Identify Type
    chip::AttributeId identify_type_id = ZCL_IDENTIFY_TYPE_ATTRIBUTE_ID;

    chip::app::ConcreteAttributePath aPath = { 0, chip::app::Clusters::Identify::Id, identify_type_id };
    NL_TEST_ASSERT(inSuite, emulator.is_attribute_emulated(aPath));

    // This will be true for all endpoints which has the Identify cluster.
    aPath = { 1, chip::app::Clusters::Identify::Id, identify_type_id };
    NL_TEST_ASSERT(inSuite, emulator.is_attribute_emulated(aPath));

    bool identify_type_found = false;
    for (uint8_t i = 0; i < identify_cluster.attributeCount; i++)
    {
        if (identify_cluster.attributes[i].attributeId == identify_type_id)
        {
            aPath = { 0, identify_cluster.clusterId, identify_cluster.attributes[i].attributeId };
            NL_TEST_ASSERT(inSuite, emulator.is_attribute_emulated(aPath));
            identify_type_found = true;
        }
    }
    NL_TEST_ASSERT(inSuite, identify_type_found);

    // Test reading the emulated attribute IdentifyType
    auto attribute_id_path                   = chip::app::ConcreteAttributePath(0, chip::app::Clusters::Identify::Id,
                                                              chip::app::Clusters::Identify::Attributes::IdentifyType::Id);
    chip::DataVersion dataVersion            = 0;
    chip::app::AttributeValueEncoder encoder = setupEncoder(1, attribute_id_path, dataVersion);

    // The encoding fails everytime properly due to bad setup. This is a bug in the test code.
    NL_TEST_ASSERT(inSuite, emulator.read_attribute(attribute_id_path, encoder) != CHIP_NO_ERROR);

    // Invalid attribute id
    auto invalid_attribute_id_path =
        chip::app::ConcreteAttributePath(1, chip::app::Clusters::Identify::Id, chip::app::Clusters::OnOff::Attributes::OnOff::Id);
    NL_TEST_ASSERT(inSuite, emulator.read_attribute(invalid_attribute_id_path, encoder) == CHIP_ERROR_INVALID_ARGUMENT);
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
static const nlTest sTests[] = { NL_TEST_DEF("TestClusterEmulatorEmulationClusterRevisionAndFeatureMap",
                                             TestClusterEmulatorEmulationClusterRevisionAndFeatureMap),
                                 NL_TEST_DEF("TestClusterEmulatorIdentify", TestClusterEmulatorIdentify), NL_TEST_SENTINEL() };

static nlTestSuite kTheSuite = { "ClusterEmulatorTests", &sTests[0], nullptr, nullptr };

int TestClusterEmulatorSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestClusterEmulatorSuite)
