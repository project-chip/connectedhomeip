// A mapping from a cluster to its device name.
#include "matter_device_types_clusters_list_updated.inc"

// Unify bridge components
#include "matter_cluster_interactor.hpp"
#include "matter_device_translator.hpp"
#include "matter_endpoint_builder.hpp"

// Chip components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Third party library
#include <nlunit-test.h>

#define TEST_LOG_TAG "ClusterInteractorTest"

static void TestClusterInteractorNotYetBuilt(nlTestSuite * inSuite, void * aContext)
{
    // Initialize the cluster interactor
    unify::matter_bridge::device_translator matter_device_translator;
    unify::matter_bridge::matter_endpoint_builder builder;
    unify::matter_bridge::cluster_interactor cluster_interactor =
        unify::matter_bridge::cluster_interactor(matter_device_translator, builder);

    // Fall back to "flowsensor" device type.
    const char * dt = matter_device_type_vs_clusters_map.find(cluster_interactor.get_matter_type().value())->second.second;
    std::string device_type(dt);
    NL_TEST_ASSERT(inSuite, device_type == "flowsensor");
}

static void TestClusterInteractorEmpty(nlTestSuite * inSuite, void * aContext)
{
    // Initialize the cluster interactor
    unify::matter_bridge::device_translator matter_device_translator;
    unify::matter_bridge::matter_endpoint_builder builder;
    unify::matter_bridge::cluster_interactor cluster_interactor =
        unify::matter_bridge::cluster_interactor(matter_device_translator, builder);

    std::unordered_map<std::string, unify::node_state_monitor::cluster> clusters = {};
    cluster_interactor.build_matter_cluster(clusters);
    const char * dt = matter_device_type_vs_clusters_map.find(cluster_interactor.get_matter_type().value())->second.second;
    std::string device_type(dt);
    NL_TEST_ASSERT(inSuite, device_type == "flowsensor");
}

static void TestClusterInteractorDoorLock(nlTestSuite * inSuite, void * aContext)
{
    // Initialize the cluster interactor
    unify::matter_bridge::device_translator matter_device_translator;
    unify::matter_bridge::matter_endpoint_builder builder;
    unify::matter_bridge::cluster_interactor cluster_interactor =
        unify::matter_bridge::cluster_interactor(matter_device_translator, builder);

    std::unordered_map<std::string, unify::node_state_monitor::cluster> clusters = {
        { "Basic", unify::node_state_monitor::cluster("Basic") },
        { "Identify", unify::node_state_monitor::cluster("Identify") },
        { "DoorLock", unify::node_state_monitor::cluster("DoorLock") },
        { "Scenes", unify::node_state_monitor::cluster("Scenes") },
        { "Groups", unify::node_state_monitor::cluster("Groups") },
    };

    for (auto & [_matter, unify] : clusters)
    {
        unify.attributes.emplace("@");
    }

    cluster_interactor.build_matter_cluster(clusters);
    const char * dt = matter_device_type_vs_clusters_map.find(cluster_interactor.get_matter_type().value())->second.second;
    std::string device_type(dt);
    NL_TEST_ASSERT(inSuite, device_type == "doorlock");
}

static void TestClusterInteractorOccupancySensor(nlTestSuite * inSuite, void * aContext)
{
    // Initialize the cluster interactor
    unify::matter_bridge::device_translator matter_device_translator;
    unify::matter_bridge::matter_endpoint_builder builder;
    unify::matter_bridge::cluster_interactor cluster_interactor =
        unify::matter_bridge::cluster_interactor(matter_device_translator, builder);

    std::unordered_map<std::string, unify::node_state_monitor::cluster> clusters = {
        { "Basic", unify::node_state_monitor::cluster("Basic") },
        { "Identify", unify::node_state_monitor::cluster("Identify") },
        { "OccupancySensing", unify::node_state_monitor::cluster("OccupancySensing") },
        { "IASZone", unify::node_state_monitor::cluster("IASZone") },
    };

    for (auto & [_matter, unify] : clusters)
    {
        unify.attributes.emplace("@");
    }

    cluster_interactor.build_matter_cluster(clusters);
    const char * dt = matter_device_type_vs_clusters_map.find(cluster_interactor.get_matter_type().value())->second.second;
    std::string device_type(dt);
    NL_TEST_ASSERT(inSuite, device_type == "occupancysensor");
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
static const nlTest sTests[] = {
    NL_TEST_DEF("ClusterInteractor::TestClusterInteractorNotYetBuilt", TestClusterInteractorNotYetBuilt),
    NL_TEST_DEF("ClusterInteractor::TestClusterInteractorEmpty", TestClusterInteractorEmpty),
    NL_TEST_DEF("ClusterInteractor::TestClusterInteractorDoorLock", TestClusterInteractorDoorLock),
    NL_TEST_DEF("ClusterInteractor::TestClusterInteractorOccupancySensor", TestClusterInteractorOccupancySensor), NL_TEST_SENTINEL()
};

static nlTestSuite kTheSuite = { "ClusterInteractorTests", &sTests[0], nullptr, nullptr };

int TestClusterInteractorSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestClusterInteractorSuite)
