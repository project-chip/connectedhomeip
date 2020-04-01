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

#ifndef ZCL_CORE_REPORTING_H
#define ZCL_CORE_REPORTING_H

// Needed for 'make test'
#ifndef EM_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES
 #ifdef EMBER_TEST
  #define EM_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES 32
 #else
  #define EM_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES 1
 #endif
#endif

// Needed for 'make test'
#ifndef EM_ZCL_MAX_SIZE_NV_REPORTABLE_ATTRIBUTES
 #ifdef EMBER_TEST
  #define EM_ZCL_MAX_SIZE_NV_REPORTABLE_ATTRIBUTES 32
 #else
  #define EM_ZCL_MAX_SIZE_NV_REPORTABLE_ATTRIBUTES 1
 #endif
#endif

#if EM_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES == 0
  #undef  EM_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES
  #define EM_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES 1
#endif

#if EM_ZCL_MAX_SIZE_NV_REPORTABLE_ATTRIBUTES == 0
  #undef  EM_ZCL_MAX_SIZE_NV_REPORTABLE_ATTRIBUTES
  #define EM_ZCL_MAX_SIZE_NV_REPORTABLE_ATTRIBUTES 1
#endif

// Define type for nv (Token based) reporting configurations.
// (The nv reporting configurations table is used to mirror the volatile reporting
// configurations in zcl-core-reporting.c:- volatile configurations are restored
// from nv at power-up, nv configurations are created/modified when the associated
// volatile configuration is modified.
// NOTE: When modifying this structure take into account NVM token space and
// backward compatibility considerations
typedef struct {
  EmberZclEndpointId_t endpointId;
  EmberZclClusterSpec_t clusterSpec;
  EmberZclReportingConfigurationId_t reportingConfigurationId;
  size_t sizeAttrFlags;
  size_t sizeReportableChanges;
  uint16_t minimumIntervalS;
  uint16_t maximumIntervalS;
  uint8_t attrFlags[EM_ZCL_MAX_SIZE_NV_REPORTABLE_ATTRIBUTES];
  uint8_t reportableChanges[EM_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES];
  uint8_t lowThresholds[EM_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES];
  uint8_t highThresholds[EM_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES];
} EmZclNvReportingConfiguration_t;

#endif // ZCL_CORE_REPORTING_H
