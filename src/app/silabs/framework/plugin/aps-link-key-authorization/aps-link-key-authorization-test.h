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
 * @brief Include file for APS link key authorization unit tests
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_APS_LINK_KEY_AUTHORIZATION_TEST_H
#define SILABS_APS_LINK_KEY_AUTHORIZATION_TEST_H

#define EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS 120

uint16_t exemptClusterList[EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS];

#define TOKEN_APS_LINK_KEY_AUTH_ENABLED
#define TOKEN_APS_LINK_KEY_AUTH_EXEMPT_LIST_CNT
#define TOKEN_APS_LINK_KEY_AUTH_EXEMPT_LIST

#undef halCommonGetToken
#define halCommonGetToken(data, token)

#undef halCommonSetToken
#define halCommonSetToken(token, data)

#undef halCommonGetIndexedToken
#define halCommonGetIndexedToken(data, token, index) \
  *(data) = exemptClusterList[index];

#undef halCommonSetIndexedToken
#define halCommonSetIndexedToken(token, index, data) \
  exemptClusterList[index] = *(data);

#endif //SILABS_APS_LINK_KEY_AUTHORIZATION_TEST_H
