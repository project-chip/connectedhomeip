/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef POLL_CONTROL_SERVER_H
#define POLL_CONTROL_SERVER_H

void chipZclClusterPollControlServerCommandCheckInResponseHandler(ChipZclMessageStatus_t status,
                                                                   const ChipZclCommandContext_t* context,
                                                                   const ChipZclClusterPollControlClientCommandCheckInResponse_t* response);
void chipZclClusterPollControlServerCommandFastPollStopRequestHandler(const ChipZclCommandContext_t* context,
                                                                       const ChipZclClusterPollControlServerCommandFastPollStopRequest_t* request);
void chipZclClusterPollControlServerCommandSetLongPollIntervalRequestHandler(const ChipZclCommandContext_t* context,
                                                                              const ChipZclClusterPollControlServerCommandSetLongPollIntervalRequest_t* request);
void chipZclClusterPollControlServerCommandSetShortPollIntervalRequestHandler(const ChipZclCommandContext_t* context,
                                                                               const ChipZclClusterPollControlServerCommandSetShortPollIntervalRequest_t* request);
void chipZclPollControlServerInitHandler(void);

#endif // POLL_CONTROL_SERVER_H
