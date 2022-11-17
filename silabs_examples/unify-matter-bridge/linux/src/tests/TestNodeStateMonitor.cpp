// Unify bridge components
#include "matter_device_translator.hpp"
#include "matter_node_state_monitor.hpp"

// Mocks
#include "MockUnifyEmberInterface.hpp"

// Unify lib components
#include "sl_log.h"
namespace unify_zap_types {
#include "zap-types.h"
}

// Chip components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Third party library
#include <nlunit-test.h>

#define TEST_LOG_TAG "NodeStateMonitorTest"
struct test_data_struct
{
    test_data_struct()
    {
        state_update_called = 0;
        last_state          = unify::matter_bridge::matter_node_state_monitor::NODE_DELETED;
        state_update_history.clear();
    }
    int state_update_called;
    unify::matter_bridge::matter_node_state_monitor::update_t last_state;
    std::vector<unify::matter_bridge::matter_node_state_monitor::update_t> state_update_history;
};

static test_data_struct test_data;

void reset_test_data()
{
    test_data.state_update_called = 0;
    test_data.last_state          = unify::matter_bridge::matter_node_state_monitor::NODE_DELETED;
    test_data.state_update_history.clear();
}

void unify_node_state_update(const unify::matter_bridge::bridged_endpoint & ep,
                             unify::matter_bridge::matter_node_state_monitor::update_t state)
{
    test_data.state_update_called++;
    test_data.state_update_history.push_back(state);
}

class WrapNodeStateMonitor : public unify::matter_bridge::matter_node_state_monitor
{
public:
    WrapNodeStateMonitor(unify::matter_bridge::device_translator & device_translator,
                         unify::matter_bridge::Test::MockUnifyEmberInterface & ember_interface) :
        matter_node_state_monitor(device_translator, ember_interface){};

    void call_on_unify_node_added(const unify::node_state_monitor::node & node) { this->on_unify_node_added(node); }

    void call_on_unify_node_removed(const std::string & unid) { this->on_unify_node_removed(unid); }

    void call_on_unify_node_state_changed(const unify::node_state_monitor::node & node) { this->on_unify_node_state_changed(node); }

    long unsigned int listeners_count() { return this->event_listeners.size(); }

    long unsigned int bridged_endpoints_count() { return this->bridged_endpoints.size(); }
};

static void TestNodeStateMonitorAddingNode(nlTestSuite * inSuite, void * aContext)
{
    using namespace unify::matter_bridge::Test;
    // 1
    // First test case covers adding a single node with a single endpoint
    reset_test_data();

    // Initialize the node state monitor
    unify::matter_bridge::device_translator matter_device_translator;
    MockUnifyEmberInterface unify_ember_interface;
    WrapNodeStateMonitor node_state_monitor(matter_device_translator, unify_ember_interface);

    node_state_monitor.register_event_listener(unify_node_state_update);
    NL_TEST_ASSERT(inSuite, node_state_monitor.listeners_count() == 1);

    // Create a node with OnOff Cluster endpoint
    unify::node_state_monitor::node node("unid-node-1");
    auto & cluster             = node.emplace_endpoint(1).emplace_cluster("OnOff");
    cluster.supported_commands = { "On", "Off", "Toggle" };
    cluster.attributes.emplace("OnOff");

    // Initial add should first call a NODE_ADDED event and then a NODE_STATE_CHANGED event
    node_state_monitor.call_on_unify_node_added(node);
    NL_TEST_ASSERT(inSuite, test_data.state_update_called == 2);
    NL_TEST_ASSERT(inSuite, test_data.state_update_history.at(0) == unify::matter_bridge::matter_node_state_monitor::NODE_ADDED);
    NL_TEST_ASSERT(inSuite,
                   test_data.state_update_history.at(1) == unify::matter_bridge::matter_node_state_monitor::NODE_STATE_CHANGED);
    NL_TEST_ASSERT(inSuite, node_state_monitor.bridged_endpoints_count() == 1);

    // 2
    // Second test case introduces another endpoint and adds the same node twice.
    // Reset test
    reset_test_data();

    // Adding another endpoint to the same node
    auto & cluster_2             = node.emplace_endpoint(2).emplace_cluster("Level");
    cluster_2.supported_commands = { "MoveToLevel", "Move", "Step" };
    cluster_2.attributes.emplace("CurrentLevel");

    // Adding a node with multiple endpoints
    node_state_monitor.call_on_unify_node_added(node);
    NL_TEST_ASSERT(inSuite, test_data.state_update_called == 4);
    NL_TEST_ASSERT(inSuite, test_data.state_update_history.at(0) == unify::matter_bridge::matter_node_state_monitor::NODE_ADDED);
    NL_TEST_ASSERT(inSuite,
                   test_data.state_update_history.at(1) == unify::matter_bridge::matter_node_state_monitor::NODE_STATE_CHANGED);
    NL_TEST_ASSERT(inSuite, test_data.state_update_history.at(2) == unify::matter_bridge::matter_node_state_monitor::NODE_ADDED);
    NL_TEST_ASSERT(inSuite,
                   test_data.state_update_history.at(3) == unify::matter_bridge::matter_node_state_monitor::NODE_STATE_CHANGED);
    sl_log_debug(TEST_LOG_TAG, "Bridged endpoints count: %d", node_state_monitor.bridged_endpoints_count());
    NL_TEST_ASSERT(inSuite, node_state_monitor.bridged_endpoints_count() == 2);

    // Adding the same node again triggers everything again except registering a new dynamic endpoint
    node_state_monitor.call_on_unify_node_added(node);
    NL_TEST_ASSERT(inSuite, test_data.state_update_called == 8);
    // This fails - we blindly adds bridged endpoints without checking if they are already added
    NL_TEST_ASSERT(inSuite, node_state_monitor.bridged_endpoints_count() == 2);
}

