/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
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
