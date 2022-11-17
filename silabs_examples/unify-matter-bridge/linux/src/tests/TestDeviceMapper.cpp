#include "matter_device_translator.hpp"
#include "matter_device_types_clusters_list_updated.inc"

// Chip components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Third party library
#include <iostream>
#include <nlunit-test.h>
#include <string>
#include <unordered_map>
#include <vector>

using namespace unify::matter_bridge;

device_translator obj;
void TestDeviceMapper(nlTestSuite * inSuite, void * aContext)
{
    std::vector<const char *> clusters = {
        "Basic",
        "Identify",
        "DoorLock",
    };
    std::vector<chip::DeviceTypeId> device_type_id = obj.get_device_types(clusters);
    const char * dt                                = matter_device_type_vs_clusters_map.find(device_type_id[0])->second.second;
    std::string device_type(dt);
    NL_TEST_ASSERT(inSuite, device_type == "doorlock");
    clusters.push_back("Scenes");
    clusters.push_back("Groups");
    device_type_id    = obj.get_device_types(clusters);
    const char * dt_2 = matter_device_type_vs_clusters_map.find(device_type_id[0])->second.second;
    std::string device_type_2(dt_2);
    NL_TEST_ASSERT(inSuite, device_type_2 == "doorlock");

    std::vector<const char *> OccupancySensing_clusters = { "Basic", "Identify", "OccupancySensing", "IASZone" };
    device_type_id                                      = obj.get_device_types(OccupancySensing_clusters);
    const char * dt_Occupancy                           = matter_device_type_vs_clusters_map.find(device_type_id[0])->second.second;
    std::string device_type_occupancy(dt_Occupancy);
    NL_TEST_ASSERT(inSuite, device_type_occupancy == "occupancysensor");

    std::vector<const char *> colordimmablelight_clusters = { "Basic", "Identify", "Groups",      "Scenes",
                                                              "OnOff", "Level",    "ColorControl" };
    device_type_id                                        = obj.get_device_types(colordimmablelight_clusters);
    const char * dt_colordimmablelight = matter_device_type_vs_clusters_map.find(device_type_id[0])->second.second;
    std::string device_type_colordimmablelight(dt_colordimmablelight);
    NL_TEST_ASSERT(inSuite, device_type_colordimmablelight == "extendedcolorlight");
}

class TestContext
{
public:
    nlTestSuite * mTestSuite;
};

static const nlTest sTests[] = { NL_TEST_DEF("TestDeviceMapper", TestDeviceMapper), NL_TEST_SENTINEL() };

static nlTestSuite TheCommandSuite = { "TestDeviceMapperHandler", &sTests[0], nullptr, nullptr };

int TestDeviceMapperHandler(void)
{
    return (chip::ExecuteTestsWithContext<TestContext>(&TheCommandSuite));
}

CHIP_REGISTER_TEST_SUITE(TestDeviceMapperHandler)
