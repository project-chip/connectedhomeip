/***************************************************************************//**
 *
 *    <COPYRIGHT>
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#ifndef ZCL_CORE_DTLS_SESSION_MANAGER_H
#define ZCL_CORE_DTLS_SESSION_MANAGER_H

/**
 * @addtogroup ZCLIP
 *
 * @{
 */

/**
 * @addtogroup ZCLIP_dtls_session_manager DTLS Session Manager
 *
 * See zcl-core-dtls-manager.h for source code.
 * @{
 */

/**************************************************************************//**
 * Gets the session identifier for the given address and port.
 *
 * @param remoteAddress address of the session to find
 * @param remotePort port of the session to find
 * @return session identifier or @ref EMBER_NULL_SESSION_ID if none found
 *
 * Sessions are created using emberZclDtlsManagerGetConnection() function.
 *
 * @sa emberZclDtlsManagerGetConnection()
 *****************************************************************************/
uint8_t emberZclDtlsManagerGetSessionIdByAddress(const EmberIpv6Address *remoteAddress,
                                                 uint16_t remotePort);

/**************************************************************************//**
 * Gets the session identifier for the given UID and port.
 *
 * @param remoteUid UID of the session to find
 * @param remotePort Port of the session to find (passing '0' will match any port)
 * @return session identifier or @ref EMBER_NULL_SESSION_ID if none found
 *
 * Sessions are created using emberZclDtlsManagerGetConnection() function.
 *
 * @sa emberZclDtlsManagerGetConnection()
 *****************************************************************************/
uint8_t emberZclDtlsManagerGetSessionIdByUid(const EmberZclUid_t *remoteUid, uint16_t remotePort);

/**************************************************************************//**
 * Gets peer UID of the given session identifier.
 *
 * @param sessionId session identifier
 * @param remoteUid pointer to UID buffer to fill on success
 * @return
 * - @ref EMBER_SUCCESS if function call was successful
 * - @ref ::EmberStatus with failure reason otherwise
 *
 * Sessions are created using emberZclDtlsManagerGetConnection() function.
 *
 * @sa emberZclDtlsManagerGetConnection()
 *****************************************************************************/
EmberStatus emberZclDtlsManagerGetUidBySessionId(const uint8_t sessionId,
                                                 EmberZclUid_t *remoteUid);

/**************************************************************************//**
 * Gets peer Address of the given session identifier.
 *
 * @param sessionId session identifier
 * @param remoteAddress pointer to Address buffer to fill on success
 * @return
 * - @ref EMBER_SUCCESS if function call was successful
 * - @ref ::EmberStatus with failure reason otherwise
 *
 * Sessions are created using emberZclDtlsManagerGetConnection() function.
 *
 * @sa emberZclDtlsManagerGetConnection()
 *****************************************************************************/
EmberStatus emberZclDtlsManagerGetAddressBySessionId(const uint8_t sessionId,
                                                     EmberIpv6Address *remoteAddress);

/**************************************************************************//**
 * Gets peer Port of the given session identifier.
 *
 * @param sessionId session identifier
 * @param remotePort pointer to Port buffer to fill on success
 * @return
 * - @ref EMBER_SUCCESS if function call was successful
 * - @ref ::EmberStatus with failure reason otherwise
 *
 * Sessions are created using emberZclDtlsManagerGetConnection() function.
 *
 * @sa emberZclDtlsManagerGetConnection()
 *****************************************************************************/
EmberStatus emberZclDtlsManagerGetPortBySessionId(const uint8_t sessionId,
                                                  uint16_t *remotePort);

/**************************************************************************//**
 * Looks up DTLS session for the given address and port. Opens up a new
 * session if one does not exist already.
 * returnHandle() is called with the session identifier.
 *
 * @param remoteAddress address of the session to lookup
 * @param remotePort port of the session to lookup
 * @param mode DTLS session mode
 * @param returnHandle function pointer to be called with session identifier
 * @return
 * - @ref EMBER_SUCCESS if function call was successful
 * - @ref ::EmberStatus with failure reason otherwise
 *
 * All sessions can be closed using emberZclDtlsManagerCloseAllConnections().
 *
 * @sa emberZclDtlsManagerCloseAllConnections()
 *****************************************************************************/
EmberStatus emberZclDtlsManagerGetConnection(const EmberIpv6Address *remoteAddress,
                                             uint16_t remotePort,
                                             EmberDtlsMode mode,
                                             void (*returnHandle)(uint8_t));

/**************************************************************************//**
 * Closes all open DTLS sessions.
 *
 * Sessions are created using emberZclDtlsManagerGetConnection() function.
 *
 * @sa emberZclDtlsManagerGetConnection()
 *****************************************************************************/
void emberZclDtlsManagerCloseAllConnections(void);

/** @} end addtogroup ZCLIP_dtls_session_manager */
/** @} end addtogroup ZCLIP */

#endif // #ifndef ZCL_CORE_DTLS_SESSION_MANAGER_H
