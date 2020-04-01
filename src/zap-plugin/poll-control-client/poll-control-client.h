/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef POLL_CONTROL_CLIENT_H
#define POLL_CONTROL_CLIENT_H

void emAfSetFastPollingMode(bool mode);
void emAfSetFastPollingTimeout(uint16_t timeout);
void emAfSetResponseMode(bool mode);
void emberZclClusterPollControlClientCommandCheckInRequestHandler(const EmberZclCommandContext_t *context,
                                                                  const EmberZclClusterPollControlClientCommandCheckInRequest_t *request);

#endif // POLL_CONTROL_CLIENT_H
