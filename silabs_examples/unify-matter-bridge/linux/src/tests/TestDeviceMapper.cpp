#include "TestDeviceMapperSelection.hpp"
#include "cluster_emulator.hpp"
#include "matter_cluster_interactor.hpp"
#include "matter_device_translator.hpp"

// Chip components
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Unify component
#include "sl_log.h"

// Third party library
#include <iostream>
#include <nlunit-test.h>
#include <string>
#include <unordered_map>
#include <vector>

using namespace unify::matter_bridge;
namespace unify_monitor = unify::node_state_monitor;

#define LOG_TAG "matter_device_translator_test"

std::string getFirstPrioritizedDevice(std::unordered_map<std::string, unify_monitor::cluster> input_clusters,
                                      device_translator dev_translator)
{

    matter_endpoint_builder builder;
    ClusterEmulator emulator;
    cluster_interactor cluster_interactor(emulator, dev_translator, builder);
    cluster_interactor.build_matter_cluster(input_clusters);

    std::vector<chip::DeviceTypeId> device_types_id = dev_translator.get_device_types(cluster_interactor.endpoint_builder.clusters);
    if (device_types_id.empty())
    {
        return "Unknown";
    }
    const char * dt = matter_device_type_vs_clusters_map.find(device_types_id[0])->second.device_type_name;
    std::string device_type(dt);
    return device_type;
}

template <typename T>
void check_optional_value(nlTestSuite * inSuite, std::optional<T> value_to_check, T value)
{
    NL_TEST_ASSERT(inSuite, value_to_check.has_value() == true);

    if (value_to_check.has_value())
    {
        NL_TEST_ASSERT(inSuite, value_to_check.value() == value);
    }
}

void TestMatterDeviceScore(nlTestSuite * inSuite, void * aContext)
{
    auto doorlock_cluster       = unify_monitor::cluster("DoorLock");
    doorlock_cluster.attributes = { "LockState" };
    auto basic_cluster          = unify_monitor::cluster("Basic");
    basic_cluster.attributes    = { "ZCLVersion" };
    auto identify_cluster       = unify_monitor::cluster("Identify");
    identify_cluster.attributes = { "IdentifyTime" };

    std::unordered_map<std::string, unify_monitor::cluster> clusters = {
        { "Basic", basic_cluster },
        { "Identify", identify_cluster },
        { "DoorLock", doorlock_cluster },
    };

    const device_translator dev_translator(false);
    matter_endpoint_builder builder;
    ClusterEmulator emulator;
    cluster_interactor cluster_interactor(emulator, dev_translator, builder);
    cluster_interactor.build_matter_cluster(clusters);

    std::vector<cluster_score> scores =
        compute_device_type_match_score(cluster_interactor.endpoint_builder.clusters, dev_translator);

    for (auto score : scores)
    {
        if (score.device_type_id == 0x000A)
        {
            sl_log_debug(LOG_TAG, "Device DoorLock, matter_miss_count: %d, matter_extra: %d", score.matter_miss_count,
                         score.extra_matter_clusters_count);
            // Clusters for DoorLock is [Binding, Descriptor, DoorLock, Identify, TimeSynchronization] we only provide 3
            NL_TEST_ASSERT(inSuite, score.matter_miss_count == 2);
            // By default FixedLabel and BridgedDeviceBasic is enabled, so we have two extra clusters.
            NL_TEST_ASSERT(inSuite, score.extra_matter_clusters_count == 2);
            // We have 3 clusters that matches, so we have 3 points
            NL_TEST_ASSERT(inSuite, score.required_matter_clusters_count == 3);
        }
    }
}

