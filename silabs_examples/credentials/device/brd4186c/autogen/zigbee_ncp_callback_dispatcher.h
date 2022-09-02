/***************************************************************************//**
 * @file zigbee_ncp_callback_dispatcher.h
 * @brief ZigBee ncp dispatcher declarations.
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

#ifndef SL_ZIGBEE_NCP_CALLBACK_DISPATCHER_H
#define SL_ZIGBEE_NCP_CALLBACK_DISPATCHER_H

#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "app/em260/command-context.h" // EmberAfPluginEzspValueCommandContext


// get_value
void emberAfEzspGetValueCommandCallback(EmberAfPluginEzspValueCommandContext* context);
void emberAfPluginEzspGetValueCommandCallback(EmberAfPluginEzspValueCommandContext* context);
void emberAfPluginEzspZigbeeProGetValueCommandCallback(EmberAfPluginEzspValueCommandContext* context);
void emberAfPluginEzspSecurityGetValueCommandCallback(EmberAfPluginEzspValueCommandContext* context);




void emberAfEzspGetExtendedValueCommandCallback(EmberAfPluginEzspValueCommandContext* context);
void emberAfPluginEzspZigbeeProGetExtendedValueCommandCallback(EmberAfPluginEzspValueCommandContext* context);



void emberAfEzspSetValueCommandCallback(EmberAfPluginEzspValueCommandContext* context);
void emberAfPluginEzspSetValueCommandCallback(EmberAfPluginEzspValueCommandContext* context);
void emberAfPluginEzspZigbeeProSetValueCommandCallback(EmberAfPluginEzspValueCommandContext* context);
void emberAfPluginEzspSecuritySetValueCommandCallback(EmberAfPluginEzspValueCommandContext* context);



void emberAfEzspGetPolicyCommandCallback(EmberAfPluginEzspPolicyCommandContext* context);
void emberAfPluginEzspBindingPolicyCommandCallback(EmberAfPluginEzspPolicyCommandContext* context);
void emberAfPluginEzspPolicyCommandCallback(EmberAfPluginEzspPolicyCommandContext* context);
void emberAfPluginEzspZigbeeProPolicyCommandCallback(EmberAfPluginEzspPolicyCommandContext* context);
void emberAfPluginEzspSecurityPolicyCommandCallback(EmberAfPluginEzspPolicyCommandContext* context);



void emberAfEzspSetPolicyCommandCallback(EmberAfPluginEzspPolicyCommandContext* context);
void  emberAfPluginEzspBindingPolicyCommandCallback(EmberAfPluginEzspPolicyCommandContext* context);
void  emberAfPluginEzspPolicyCommandCallback(EmberAfPluginEzspPolicyCommandContext* context);
void  emberAfPluginEzspZigbeeProPolicyCommandCallback(EmberAfPluginEzspPolicyCommandContext* context);
void  emberAfPluginEzspSecurityPolicyCommandCallback(EmberAfPluginEzspPolicyCommandContext* context);


void emberAfEzspGetConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context);
void emberAfPluginEzspBindingConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context);
void emberAfPluginEzspGetConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context);
void emberAfPluginEzspSourceRouteGetConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context);
void emberAfPluginEzspZigbeeProGetConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context);
void emberAfPluginEzspSecurityGetConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context);



void emberAfEzspSetConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context);
void emberAfPluginEzspBindingConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context);
void emberAfPluginEzspSetConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context);
void emberAfPluginEzspSourceRouteSetConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context);
void emberAfPluginEzspZigbeeProSetConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context);
void emberAfPluginEzspSecuritySetConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context);


void emberAfPluginEzspModifyMemoryAllocationCallback(void);
void emInitializeBindingTable(void);
void emberAfPluginEzspZigbeeProModifyMemoryAllocationCallback(void);



void emberAfPluginEzspPermitHostToNcpFrameCallback(bool* permit);
void emberAfPluginEzspBindingPermitHostToNcpFrameCallback(bool* permit);

#endif // SL_ZIGBEE_NCP_CALLBACK_DISPATCHER_H