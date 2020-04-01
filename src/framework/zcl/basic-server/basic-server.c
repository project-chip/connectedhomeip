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

void chipZclClusterBasicServerCommandResetToFactoryDefaultsRequestHandler(const ChipZclCommandContext_t *context,
                                                                           const ChipZclClusterBasicServerCommandResetToFactoryDefaultsRequest_t *request)
{
  chipAfCorePrintln("RX: ResetToFactoryDefaults");

  chipZclSendDefaultResponse(context, CHIP_ZCL_STATUS_SUCCESS);

  chipZclResetAttributes(context->endpointId);
  chipZclReportingConfigurationsFactoryReset(context->endpointId);
}