void TestDeviceMapperPrioritization(nlTestSuite * inSuite, void * aContext)
{
    // 1 Test if the device mapper prioritizes the doorlock cluster
    auto doorlock_cluster       = unify_monitor::cluster("DoorLock");
    doorlock_cluster.attributes = { "LockState" };
    auto basic_cluster          = unify_monitor::cluster("Basic");
    basic_cluster.attributes    = { "ZCLVersion" };
    auto identify_cluster       = unify_monitor::cluster("Identify");
    identify_cluster.attributes = { "IdentifyTime" };

    std::unordered_map<std::string, unify_monitor::cluster> clusters = {
        { "Basic", basic_cluster },
        { "Identify", identify_cluster },
        { "DoorLock", doorlock_cluster },
    };

    const device_translator dev_translator(false);
    std::string output_device = getFirstPrioritizedDevice(clusters, dev_translator);
    NL_TEST_ASSERT(inSuite, output_device == "doorlock");

    // 2 When adding scenes and groups it will not pick the doorlock because it
    // matches those two clusters on another cluster even though it has the
    // doorlock cluster. This is no good but is fixed when enabling conformance to spec.
    auto group_cluster        = unify_monitor::cluster("Groups");
    group_cluster.attributes  = { "NameSupport" };
    auto scenes_cluster       = unify_monitor::cluster("Scenes");
    scenes_cluster.attributes = { "Count" };
    clusters                  = {
        { "Basic", basic_cluster },
        { "Identify", identify_cluster },
        { "DoorLock", doorlock_cluster },
        { "Groups", group_cluster },
    };
    output_device = getFirstPrioritizedDevice(clusters, dev_translator);
    NL_TEST_ASSERT(inSuite, output_device == "doorlock");

    // 3 Check that the device mapper prioritizes the occupancysensor cluster
    auto occupancy_sensing_cluster                                                    = unify_monitor::cluster("OccupancySensing");
    occupancy_sensing_cluster.attributes                                              = { "Occupancy" };
    auto ias_zone_cluster                                                             = unify_monitor::cluster("IASZone");
    ias_zone_cluster.attributes                                                       = { "ZoneState" };
    std::unordered_map<std::string, unify_monitor::cluster> OccupancySensing_clusters = { { "Basic", basic_cluster },
                                                                                          { "Identify", identify_cluster },
                                                                                          { "OccupancySensing",
                                                                                            occupancy_sensing_cluster },
                                                                                          { "IASZone", ias_zone_cluster } };
    output_device = getFirstPrioritizedDevice(OccupancySensing_clusters, dev_translator);
    NL_TEST_ASSERT(inSuite, output_device == "occupancysensor");

    // 4 Check that the device mapper prioritizes the extendedcolorlight cluster
    auto color_control_cluster       = unify_monitor::cluster("ColorControl");
    color_control_cluster.attributes = { "CurrentX", "CurrentY" };
    auto on_off_cluster              = unify_monitor::cluster("OnOff");
    on_off_cluster.attributes        = { "OnOff" };
    auto level_cluster               = unify_monitor::cluster("Level");
    level_cluster.attributes         = { "CurrentLevel" };

    std::unordered_map<std::string, unify_monitor::cluster> colordimmablelight_clusters = { { "Basic", basic_cluster },
                                                                                            { "Identify", identify_cluster },
                                                                                            { "Groups", group_cluster },
                                                                                            { "Scenes", scenes_cluster },
                                                                                            { "OnOff", on_off_cluster },
                                                                                            { "Level", level_cluster },
                                                                                            { "ColorControl",
                                                                                              color_control_cluster } };
    output_device = getFirstPrioritizedDevice(colordimmablelight_clusters, dev_translator);
    NL_TEST_ASSERT(inSuite, output_device == "extendedcolorlight");
}

