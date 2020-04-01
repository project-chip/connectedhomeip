/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef POLL_CONTROL_CLIENT_H
#define POLL_CONTROL_CLIENT_H

void emAfSetFastPollingMode(bool mode);
void emAfSetFastPollingTimeout(uint16_t timeout);
void emAfSetResponseMode(bool mode);
void chipZclClusterPollControlClientCommandCheckInRequestHandler(const ChipZclCommandContext_t *context,
                                                                  const ChipZclClusterPollControlClientCommandCheckInRequest_t *request);

#endif // POLL_CONTROL_CLIENT_H