static void TestNodeStateMonitoringStateChangingNode(nlTestSuite * inSuite, void * aContext)
{
    using namespace unify::matter_bridge::Test;
    reset_test_data();

    // Initialize the node state monitor
    unify::matter_bridge::device_translator matter_device_translator;
    MockUnifyEmberInterface unify_ember_interface;
    WrapNodeStateMonitor node_state_monitor(matter_device_translator, unify_ember_interface);
    node_state_monitor.register_event_listener(unify_node_state_update);

    // Create a node with OnOff Cluster endpoint
    unify::node_state_monitor::node node_2("unid-node-2");
    auto & cluster             = node_2.emplace_endpoint(1).emplace_cluster("OnOff");
    cluster.supported_commands = { "On", "Off", "Toggle" };
    cluster.attributes.emplace("OnOff");

    // Call a state change before node is added nothing should happen!
    node_state_monitor.call_on_unify_node_state_changed(node_2);
    NL_TEST_ASSERT(inSuite, test_data.state_update_called == 0);

    // Initial add should first call a NODE_ADDED event and then a NODE_STATE_CHANGED event
    node_state_monitor.call_on_unify_node_added(node_2);
    NL_TEST_ASSERT(inSuite, test_data.state_update_called == 2);
    NL_TEST_ASSERT(inSuite, test_data.state_update_history.at(0) == unify::matter_bridge::matter_node_state_monitor::NODE_ADDED);
    NL_TEST_ASSERT(inSuite,
                   test_data.state_update_history.at(1) == unify::matter_bridge::matter_node_state_monitor::NODE_STATE_CHANGED);

    // Call a state change after node is added should trigger a NODE_STATE_CHANGED event
    node_state_monitor.call_on_unify_node_state_changed(node_2);
    NL_TEST_ASSERT(inSuite, test_data.state_update_called == 3);
    NL_TEST_ASSERT(inSuite,
                   test_data.state_update_history.at(2) == unify::matter_bridge::matter_node_state_monitor::NODE_STATE_CHANGED);
}

static void TestOnNodeRemoved(nlTestSuite * inSuite, void * aContext)
{
    using namespace unify::matter_bridge::Test;
    reset_test_data();

    // Setup
    // Initialize the node state monitor
    unify::matter_bridge::device_translator matter_device_translator;
    MockUnifyEmberInterface unify_ember_interface;
    WrapNodeStateMonitor node_state_monitor(matter_device_translator, unify_ember_interface);
    node_state_monitor.register_event_listener(unify_node_state_update);

    // Create a node with OnOff Cluster endpoint
    const std::string node_id = "unid-node-3";
    unify::node_state_monitor::node node_3(node_id);
    auto & cluster             = node_3.emplace_endpoint(1).emplace_cluster("OnOff");
    cluster.supported_commands = { "On", "Off", "Toggle" };
    cluster.attributes.emplace("OnOff");

    // 1
    // First test case introduces a node and removes it

    // Initial add should first call a NODE_ADDED event and then a NODE_STATE_CHANGED event
    node_state_monitor.call_on_unify_node_added(node_3);
    NL_TEST_ASSERT(inSuite, test_data.state_update_called == 2);
    NL_TEST_ASSERT(inSuite, test_data.state_update_history.at(0) == unify::matter_bridge::matter_node_state_monitor::NODE_ADDED);
    NL_TEST_ASSERT(inSuite,
                   test_data.state_update_history.at(1) == unify::matter_bridge::matter_node_state_monitor::NODE_STATE_CHANGED);

    // Remove the node
    node_state_monitor.call_on_unify_node_removed(node_id);
    NL_TEST_ASSERT(inSuite, test_data.state_update_called == 3);
    NL_TEST_ASSERT(inSuite, test_data.state_update_history.at(2) == unify::matter_bridge::matter_node_state_monitor::NODE_DELETED);
    NL_TEST_ASSERT(inSuite, node_state_monitor.bridged_endpoints_count() == 0);

    // 2
    // Second test case adds the same node again and removes it
    node_state_monitor.call_on_unify_node_added(node_3);
    NL_TEST_ASSERT(inSuite, test_data.state_update_called == 5);
    NL_TEST_ASSERT(inSuite, test_data.state_update_history.at(3) == unify::matter_bridge::matter_node_state_monitor::NODE_ADDED);
    NL_TEST_ASSERT(inSuite,
                   test_data.state_update_history.at(4) == unify::matter_bridge::matter_node_state_monitor::NODE_STATE_CHANGED);

    node_state_monitor.call_on_unify_node_removed(node_id);
    NL_TEST_ASSERT(inSuite, test_data.state_update_called == 6);

    // 3
    // Removed a node which does not exist
    node_state_monitor.call_on_unify_node_removed(node_id);
    NL_TEST_ASSERT(inSuite, test_data.state_update_called == 6);
}