void TestDeviceMapperPrioritizationConformingToSpec(nlTestSuite * inSuite, void * aContext)
{
    // 1 Test that the door lock is not mapped if it does not conform to spec
    auto doorlock_cluster                                            = unify_monitor::cluster("DoorLock");
    doorlock_cluster.attributes                                      = { "LockState" };
    doorlock_cluster.supported_commands                              = { "LockDoor", "UnlockDoor" };
    auto basic_cluster                                               = unify_monitor::cluster("Basic");
    basic_cluster.attributes                                         = { "ZCLVersion" };
    std::unordered_map<std::string, unify_monitor::cluster> clusters = {
        { "Basic", basic_cluster },
        { "DoorLock", doorlock_cluster },
    };
    sl_log_debug(LOG_TAG, "TestDeviceMapperPrioritizationConformingToSpec");
    const device_translator dev_translator(true);
    std::string output_device = getFirstPrioritizedDevice(clusters, dev_translator);
    NL_TEST_ASSERT(inSuite, output_device != "doorlock");

    // 2 Test onofflight device that conforms to spec
    auto groups_cluster               = unify_monitor::cluster("Groups");
    groups_cluster.attributes         = { "NameSupport" };
    groups_cluster.supported_commands = { "AddGroup",           "AddGroupResponse",           "ViewGroup",   "ViewGroupResponse",
                                          "GetGroupMembership", "GetGroupMembershipResponse", "RemoveGroup", "RemoveGroupResponse",
                                          "RemoveAllGroups",    "AddGroupIfIdentifying" };
    // We actually do not care about scene cluster as it is not specced out properly in Matter.
    auto scenes_cluster                  = unify_monitor::cluster("Scenes");
    scenes_cluster.attributes            = { "Count" };
    auto level_cluster                   = unify_monitor::cluster("Level");
    level_cluster.attributes             = { "CurrentLevel", "Options", "RemainingTime", "StartUpCurrentLevel" };
    level_cluster.supported_commands     = { "MoveToLevel",          "Move",          "Step",          "Stop",
                                         "MoveToLevelWithOnOff", "MoveWithOnOff", "StepWithOnOff", "StopWithOnOff" };
    auto on_off_cluster                  = unify_monitor::cluster("OnOff");
    on_off_cluster.attributes            = { "OnOff", "GlobalSceneControl", "OnTime", "OffWaitTime", "StartUpOnOff" };
    on_off_cluster.supported_commands    = { "Off", "On", "Toggle", "OffWithEffect", "OnWithRecallGlobalScene", "OnWithTimedOff" };
    auto occupancy_sensing_cluster       = unify_monitor::cluster("OccupancySensing");
    occupancy_sensing_cluster.attributes = { "Occupancy" };
    auto identify_cluster                = unify_monitor::cluster("Identify");
    identify_cluster.attributes          = { "IdentifyTime" };
    identify_cluster.supported_commands  = { "Identify", "TriggerEffect" };
    clusters                             = { { "Groups", groups_cluster },
                 { "Identify", identify_cluster },
                 { "OccupancySensing", occupancy_sensing_cluster },
                 { "OnOff", on_off_cluster },
                 { "Scenes", scenes_cluster } };
    output_device                        = getFirstPrioritizedDevice(clusters, dev_translator);
    NL_TEST_ASSERT(inSuite, output_device == "onofflight");

    // 3 Test onofflight device that does not conform to spec when missing scenes
    clusters = {
        { "Groups", groups_cluster }, { "Identify", identify_cluster }, { "Level", level_cluster }, { "OnOff", on_off_cluster }
    };
    output_device = getFirstPrioritizedDevice(clusters, dev_translator);
    NL_TEST_ASSERT(inSuite, output_device != "onofflight");
    NL_TEST_ASSERT(inSuite, output_device == "dimmablepluginunit");

    // 4 Test dimmablelight device mapping
    clusters      = { { "Groups", groups_cluster }, { "Identify", identify_cluster },
                 { "Level", level_cluster },   { "OccupancySensing", occupancy_sensing_cluster },
                 { "OnOff", on_off_cluster },  { "Scenes", scenes_cluster } };
    output_device = getFirstPrioritizedDevice(clusters, dev_translator);
    NL_TEST_ASSERT(inSuite, output_device == "dimmablelight");

    // 5 Test switch device mapping
    clusters = {
        { "Groups", groups_cluster }, { "Identify", identify_cluster }, { "OnOff", on_off_cluster }, { "Scenes", scenes_cluster }
    };
    output_device = getFirstPrioritizedDevice(clusters, dev_translator);
    NL_TEST_ASSERT(inSuite, output_device == "onoffpluginunit");
}

