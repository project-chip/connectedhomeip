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

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE

void emberZclClusterBasicServerCommandResetToFactoryDefaultsRequestHandler(const EmberZclCommandContext_t *context,
                                                                           const EmberZclClusterBasicServerCommandResetToFactoryDefaultsRequest_t *request)
{
  emberAfCorePrintln("RX: ResetToFactoryDefaults");

  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);

  emberZclResetAttributes(context->endpointId);
  emberZclReportingConfigurationsFactoryReset(context->endpointId);
}
