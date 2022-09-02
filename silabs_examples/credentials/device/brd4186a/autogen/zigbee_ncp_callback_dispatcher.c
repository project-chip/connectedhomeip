/***************************************************************************//**
 * @file zigbee_ncp_callback_dispatcher.c
 * @brief ZigBee ncp dispatcher definitions.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "stack/include/ember.h"
#include "zigbee_ncp_callback_dispatcher.h"


void emberAfEzspGetValueCommandCallback(EmberAfPluginEzspValueCommandContext* context)
{
  emberAfPluginEzspGetValueCommandCallback(context);
  emberAfPluginEzspZigbeeProGetValueCommandCallback(context);
  emberAfPluginEzspSecurityGetValueCommandCallback(context);
}


void emberAfEzspGetExtendedValueCommandCallback(EmberAfPluginEzspValueCommandContext* context)
{
  emberAfPluginEzspZigbeeProGetExtendedValueCommandCallback(context);
}


void emberAfEzspSetValueCommandCallback(EmberAfPluginEzspValueCommandContext* context)
{
  emberAfPluginEzspSetValueCommandCallback(context);
  emberAfPluginEzspZigbeeProSetValueCommandCallback(context);
  emberAfPluginEzspSecuritySetValueCommandCallback(context);
}


void emberAfEzspGetPolicyCommandCallback(EmberAfPluginEzspPolicyCommandContext* context)
{
  emberAfPluginEzspBindingPolicyCommandCallback(context);
  emberAfPluginEzspPolicyCommandCallback(context);
  emberAfPluginEzspZigbeeProPolicyCommandCallback(context);
  emberAfPluginEzspSecurityPolicyCommandCallback(context);
}


void emberAfEzspSetPolicyCommandCallback(EmberAfPluginEzspPolicyCommandContext* context)
{
  emberAfPluginEzspBindingPolicyCommandCallback(context);
  emberAfPluginEzspPolicyCommandCallback(context);
  emberAfPluginEzspZigbeeProPolicyCommandCallback(context);
  emberAfPluginEzspSecurityPolicyCommandCallback(context);
}


void emberAfEzspGetConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context)
{
  emberAfPluginEzspBindingConfigurationValueCommandCallback(context);
  emberAfPluginEzspGetConfigurationValueCommandCallback(context);
  emberAfPluginEzspSourceRouteGetConfigurationValueCommandCallback(context);
  emberAfPluginEzspZigbeeProGetConfigurationValueCommandCallback(context);
  emberAfPluginEzspSecurityGetConfigurationValueCommandCallback(context);
}


void emberAfEzspSetConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context)
{
  emberAfPluginEzspBindingConfigurationValueCommandCallback(context);
  emberAfPluginEzspSetConfigurationValueCommandCallback(context);
  emberAfPluginEzspSourceRouteSetConfigurationValueCommandCallback(context);
  emberAfPluginEzspZigbeeProSetConfigurationValueCommandCallback(context);
  emberAfPluginEzspSecuritySetConfigurationValueCommandCallback(context);
}


void emberAfPluginEzspModifyMemoryAllocationCallback(void)
{
  emInitializeBindingTable();
  emberAfPluginEzspZigbeeProModifyMemoryAllocationCallback();
}


void emberAfPluginEzspPermitHostToNcpFrameCallback(bool* permit)
{
  emberAfPluginEzspBindingPermitHostToNcpFrameCallback(permit);
}