void TestDeviceMapperSpecCommands(nlTestSuite * inSuite, void * aContext)
{
    auto onoff_cluster               = unify_monitor::cluster("OnOff");
    onoff_cluster.attributes         = { "OnOff" };
    onoff_cluster.supported_commands = { "Off", "Toggle", "On" };

    std::unordered_map<std::string, unify_monitor::cluster> clusters = { { "OnOff", onoff_cluster } };

    const device_translator dev_translator(false);
    matter_endpoint_builder builder;
    ClusterEmulator emulator;
    cluster_interactor cluster_interactor(emulator, dev_translator, builder);
    cluster_interactor.build_matter_cluster(clusters);

    bool state = compare_commands(cluster_interactor.endpoint_builder.clusters[0].acceptedCommandList, "OnOff",
                                  { "On", "Off", "Toggle" }, dev_translator);
    NL_TEST_ASSERT(inSuite, state == true);

    state = compare_commands(cluster_interactor.endpoint_builder.clusters[0].acceptedCommandList, "OnOff",
                             { "On", "Off", "Toggle", "RandomNonesenseRequiredCommand" }, dev_translator);
    NL_TEST_ASSERT(inSuite, state == false);
}

void TestDeviceMapperSpecCommandsFail(nlTestSuite * inSuite, void * aContext)
{
    auto new_onoff_cluster               = unify_monitor::cluster("OnOff");
    new_onoff_cluster.attributes         = { "OnOff" };
    new_onoff_cluster.supported_commands = { "Off", "Toggle" };

    std::unordered_map<std::string, unify_monitor::cluster> new_clusters = { { "OnOff", new_onoff_cluster } };

    const device_translator dev_translator(false);
    matter_endpoint_builder new_builder;
    ClusterEmulator emulator;
    cluster_interactor cluster_interactor(emulator, dev_translator, new_builder);
    cluster_interactor.build_matter_cluster(new_clusters);

    bool state = compare_commands(cluster_interactor.endpoint_builder.clusters[0].acceptedCommandList, "OnOff",
                                  { "On", "Off", "Toggle" }, dev_translator);
    NL_TEST_ASSERT(inSuite, state == false);
    state =
        compare_commands(cluster_interactor.endpoint_builder.clusters[0].acceptedCommandList, "OnOff", { "Off" }, dev_translator);
    NL_TEST_ASSERT(inSuite, state == true);
    state = compare_commands(cluster_interactor.endpoint_builder.clusters[0].acceptedCommandList, "OnOff", { "Off", "Toggle" },
                             dev_translator);
    NL_TEST_ASSERT(inSuite, state == true);
}

void TestDeviceMapperSpecAttributes(nlTestSuite * inSuite, void * aContext)
{
    auto onoff_cluster       = unify_monitor::cluster("OnOff");
    onoff_cluster.attributes = { "OnOff", "OnTime", "OffWaitTime" };

    std::unordered_map<std::string, unify_monitor::cluster> clusters = { { "OnOff", onoff_cluster } };

    const device_translator dev_translator(false);
    matter_endpoint_builder builder;
    ClusterEmulator emulator;
    cluster_interactor cluster_interactor(emulator, dev_translator, builder);
    cluster_interactor.build_matter_cluster(clusters);

    bool state = compare_attributes(cluster_interactor.endpoint_builder.clusters[0].attributes,
                                    cluster_interactor.endpoint_builder.clusters[0].attributeCount, "OnOff",
                                    { "OnOff", "OnTime", "OffWaitTime" }, dev_translator);
    NL_TEST_ASSERT(inSuite, state == true);

    state = compare_attributes(cluster_interactor.endpoint_builder.clusters[0].attributes,
                               cluster_interactor.endpoint_builder.clusters[0].attributeCount, "OnOff",
                               { "OnOff", "OnTime", "OffWaitTime", "RandomNonesenseRequiredAttribute" }, dev_translator);
    NL_TEST_ASSERT(inSuite, state == false);
}

