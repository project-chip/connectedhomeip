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

#ifndef POLL_CONTROL_SERVER_H
#define POLL_CONTROL_SERVER_H

void emberZclClusterPollControlServerCommandCheckInResponseHandler(EmberZclMessageStatus_t status,
                                                                   const EmberZclCommandContext_t* context,
                                                                   const EmberZclClusterPollControlClientCommandCheckInResponse_t* response);
void emberZclClusterPollControlServerCommandFastPollStopRequestHandler(const EmberZclCommandContext_t* context,
                                                                       const EmberZclClusterPollControlServerCommandFastPollStopRequest_t* request);
void emberZclClusterPollControlServerCommandSetLongPollIntervalRequestHandler(const EmberZclCommandContext_t* context,
                                                                              const EmberZclClusterPollControlServerCommandSetLongPollIntervalRequest_t* request);
void emberZclClusterPollControlServerCommandSetShortPollIntervalRequestHandler(const EmberZclCommandContext_t* context,
                                                                               const EmberZclClusterPollControlServerCommandSetShortPollIntervalRequest_t* request);
void emberZclPollControlServerInitHandler(void);

#endif // POLL_CONTROL_SERVER_H
