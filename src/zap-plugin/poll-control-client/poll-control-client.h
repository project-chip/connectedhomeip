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

#ifndef POLL_CONTROL_CLIENT_H
#define POLL_CONTROL_CLIENT_H

void emAfSetFastPollingMode(bool mode);
void emAfSetFastPollingTimeout(uint16_t timeout);
void emAfSetResponseMode(bool mode);
void emberZclClusterPollControlClientCommandCheckInRequestHandler(const EmberZclCommandContext_t *context,
                                                                  const EmberZclClusterPollControlClientCommandCheckInRequest_t *request);

#endif // POLL_CONTROL_CLIENT_H