void TestDeviceMapperSpecAttributesFail(nlTestSuite * inSuite, void * aContext)
{
    auto new_onoff_cluster       = unify_monitor::cluster("OnOff");
    new_onoff_cluster.attributes = { "OnOff", "OnTime" };

    std::unordered_map<std::string, unify_monitor::cluster> new_clusters = { { "OnOff", new_onoff_cluster } };

    const device_translator dev_translator(true);
    matter_endpoint_builder new_builder;
    ClusterEmulator emulator;
    cluster_interactor cluster_interactor(emulator, dev_translator, new_builder);
    cluster_interactor.build_matter_cluster(new_clusters);

    bool state = compare_attributes(cluster_interactor.endpoint_builder.clusters[0].attributes,
                                    cluster_interactor.endpoint_builder.clusters[0].attributeCount, "OnOff",
                                    { "OnOff", "OnTime", "OffWaitTime" }, dev_translator);
    NL_TEST_ASSERT(inSuite, state == false);
    state = compare_attributes(cluster_interactor.endpoint_builder.clusters[0].attributes,
                               cluster_interactor.endpoint_builder.clusters[0].attributeCount, "OnOff", { "OnOff", "OnTime" },
                               dev_translator);
    NL_TEST_ASSERT(inSuite, state == true);
    state =
        compare_attributes(cluster_interactor.endpoint_builder.clusters[0].attributes,
                           cluster_interactor.endpoint_builder.clusters[0].attributeCount, "OnOff", { "OnOff" }, dev_translator);
    NL_TEST_ASSERT(inSuite, state == true);
}

void TestDeviceMapperSpecOnOffDeviceFail(nlTestSuite * inSuite, void * aContext)
{
    // None compliant OnOffplugin unit device
    auto new_onoff_cluster       = unify_monitor::cluster("OnOff");
    new_onoff_cluster.attributes = { "OnOff", "OnTime", "Global" };

    std::unordered_map<std::string, unify_monitor::cluster> new_clusters = { { "OnOff", new_onoff_cluster } };

    const device_translator dev_translator(true);
    matter_endpoint_builder new_builder;
    ClusterEmulator emulator;
    cluster_interactor cluster_interactor(emulator, dev_translator, new_builder);
    cluster_interactor.build_matter_cluster(new_clusters);

    chip::DeviceTypeId onoff_pluginunit = 0x010A;

    const DeviceTypeData onoff_plugin_unit_requirements = matter_device_type_vs_clusters_map.at(onoff_pluginunit);

    bool compliance = matter_clusters_conform_to_device_type(cluster_interactor.endpoint_builder.clusters,
                                                             onoff_plugin_unit_requirements.clusters, dev_translator);

    NL_TEST_ASSERT(inSuite, compliance == false);
}