static void TestMultipleListeners(nlTestSuite * inSuite, void * aContext)
{
    using namespace unify::matter_bridge::Test;
    reset_test_data();
    // Setup
    // Initialize the node state monitor
    unify::matter_bridge::device_translator matter_device_translator;
    MockUnifyEmberInterface unify_ember_interface;
    WrapNodeStateMonitor node_state_monitor(matter_device_translator, unify_ember_interface);

    // Create a node with OnOff Cluster endpoint
    const std::string node_id = "unid-node-3";
    unify::node_state_monitor::node node_4(node_id);
    auto & cluster             = node_4.emplace_endpoint(1).emplace_cluster("OnOff");
    cluster.supported_commands = { "On", "Off", "Toggle" };
    cluster.attributes.emplace("OnOff");

    // Create additional listener functions
    int listener_1_called                = 0;
    uint8_t listener_1_unify_endpoint_id = 0;
    int listener_2_called                = 0;
    uint8_t listener_2_unify_endpoint_id = 0;

    auto listener_1 = [&](const unify::matter_bridge::bridged_endpoint & ep,
                          unify::matter_bridge::matter_node_state_monitor::update_t state) {
        listener_1_called++;
        listener_1_unify_endpoint_id = ep.unify_endpoint;
    };

    auto listener_2 = [&](const unify::matter_bridge::bridged_endpoint & ep,
                          unify::matter_bridge::matter_node_state_monitor::update_t state) {
        listener_2_called++;
        listener_2_unify_endpoint_id = ep.unify_endpoint;
    };

    // Register a listener
    node_state_monitor.register_event_listener(unify_node_state_update);
    node_state_monitor.register_event_listener(listener_1);
    node_state_monitor.register_event_listener(listener_2);

    // Add the node
    node_state_monitor.call_on_unify_node_added(node_4);
    NL_TEST_ASSERT(inSuite, test_data.state_update_called == 2);
    NL_TEST_ASSERT(inSuite, listener_1_called == 2);
    NL_TEST_ASSERT(inSuite, listener_2_called == 2);
    NL_TEST_ASSERT(inSuite, listener_1_unify_endpoint_id == listener_2_unify_endpoint_id);
}

