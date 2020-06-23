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
 * @brief APIs for the APS Link Key Authorization feature.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_APS_LINK_KEY_AUTHORIZATION_H
#define SILABS_APS_LINK_KEY_AUTHORIZATION_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#endif

#define emberAfApsLinkKeyAuthorizationIsRequiredOnCluster(clusterId)                     \
  /* If APS level security is enabled, the exempt list serves as a security disabler, */ \
  /* whereas, if security is disabled, the exempt list serves as a security enabler */   \
  (emberAfApsLinkKeyAuthorizationIsEnabled()                                             \
   != emberAfApsLinkKeyAuthorizationIsClusterExempted(clusterId))

bool emberAfApsLinkKeyAuthorizationIsEnabled(void);
bool emberAfApsLinkKeyAuthorizationIsClusterExempted(uint16_t clusterId);
bool emberAfApsLinkKeyAuthorizationIsExemptClusterListFull(void);
uint8_t emberAfApsLinkKeyAuthorizationExemptClusterListCount(void);
void emberAfApsLinkKeyAuthorizationGetExemptClusterList(uint8_t *exemptListCount, uint16_t *exemptClusterList);
void emberAfApsLinkKeyAuthorizationClearExemptClusterList(void);
void emberAfApsLinkKeyAuthorizationEnable(bool enable);
EmberStatus emberAfApsLinkKeyAuthorizationAddClusterExempted(uint16_t clusterId);
EmberStatus emberAfApsLinkKeyAuthorizationEnableWithExemptClusterList(uint8_t numberExemptClusters, uint8_t *exemptClusterList);
EmberStatus emberAfApsLinkKeyAuthorizationDisableWithExemptClusterList(uint8_t numberExemptClusters, uint8_t *exemptClusterList);

#endif // SILABS_APS_LINK_KEY_AUTHORIZATION_H