void TestDeviceMapperSpecOnOffDeviceComplying(nlTestSuite * inSuite, void * aContext)
{
    // Compliant OnOffplugin unit device
    auto identify_cluster               = unify_monitor::cluster("Identify");
    identify_cluster.attributes         = { "IdentifyTime" };
    identify_cluster.supported_commands = { "Identify", "TriggerEffect" };
    auto onoff_cluster                  = unify_monitor::cluster("OnOff");
    onoff_cluster.attributes            = { "OnOff", "GlobalSceneControl", "OnTime", "OffWaitTime", "StartUpOnOff" };
    onoff_cluster.supported_commands    = { "Off", "On", "Toggle", "OffWithEffect", "OnWithRecallGlobalScene", "OnWithTimedOff" };
    auto scenes_cluster                 = unify_monitor::cluster("Scenes");
    scenes_cluster.supported_commands   = { "AddScene",
                                          "AddSceneResponse",
                                          "ViewScene",
                                          "ViewSceneResponse",
                                          "RemoveScene",
                                          "RemoveSceneResponse",
                                          "RemoveAllScenes",
                                          "RemoveAllScenesResponse",
                                          "StoreScene",
                                          "StoreSceneResponse",
                                          "RecallScene",
                                          "GetSceneMembership",
                                          "GetSceneMembershipResponse" };
    scenes_cluster.attributes           = { "SceneCount", "CurrentScene", "CurrentGroup", "SceneValid", "NameSupport" };
    auto groups_cluster                 = unify_monitor::cluster("Groups");
    groups_cluster.supported_commands   = { "AddGroup",           "AddGroupResponse",           "ViewGroup",   "ViewGroupResponse",
                                          "GetGroupMembership", "GetGroupMembershipResponse", "RemoveGroup", "RemoveGroupResponse",
                                          "RemoveAllGroups",    "AddGroupIfIdentifying" };
    groups_cluster.attributes           = { "NameSupport" };

    std::unordered_map<std::string, unify_monitor::cluster> new_clusters = {
        { "OnOff", onoff_cluster },
        { "Identify", identify_cluster },
        { "Scenes", scenes_cluster },
        { "Groups", groups_cluster },
    };

    const device_translator dev_translator(true);
    matter_endpoint_builder new_builder;
    ClusterEmulator emulator;
    cluster_interactor cluster_interactor(emulator, dev_translator, new_builder);
    cluster_interactor.build_matter_cluster(new_clusters);

    chip::DeviceTypeId onoff_pluginunit = 0x010A;

    const DeviceTypeData onoff_plugin_unit_requirements = matter_device_type_vs_clusters_map.at(onoff_pluginunit);

    // This wont comply to spec before following attributes and commands are
    // properly mapped or emulated:
    // Attribute: IdentifyType - is emulated now
    // Whole cluster Scenes
    bool compliance = matter_clusters_conform_to_device_type(cluster_interactor.endpoint_builder.clusters,
                                                             onoff_plugin_unit_requirements.clusters, dev_translator);
    NL_TEST_ASSERT(inSuite, compliance == true);
}

void TestDeviceMapperGetDeviceType(nlTestSuite * inSuite, void * aContext)
{
    const device_translator dev_translator(false);

    // 1 Test the correct device name is returned
    chip::DeviceTypeId device_type_id       = 0x0100;
    std::optional<const char *> device_name = dev_translator.get_device_name(device_type_id);
    std::string expected_value              = "onofflight";
    NL_TEST_ASSERT(inSuite, device_name.has_value() == true);
    if (device_name.has_value())
    {
        NL_TEST_ASSERT(inSuite, std::string(device_name.value()) == expected_value);
    }

    // 2 Test no device name is returned on unkonwn device
    device_type_id = 0x0000;
    device_name    = dev_translator.get_device_name(device_type_id);
    NL_TEST_ASSERT(inSuite, device_name.has_value() == false);
}

void TestDeviceMapperGetClusterId(nlTestSuite * inSuite, void * aContext)
{
    const device_translator dev_translator(false);

    // 1 Test the correct cluster id is returned
    std::string cluster_name                  = "Basic";
    std::optional<chip::ClusterId> cluster_id = dev_translator.get_cluster_id(cluster_name);
    check_optional_value(inSuite, cluster_id, chip::app::Clusters::BridgedDeviceBasic::Id);

    // 2 Test no cluster id is returned on unkonwn cluster
    cluster_name = "Unknown";
    cluster_id   = dev_translator.get_cluster_id(cluster_name);
    NL_TEST_ASSERT(inSuite, cluster_id.has_value() == false);

    // 3 Test OnOff cluster id is returned
    cluster_name = "OnOff";
    cluster_id   = dev_translator.get_cluster_id(cluster_name);
    check_optional_value(inSuite, cluster_id, chip::app::Clusters::OnOff::Id);
}

