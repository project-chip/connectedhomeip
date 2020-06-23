/**
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
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief Unit tests for APS link key authorization
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "aps-link-key-authorization.h"
#include "app/framework/util/common.h"

#include "app/framework/plugin/aps-link-key-authorization/aps-link-key-authorization-test.h"
#include "app/framework/test/test-framework.h"

static uint8_t  exemptList[2 * (EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS + 1)];

// -----------------------------------------------------------------------------
// Interface for callbacks implemeted in the plugin and tested here
// -----------------------------------------------------------------------------
bool emberAfClusterSecurityCustomCallback(EmberAfProfileId profileId,
                                          EmberAfClusterId clusterId,
                                          bool incoming,
                                          uint8_t commandId);

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------
static void testApsEnableAndExemptCount(bool expectedEnableState, uint8_t expectedCount)
{
  bool apsAuthorizationEnabled = emberAfApsLinkKeyAuthorizationIsEnabled();
  uint8_t exemptClusterListCount = emberAfApsLinkKeyAuthorizationExemptClusterListCount();

  expectComparisonDecimal(
    expectedEnableState,
    apsAuthorizationEnabled,
    "expected APS authorization enable state",
    "observed APS authorization enable state");

  expectComparisonDecimal(
    expectedCount,
    exemptClusterListCount,
    "expected exempt cluster list count",
    "observer exempt cluster list count");
}

// -----------------------------------------------------------------------------
// stubbed out functions: In general, we don't care about these functions; they
// just need to compile and always return success when called at runtime
// -----------------------------------------------------------------------------

// some data types (like strings) are sent OTA in human readable order
// (how they are read) instead of little endian as the data types are.
bool emberAfIsThisDataTypeAStringType(EmberAfAttributeType dataType)
{
  return (dataType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE
          || dataType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE
          || dataType == ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE
          || dataType == ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE);
}

bool emberAfIsStringAttributeType(EmberAfAttributeType attributeType)
{
  return (attributeType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE
          || attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
}

// -----------------------------------------------------------------------------
// test cases: These are the test cases called by the test framework when unit
// tests are run.
// -----------------------------------------------------------------------------

// Verify that list is empty and can be emptied
static void emptyListTest(void)
{
  // Test empty list
  testApsEnableAndExemptCount(false, 0);

  // Add some guys to the exempt lislt and test if added
  expectMessage(EMBER_SUCCESS == emberAfApsLinkKeyAuthorizationAddClusterExempted(0xACDC), "ACDC cannot be added to the exempt list!\n");
  expectMessage(EMBER_SUCCESS == emberAfApsLinkKeyAuthorizationAddClusterExempted(0xBEEF), "BEEF cannot be added to the exempt list!\n");

  testApsEnableAndExemptCount(false, 2);

  // Clear list again
  emberAfApsLinkKeyAuthorizationClearExemptClusterList();

  testApsEnableAndExemptCount(false, 0);
}

// Verify that APS authorization can be enabled and disabled with all APIs
static void enableAuthorizationTest(void)
{
  testApsEnableAndExemptCount(false, 0);

  emberAfApsLinkKeyAuthorizationEnable(true);

  testApsEnableAndExemptCount(true, 0);

  emberAfApsLinkKeyAuthorizationEnable(false);

  testApsEnableAndExemptCount(false, 0);
}

// Verify that APS authorization exemption can added
static void addQueryRemoveExemptionTest(void)
{
  bool isClusterExempted;
  uint16_t exemptClusterIdx;

  // Load the full list with elements
  // Note: this is one more than there is space available in the exempt list of the plugin,
  // so that overload can be tested
  for (exemptClusterIdx = 0; exemptClusterIdx < (EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS + 1); exemptClusterIdx++) {
    exemptList[2 * exemptClusterIdx] = LOW_BYTE(exemptClusterIdx);
    exemptList[2 * exemptClusterIdx + 1] = HIGH_BYTE(exemptClusterIdx);
  }

  // Try to load more elements than there is space available
  expectMessage(EMBER_TABLE_FULL == emberAfApsLinkKeyAuthorizationEnableWithExemptClusterList(
                  EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS + 1, exemptList),
                "Link key authorization cluster exempt list is not supposed to be overloaded !\n");

  // Try to properly fill up the list
  expectMessage(EMBER_SUCCESS == emberAfApsLinkKeyAuthorizationEnableWithExemptClusterList(
                  EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS, exemptList),
                "Link key authorization cluster exempt list cannot be filled up !\n");

  // Test if authorization is enabled and exempt list is ok
  testApsEnableAndExemptCount(true, EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS);

  // Test if all clusters are stored properly to the exempt cluster list
  for (exemptClusterIdx = 0; exemptClusterIdx < EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS; exemptClusterIdx++) {
    isClusterExempted = emberAfApsLinkKeyAuthorizationIsClusterExempted(exemptClusterIdx);
    expectComparisonDecimal(
      true,
      isClusterExempted,
      "expected cluster exemption",
      "observer cluster exemption");
  }

  // Test for clusters existing and missing
  expectMessage(true
                == emberAfApsLinkKeyAuthorizationIsClusterExempted(0),
                "Cluster is supposed to be in the exempt list !\n");
  expectMessage(true
                == emberAfApsLinkKeyAuthorizationIsClusterExempted(10),
                "Cluster is supposed to be in the exempt list !\n");
  expectMessage(false
                == emberAfApsLinkKeyAuthorizationIsClusterExempted(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS + 1),
                "Cluster is not supposed to be in the exempt list !\n");
  expectMessage(false
                == emberAfApsLinkKeyAuthorizationIsClusterExempted(EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS + 10),
                "Cluster is not supposed to be in the exempt list !\n");
}

int main(int argc, char* argv[])
{
  const TestCase tests[] = {
    { "empty-list-test", emptyListTest },
    { "enable-authorization-test", enableAuthorizationTest },
    { "add-query-remove-exemption-test", addQueryRemoveExemptionTest },
    { NULL },
  };

  return parseCommandLineAndExecuteTest(argc, argv, "APS Link Key Authorization", tests);
}
