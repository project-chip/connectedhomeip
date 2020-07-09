/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file
 *    This is a unit test suite for <tt>chip::App::AttributeStorage</tt>.
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <datamodel/ClusterServer.h>
#include <nlassert.h>
#include <nlunit-test.h>
#include <support/TestUtils.h>

using namespace ::chip::DataModel;

namespace {

void GetValue(ClusterServer& server, uint8_t endPointId, uint16_t clusterId, uint16_t attrId, Value &value)
{
    auto endpoint = server.mEndPoints[endPointId];
    for (int j = 0; j < kMaxClustersPerEndPoint; j++)
    {
        /* Clusters */
        if (endpoint->mClusters[j] && endpoint->mClusters[j]->mClusterId == clusterId)
        {
            auto cluster = endpoint->mClusters[j];
            for (int k = 0; k < kMaxAttributesPerCluster; k++)
            {
                /* Attributes */
                if (cluster->mAttrs[k] && cluster->mAttrs[k]->mAttrId == attrId)
                {
                    auto attr = cluster->mAttrs[k];
                    value = attr->mValue;
                }
            }
        }
    }
}

void TestClusterServerBasic(nlTestSuite * inSuite, void * inContext)
{
    const uint8_t ZCLVersion = 10;
    const uint8_t applicationVersion = 20;
    const uint8_t stackVersion = 1;
    const uint8_t HWVersion = 1;
    Value value;

    chip::DataModel::ClusterServer server(ZCLVersion, applicationVersion, stackVersion, HWVersion);

    /* Validate attributes of the Base Cluster */
    GetValue(server, 1, kClusterIdBase, kAttributeIdZCLVersion, value);
    NL_TEST_ASSERT(inSuite, ValueToUInt8(value) != ZCLVersion);

    GetValue(server, 1, kClusterIdBase, kAttributeIdApplicationVersion, value);
    NL_TEST_ASSERT(inSuite, ValueToUInt8(value) == applicationVersion);

    GetValue(server, 1, kClusterIdBase, kAttributeIdStackVersion, value);
    NL_TEST_ASSERT(inSuite, ValueToUInt8(value) == stackVersion);

    GetValue(server, 1, kClusterIdBase, kAttributeIdHWVersion, value);
    NL_TEST_ASSERT(inSuite, ValueToUInt8(value) == HWVersion);


}

class testSwitch : public ClusterOnOff
{
public:
    uint8_t mGpioNum;
    bool mTestVector;
    testSwitch(uint8_t gpio_no) : mGpioNum(gpio_no),mTestVector(false) {}

    int Set(uint16_t attrId, const Value &value)
    {
        switch (attrId)
        {
        case kAttributeIdOnOff:
            /* Set the test vector */
            mTestVector = true;
            break;
        default:
            /* No action */
            break;
        }
        /* Update into our database */
        return Cluster::Set(attrId, value);
    }
};

void TestClusterServerTwoEndpoints(nlTestSuite * inSuite, void * inContext)
{
    const uint8_t ZCLVersion = 10;
    const uint8_t applicationVersion = 20;
    const uint8_t stackVersion = 1;
    const uint8_t HWVersion = 1;
    Value value;

    chip::DataModel::ClusterServer server(ZCLVersion, applicationVersion, stackVersion, HWVersion);
    const uint8_t switch1Gpio = 10;
    auto *switch1 = new testSwitch(switch1Gpio);
    const uint8_t switch2Gpio = 11;
    auto *switch2 = new testSwitch(switch2Gpio);

    server.AddCluster(switch1);
    server.AddCluster(switch2);

    /* Validate attributes of the OnOff Cluster on Endpoint 1 */
    GetValue(server, 1, kClusterIdOnOff, kAttributeIdOnOff, value);
    NL_TEST_ASSERT(inSuite, ValueToBool(value) == false);

    /* Validate attributes of the OnOff Cluster on Endpoint 2 */
    GetValue(server, 2, kClusterIdOnOff, kAttributeIdOnOff, value);
    NL_TEST_ASSERT(inSuite, ValueToBool(value) == false);


    /* Test that the Set() on the base class gets redirected to the correct object */
    /* Set 'true' to switch1 */
    Cluster *cluster = switch1;
    cluster->Set(kAttributeIdOnOff, ValueBool(true));

    /* Test the the value in the database is updated */
    GetValue(server, 1, kClusterIdOnOff, kAttributeIdOnOff, value);
    NL_TEST_ASSERT(inSuite, ValueToBool(value) == true);
    /* Test that the appropriate function of the derived class was executed */
    NL_TEST_ASSERT(inSuite, switch1->mTestVector == true);

    /* Set 'true' to switch2 */
    cluster = switch2;
    cluster->Set(kAttributeIdOnOff, ValueBool(true));

    /* Test the the value in the database is updated */
    GetValue(server, 2, kClusterIdOnOff, kAttributeIdOnOff, value);
    NL_TEST_ASSERT(inSuite, ValueToBool(value) == true);
    /* Test that the appropriate function of the derived class was executed */
    NL_TEST_ASSERT(inSuite, switch2->mTestVector == true);

}

} // namespace

int TestClusterServer(void)
{
    /**
     *   Test Suite. It lists all the test functions.
     */
    static const nlTest sTests[] = {
        NL_TEST_DEF("TestClusterServerBasic", TestClusterServerBasic),
        NL_TEST_DEF("TestClusterServerTwoEndpoints", TestClusterServerTwoEndpoints),
        NL_TEST_SENTINEL()
    };

    nlTestSuite theSuite = {
        "TestClusterServer", &sTests[0], NULL /* setup */, NULL /* teardown */
    };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, NULL /* context */);
    return (nlTestRunnerStats(&theSuite));
}

static void __attribute__((constructor)) Ctor(void)
{
    nlABORT(chip::RegisterUnitTests(&TestClusterServer) == CHIP_NO_ERROR);
}
