/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef ZCL_CORE_REPORTING_H
#define ZCL_CORE_REPORTING_H

// Needed for 'make test'
#ifndef CH_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES
 #ifdef CHIP_TEST
  #define CH_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES 32
 #else
  #define CH_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES 1
 #endif
#endif

// Needed for 'make test'
#ifndef CH_ZCL_MAX_SIZE_NV_REPORTABLE_ATTRIBUTES
 #ifdef CHIP_TEST
  #define CH_ZCL_MAX_SIZE_NV_REPORTABLE_ATTRIBUTES 32
 #else
  #define CH_ZCL_MAX_SIZE_NV_REPORTABLE_ATTRIBUTES 1
 #endif
#endif

#if CH_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES == 0
  #undef  CH_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES
  #define CH_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES 1
#endif

#if CH_ZCL_MAX_SIZE_NV_REPORTABLE_ATTRIBUTES == 0
  #undef  CH_ZCL_MAX_SIZE_NV_REPORTABLE_ATTRIBUTES
  #define CH_ZCL_MAX_SIZE_NV_REPORTABLE_ATTRIBUTES 1
#endif

// Define type for nv (Token based) reporting configurations.
// (The nv reporting configurations table is used to mirror the volatile reporting
// configurations in zcl-core-reporting.c:- volatile configurations are restored
// from nv at power-up, nv configurations are created/modified when the associated
// volatile configuration is modified.
// NOTE: When modifying this structure take into account NVM token space and
// backward compatibility considerations
typedef struct {
  ChipZclEndpointId_t endpointId;
  ChipZclClusterSpec_t clusterSpec;
  ChipZclReportingConfigurationId_t reportingConfigurationId;
  size_t sizeAttrFlags;
  size_t sizeReportableChanges;
  uint16_t minimumIntervalS;
  uint16_t maximumIntervalS;
  uint8_t attrFlags[CH_ZCL_MAX_SIZE_NV_REPORTABLE_ATTRIBUTES];
  uint8_t reportableChanges[CH_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES];
  uint8_t lowThresholds[CH_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES];
  uint8_t highThresholds[CH_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES];
} ChZclNvReportingConfiguration_t;

#endif // ZCL_CORE_REPORTING_H
