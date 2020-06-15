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
 * @brief Implementation of the APS Link Key Authorization feature, which sets
 *        APS encryption requirement when interfacing defined clusters.
 *******************************************************************************
   ******************************************************************************/

#include "af.h"

#include "aps-link-key-authorization.h"

#ifdef EMBER_SCRIPTED_TEST
#include "app/framework/util/af-main.h"
#include "app/framework/plugin/aps-link-key-authorization/aps-link-key-authorization-test.h"
#endif

#include "app/framework/util/common.h"

static bool apsLinkKeyAuthorizationEnabled = false;
static uint8_t exemptClusterCount = 0;

// -----------------------------------------------------------------------------
// Helper functions

// -----------------------------------------------------------------------------
// Public functions

bool emberAfApsLinkKeyAuthorizationIsEnabled(void)
{
  halCommonGetToken(&apsLinkKeyAuthorizationEnabled, TOKEN_APS_LINK_KEY_AUTH_ENABLED);
  return apsLinkKeyAuthorizationEnabled;
}

bool emberAfApsLinkKeyAuthorizationIsExemptClusterListFull(void)
{
  halCommonGetToken(&exemptClusterCount, TOKEN_APS_LINK_KEY_AUTH_EXEMPT_LIST_CNT);
  return (exemptClusterCount >= EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS);
}

uint8_t emberAfApsLinkKeyAuthorizationExemptClusterListCount(void)
{
  halCommonGetToken(&exemptClusterCount, TOKEN_APS_LINK_KEY_AUTH_EXEMPT_LIST_CNT);
  return exemptClusterCount;
}

void emberAfApsLinkKeyAuthorizationGetExemptClusterList(uint8_t *exemptListCount, uint16_t *exemptClusterList)
{
  uint16_t idx;
  uint16_t tmpCluster;

  halCommonGetToken(&exemptClusterCount, TOKEN_APS_LINK_KEY_AUTH_EXEMPT_LIST_CNT);
  *exemptListCount = exemptClusterCount;

  for (idx = 0; idx < *exemptListCount; idx++) {
    halCommonGetIndexedToken(&tmpCluster, TOKEN_APS_LINK_KEY_AUTH_EXEMPT_LIST, idx);
    exemptClusterList[idx] = tmpCluster;
  }
}

void emberAfApsLinkKeyAuthorizationClearExemptClusterList(void)
{
  exemptClusterCount = 0;
  halCommonSetToken(TOKEN_APS_LINK_KEY_AUTH_EXEMPT_LIST_CNT, &exemptClusterCount);
}

void emberAfApsLinkKeyAuthorizationEnable(bool enable)
{
  apsLinkKeyAuthorizationEnabled = enable;
  halCommonSetToken(TOKEN_APS_LINK_KEY_AUTH_ENABLED, &apsLinkKeyAuthorizationEnabled);
}

bool emberAfApsLinkKeyAuthorizationIsClusterExempted(uint16_t clusterId)
{
  uint16_t idx = 0;
  uint16_t tmpCluster;

  halCommonGetToken(&exemptClusterCount, TOKEN_APS_LINK_KEY_AUTH_EXEMPT_LIST_CNT);

  while (idx < exemptClusterCount) {
    halCommonGetIndexedToken(&tmpCluster, TOKEN_APS_LINK_KEY_AUTH_EXEMPT_LIST, idx);
    if (clusterId == tmpCluster) {
      return true;
    }
    idx++;
  }

  return false;
}

EmberStatus emberAfApsLinkKeyAuthorizationAddClusterExempted(uint16_t clusterId)
{
  // Do not add twice
  if (emberAfApsLinkKeyAuthorizationIsClusterExempted(clusterId)) {
    return EMBER_SUCCESS;
  }

  // Have more space?
  if (emberAfApsLinkKeyAuthorizationIsExemptClusterListFull()) {
    return EMBER_TABLE_FULL;
  }

  // Add clusterId to the end of the list and increment list count
  halCommonGetToken(&exemptClusterCount, TOKEN_APS_LINK_KEY_AUTH_EXEMPT_LIST_CNT);
  halCommonSetIndexedToken(TOKEN_APS_LINK_KEY_AUTH_EXEMPT_LIST, exemptClusterCount, &clusterId);
  exemptClusterCount++;
  halCommonSetToken(TOKEN_APS_LINK_KEY_AUTH_EXEMPT_LIST_CNT, &exemptClusterCount);

  return EMBER_SUCCESS;
}

EmberStatus emberAfApsLinkKeyAuthorizationEnableWithExemptClusterList(
  uint8_t numberExemptClusters,
  uint8_t* exemptClusterList)
{
  EmberStatus retVal;

  // Clear full exempt list
  emberAfApsLinkKeyAuthorizationClearExemptClusterList();

  // Enable APS link key authorization for all clusters...
  emberAfApsLinkKeyAuthorizationEnable(true);

  // ... but for the ones on the exempt list
  while (numberExemptClusters) {
    retVal = emberAfApsLinkKeyAuthorizationAddClusterExempted(
      HIGH_LOW_TO_INT(exemptClusterList[2 * numberExemptClusters - 1], exemptClusterList[2 * numberExemptClusters - 2]));

    if (retVal != EMBER_SUCCESS) {
      return retVal;
    }

    numberExemptClusters--;
  }

  return EMBER_SUCCESS;
}

EmberStatus emberAfApsLinkKeyAuthorizationDisableWithExemptClusterList(
  uint8_t numberExemptClusters,
  uint8_t* exemptClusterList)
{
  EmberStatus retVal;

  // Clear full exempt list
  emberAfApsLinkKeyAuthorizationClearExemptClusterList();

  // Disable APS link key authorization for all clusters...
  emberAfApsLinkKeyAuthorizationEnable(false);

  // ... but for the ones on the exempt list
  while (numberExemptClusters) {
    retVal = emberAfApsLinkKeyAuthorizationAddClusterExempted(
      HIGH_LOW_TO_INT(exemptClusterList[2 * numberExemptClusters - 1], exemptClusterList[2 * numberExemptClusters - 2]));

    if (retVal != EMBER_SUCCESS) {
      return EMBER_ERR_FATAL;
    }

    numberExemptClusters--;
  }

  return EMBER_SUCCESS;
}

// Implement Cluster Security for APS Link Key Authorization
bool emberAfClusterSecurityCustomCallback(EmberAfProfileId profileId,
                                          EmberAfClusterId clusterId,
                                          bool incoming,
                                          uint8_t commandId)
{
  return (emberAfApsLinkKeyAuthorizationIsRequiredOnCluster(clusterId));
}