void TestDeviceMapperGetAttributeIdAndCommandId(nlTestSuite * inSuite, void * aContext)
{
    // Test this instance calling function of a static class member.
    const device_translator dev_translator(false);

    // 1 Test the correct attribute id is returned
    std::string cluster_name                      = "OnOff";
    std::string attribute_name                    = "OnOff";
    std::optional<chip::AttributeId> attribute_id = dev_translator.get_attribute_id(cluster_name, attribute_name);
    check_optional_value(inSuite, attribute_id, chip::app::Clusters::OnOff::Attributes::OnOff::Id);

    // 2 Test no attribute id is returned on unkonwn attribute
    cluster_name   = "Unknown";
    attribute_name = "Unknown";
    attribute_id   = dev_translator.get_attribute_id(cluster_name, attribute_name);
    NL_TEST_ASSERT(inSuite, attribute_id.has_value() == false);

    // 3 Test the correct command id is returned
    cluster_name                              = "OnOff";
    std::string command_name                  = "Off";
    std::optional<chip::CommandId> command_id = dev_translator.get_command_id(cluster_name, command_name);
    check_optional_value(inSuite, command_id, chip::app::Clusters::OnOff::Commands::Off::Id);

    // 4 Test no command id is returned on unkonwn command
    cluster_name = "Unknown";
    command_name = "Unknown";
    command_id   = dev_translator.get_command_id(cluster_name, command_name);
    NL_TEST_ASSERT(inSuite, command_id.has_value() == false);
}

class TestContext
{
public:
    nlTestSuite * mTestSuite;
};

static const nlTest sTests[] = {
    NL_TEST_DEF("TestMatterDeviceScore", TestMatterDeviceScore),
    NL_TEST_DEF("TestDeviceMapperPrioritization", TestDeviceMapperPrioritization),
    NL_TEST_DEF("TestDeviceMapperPrioritizationConformingToSpec", TestDeviceMapperPrioritizationConformingToSpec),
    NL_TEST_DEF("TestDeviceMapperSpecCommands", TestDeviceMapperSpecCommands),
    NL_TEST_DEF("TestDeviceMapperSpecCommandsFail", TestDeviceMapperSpecCommandsFail),
    NL_TEST_DEF("TestDeviceMapperSpecAttributes", TestDeviceMapperSpecAttributes),
    NL_TEST_DEF("TestDeviceMapperSpecAttributesFail", TestDeviceMapperSpecAttributesFail),
    NL_TEST_DEF("TestDeviceMapperSpecOnOffDeviceFail", TestDeviceMapperSpecOnOffDeviceFail),
    NL_TEST_DEF("TestDeviceMapperSpecOnOffDeviceComplying", TestDeviceMapperSpecOnOffDeviceComplying),
    NL_TEST_DEF("TestDeviceMapperGetDeviceType", TestDeviceMapperGetDeviceType),
    NL_TEST_DEF("TestDeviceMapperGetClusterId", TestDeviceMapperGetClusterId),
    NL_TEST_DEF("TestDeviceMapperGetAttributeIdAndCommandId", TestDeviceMapperGetAttributeIdAndCommandId),
    NL_TEST_SENTINEL()
};

static nlTestSuite TheCommandSuite = { "TestDeviceMapperHandler", &sTests[0], nullptr, nullptr };

int TestDeviceMapperHandler(void)
{
    return (chip::ExecuteTestsWithContext<TestContext>(&TheCommandSuite));
}

CHIP_REGISTER_TEST_SUITE(TestDeviceMapperHandler)
