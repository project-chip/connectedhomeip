/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif
#include CHIP_AF_API_ZCL_CORE

#define MILLI_TO_DECI_CONVERSION_FACTOR 100

//------------------------------------------------------------------------------
// Implemented callbacks

void chipAfPluginBatteryMonitorDataCallback(uint16_t voltageMilliV)
{
  ChipZclStatus_t status;
  uint8_t voltageDeciV;
  ChipZclEndpointId_t endpoint;
  ChipZclEndpointIndex_t i;

  chipAfCorePrintln("New voltage reading: %d mV", voltageMilliV);

  // convert from mV to 100 mV, which are the units specified by zigbee spec for
  // the power configuration cluster's voltage attribute.
  voltageDeciV = ((uint8_t) (voltageMilliV / MILLI_TO_DECI_CONVERSION_FACTOR));

  for (i = 0; i < chZclEndpointCount; i++) {
    endpoint = chipZclEndpointIndexToId(i,
                                         &chipZclClusterPowerConfigServerSpec);
    if (endpoint != CHIP_ZCL_ENDPOINT_NULL) {
      status
        = chipZclWriteAttribute(endpoint,
                                 &chipZclClusterPowerConfigServerSpec,
                                 CHIP_ZCL_CLUSTER_POWER_CONFIG_SERVER_ATTRIBUTE_BATTERY_VOLTAGE,
                                 (uint8_t *)&voltageDeciV,
                                 sizeof(voltageDeciV));
      if (status != CHIP_ZCL_STATUS_SUCCESS) {
        chipAfCorePrintln("Power Configuration Server: failed to write value "
                           "0x%x to endpoint %d",
                           voltageDeciV,
                           endpoint);
      }
    }
  }
}
