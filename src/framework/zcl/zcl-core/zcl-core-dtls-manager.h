/***************************************************************************//**
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
 * @return session identifier or @ref CHIP_NULL_SESSION_ID if none found
 *
 * Sessions are created using chipZclDtlsManagerGetConnection() function.
 *
 * @sa chipZclDtlsManagerGetConnection()
 *****************************************************************************/
uint8_t chipZclDtlsManagerGetSessionIdByAddress(const ChipIpv6Address *remoteAddress,
                                                 uint16_t remotePort);

/**************************************************************************//**
 * Gets the session identifier for the given UID and port.
 *
 * @param remoteUid UID of the session to find
 * @param remotePort Port of the session to find (passing '0' will match any port)
 * @return session identifier or @ref CHIP_NULL_SESSION_ID if none found
 *
 * Sessions are created using chipZclDtlsManagerGetConnection() function.
 *
 * @sa chipZclDtlsManagerGetConnection()
 *****************************************************************************/
uint8_t chipZclDtlsManagerGetSessionIdByUid(const ChipZclUid_t *remoteUid, uint16_t remotePort);

/**************************************************************************//**
 * Gets peer UID of the given session identifier.
 *
 * @param sessionId session identifier
 * @param remoteUid pointer to UID buffer to fill on success
 * @return
 * - @ref CHIP_SUCCESS if function call was successful
 * - @ref ::ChipStatus with failure reason otherwise
 *
 * Sessions are created using chipZclDtlsManagerGetConnection() function.
 *
 * @sa chipZclDtlsManagerGetConnection()
 *****************************************************************************/
ChipStatus chipZclDtlsManagerGetUidBySessionId(const uint8_t sessionId,
                                                 ChipZclUid_t *remoteUid);

/**************************************************************************//**
 * Gets peer Address of the given session identifier.
 *
 * @param sessionId session identifier
 * @param remoteAddress pointer to Address buffer to fill on success
 * @return
 * - @ref CHIP_SUCCESS if function call was successful
 * - @ref ::ChipStatus with failure reason otherwise
 *
 * Sessions are created using chipZclDtlsManagerGetConnection() function.
 *
 * @sa chipZclDtlsManagerGetConnection()
 *****************************************************************************/
ChipStatus chipZclDtlsManagerGetAddressBySessionId(const uint8_t sessionId,
                                                     ChipIpv6Address *remoteAddress);

/**************************************************************************//**
 * Gets peer Port of the given session identifier.
 *
 * @param sessionId session identifier
 * @param remotePort pointer to Port buffer to fill on success
 * @return
 * - @ref CHIP_SUCCESS if function call was successful
 * - @ref ::ChipStatus with failure reason otherwise
 *
 * Sessions are created using chipZclDtlsManagerGetConnection() function.
 *
 * @sa chipZclDtlsManagerGetConnection()
 *****************************************************************************/
ChipStatus chipZclDtlsManagerGetPortBySessionId(const uint8_t sessionId,
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
 * - @ref CHIP_SUCCESS if function call was successful
 * - @ref ::ChipStatus with failure reason otherwise
 *
 * All sessions can be closed using chipZclDtlsManagerCloseAllConnections().
 *
 * @sa chipZclDtlsManagerCloseAllConnections()
 *****************************************************************************/
ChipStatus chipZclDtlsManagerGetConnection(const ChipIpv6Address *remoteAddress,
                                             uint16_t remotePort,
                                             ChipDtlsMode mode,
                                             void (*returnHandle)(uint8_t));

/**************************************************************************//**
 * Closes all open DTLS sessions.
 *
 * Sessions are created using chipZclDtlsManagerGetConnection() function.
 *
 * @sa chipZclDtlsManagerGetConnection()
 *****************************************************************************/
void chipZclDtlsManagerCloseAllConnections(void);

/** @} end addtogroup ZCLIP_dtls_session_manager */
/** @} end addtogroup ZCLIP */

#endif // #ifndef ZCL_CORE_DTLS_SESSION_MANAGER_H
