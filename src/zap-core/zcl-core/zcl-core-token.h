/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

// TODO: Pick creator codes.
#define CREATOR_ZCL_CORE_BINDING_TABLE                    0x7A62 // zb - ZCL/IP bindings
#define CREATOR_ZCL_CORE_GROUP_TABLE                      0x7A67 // zg - ZCL/IP groups
#define CREATOR_ZCL_CORE_REPORTING_CONFIGURATIONS_TABLE   0x7A72 // zr - ZCL/IP reporting

// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_ZCL_CORE_BINDING_TABLE                  (NVM3KEY_DOMAIN_USER | 0x8000)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_ZCL_CORE_GROUP_TABLE                    (NVM3KEY_DOMAIN_USER | 0x8080)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_ZCL_CORE_REPORTING_CONFIGURATIONS_TABLE (NVM3KEY_DOMAIN_USER | 0x8100)

#ifdef DEFINETYPES
  #include EMBER_AF_API_ZCL_CORE
  #include "zcl-core-reporting.h"
#endif

#ifdef DEFINETOKENS
DEFINE_INDEXED_TOKEN(ZCL_CORE_BINDING_TABLE,
                     EmberZclBindingEntry_t,
                     EMBER_ZCL_BINDING_TABLE_SIZE,
                     { EMBER_ZCL_ENDPOINT_NULL, })

DEFINE_INDEXED_TOKEN(ZCL_CORE_GROUP_TABLE,
                     EmberZclGroupEntry_t,
                     EMBER_ZCL_GROUP_TABLE_SIZE,
                     { EMBER_ZCL_GROUP_NULL, })

DEFINE_INDEXED_TOKEN(ZCL_CORE_REPORTING_CONFIGURATIONS_TABLE,
                     EmZclNvReportingConfiguration_t,
                     EMBER_ZCL_REPORTING_CONFIGURATIONS_TABLE_SIZE,
                     { EMBER_ZCL_ENDPOINT_NULL, })
#endif