static void TestEmberInterfaceNodeAdded(nlTestSuite * inSuite, void * aContext)
{
    using namespace unify::matter_bridge::Test;
    sl_log_warning(TEST_LOG_TAG, "Starting TestEmberInterfaceNodeAdded");
    reset_test_data();

    // Setup
    // Initialize the node state monitor
    unify::matter_bridge::device_translator matter_device_translator;
    MockUnifyEmberInterface unify_ember_interface;
    WrapNodeStateMonitor node_state_monitor(matter_device_translator, unify_ember_interface);
    node_state_monitor.register_event_listener(unify_node_state_update);

    // Create a node with clustr endpoints resembling a dimmable light.
    const std::string node_id = "unid-ember-check-1";
    unify::node_state_monitor::node node_ember_1(node_id);
    auto & ep                        = node_ember_1.emplace_endpoint(1);
    auto & onoff_cluster             = ep.emplace_cluster("OnOff");
    onoff_cluster.supported_commands = { "On", "Off", "Toggle" };
    onoff_cluster.attributes.emplace("OnOff");

    auto & level_cluster             = ep.emplace_cluster("Level");
    level_cluster.supported_commands = { "MoveToLevel",          "Move",          "Step",          "Stop",
                                         "MoveToLevelWithOnOff", "MoveWithOnOff", "StepWithOnOff", "StopWithOnOff" };
    level_cluster.attributes.emplace("CurrentLevel");

    auto & basic_cluster = ep.emplace_cluster("Basic");
    basic_cluster.attributes.emplace("ClusterRevision");
    basic_cluster.attributes.emplace("ManufacturerName");
    basic_cluster.attributes.emplace("SerialNumber");
    basic_cluster.attributes.emplace("HWVersion");
    basic_cluster.attributes.emplace("PowerSource");

    auto & identify_cluster             = ep.emplace_cluster("Identify");
    identify_cluster.supported_commands = { "Identify", "IdentifyQuery", "TriggerEffect" };
    identify_cluster.attributes.emplace("IdentifyTime");
    identify_cluster.attributes.emplace("ClusterRevision");

    auto & groups_cluster             = ep.emplace_cluster("Groups");
    groups_cluster.supported_commands = { "AddGroup",           "ViewGroup",
                                          "GetGroupMembership", "RemoveGroup",
                                          "RemoveAllGroups",    "AddGroupIfIdentifying",
                                          "ViewGroupResponse",  "GetGroupMembershipResponse" };
    groups_cluster.attributes.emplace("NameSupport");
    groups_cluster.attributes.emplace("ClusterRevision");
    groups_cluster.attributes.emplace("GroupList");

    ep.emplace_cluster("Scenes");

    // 1
    // Add a unify node and check the proper ember calls are made.

    // Initial add should first call a NODE_ADDED event and then a NODE_STATE_CHANGED event
    node_state_monitor.call_on_unify_node_added(node_ember_1);
    NL_TEST_ASSERT(inSuite, test_data.state_update_called == 2);
    NL_TEST_ASSERT(inSuite, test_data.state_update_history.at(0) == unify::matter_bridge::matter_node_state_monitor::NODE_ADDED);
    NL_TEST_ASSERT(inSuite,
                   test_data.state_update_history.at(1) == unify::matter_bridge::matter_node_state_monitor::NODE_STATE_CHANGED);

    // Check the ember calls
    chip::EndpointId matter_endpoint_id = unify_ember_interface.getLastReturnedAvailableEndpointId();

    // Check endpoints and indexes
    NL_TEST_ASSERT(inSuite, unify_ember_interface.calledWithId == matter_endpoint_id);
    // The matter_endpoint_id and index should be the same when matter_endpoint_id is substracted by FIXED_ENDPOINT_COUNT
    NL_TEST_ASSERT(inSuite, unify_ember_interface.calledWithIndex == matter_endpoint_id - FIXED_ENDPOINT_COUNT);
    NL_TEST_ASSERT(inSuite, unify_ember_interface.cleared_endpoints.empty());

    // Check the cluster mapping to a device of the node is always
    // present in the device type list. Do not know why.
    const uint16_t dimmable_light   = 0x0101;
    const uint16_t bridge_device_id = 0x0013;

    bool dimmable_light_device_id_present = false;
    bool device_is_a_bridge_device        = false;
    for (auto & deviceType : unify_ember_interface.calledWithDeviceTypeList)
    {
        sl_log_debug(TEST_LOG_TAG, "Device types in list %d", deviceType.deviceId);
        if (deviceType.deviceId == dimmable_light)
        {
            dimmable_light_device_id_present = true;
        }

        if (deviceType.deviceId == bridge_device_id)
        {
            device_is_a_bridge_device = true;
        }
    }
    NL_TEST_ASSERT(inSuite, dimmable_light_device_id_present);
    NL_TEST_ASSERT(inSuite, device_is_a_bridge_device);
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
    NL_TEST_DEF("NodeStateMonitor::TestNodeStateMonitorAddingNode",             TestNodeStateMonitorAddingNode),
    NL_TEST_DEF("NodeStateMonitor::TestNodeStateMonitoringStateChangingNode",             TestNodeStateMonitoringStateChangingNode),
    NL_TEST_DEF("NodeStateMonitor::TestOnNodeRemoved",             TestOnNodeRemoved),
    NL_TEST_DEF("NodeStateMonitor::TestMultipleListeners",             TestMultipleListeners),
    NL_TEST_DEF("NodeStateMonitor::TestEmberInterfaceNodeAdded",             TestEmberInterfaceNodeAdded),
    NL_TEST_SENTINEL()
};


// clang-format off
static nlTestSuite kTheSuite =
{
    "NodeStateMonitorTests",
    &sTests[0],
    nullptr,
    nullptr
};

int TestNodeStateMonitorSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestNodeStateMonitorSuite)
