/***************************************************************************//**
 * @file
 * @brief
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
