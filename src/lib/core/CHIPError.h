/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 */

/**
 *    @file
 *      This file defines error constants for the CHIP core
 *      subsystem.
 *
 *      Error types, ranges, and mappings overrides may be made by
 *      defining the appropriate CHIP_CONFIG_* or _CHIP_CONFIG_*
 *      macros.
 *
 *  NOTE WELL: On some platforms, this header is included by C-language programs.
 *
 */

#pragma once

#include <core/CHIPConfig.h>
// clang-format off

/**
 *  @def CHIP_NO_ERROR
 *
 *  @brief
 *    This defines the CHIP error code for success or no error.
 *    This value may be configured via #CHIP_CONFIG_NO_ERROR."
 *
 */
#define CHIP_NO_ERROR                                          CHIP_CONFIG_NO_ERROR

/**
 *  @def CHIP_ERROR_MIN
 *
 *  @brief
 *    This defines the bottom or minimum CHIP error number range.
 *    This value may be configured via #CHIP_CONFIG_ERROR_MIN.
 *
 */
#define CHIP_ERROR_MIN                                         CHIP_CONFIG_ERROR_MIN

/**
 *  @def CHIP_ERROR_MAX
 *
 *  @brief
 *    This defines the top or maximum CHIP error number range.
 *    This value may be configured via #CHIP_CONFIG_ERROR_MAX.
 *
 */
#define CHIP_ERROR_MAX                                         CHIP_CONFIG_ERROR_MAX

/**
 *  @def _CHIP_ERROR(e)
 *
 *  @brief
 *    This defines a mapping function for CHIP errors that allows
 *    mapping such errors into a platform- or system-specific range.
 *    This function may be configured via #_CHIP_CONFIG_ERROR(e).
 *
 *  @param[in] e       The CHIP error to map.
 *
 *  @return    The mapped CHIP error.
 *
 *
 */
#define _CHIP_ERROR(e)                                         _CHIP_CONFIG_ERROR(e)

/**
 *  The basic type for all CHIP errors.
 *
 *  @brief
 *    This is defined to a platform- or system-specific type.
 *
 */
typedef CHIP_CONFIG_ERROR_TYPE CHIP_ERROR;

/**
 *  @name Error Definitions
 *
 *  @{
 */

/**
 *  @def CHIP_ERROR_TOO_MANY_CONNECTIONS
 *
 *  @brief
 *    The attempt to allocate a connection object failed because too many
 *    connections exist.
 *
 */
#define CHIP_ERROR_TOO_MANY_CONNECTIONS                        _CHIP_ERROR(0)

/**
 *  @def CHIP_ERROR_SENDING_BLOCKED
 *
 *  @brief
 *    A message exceeds the sent limit.
 *
 */
#define CHIP_ERROR_SENDING_BLOCKED                             _CHIP_ERROR(1)

/**
 *  @def CHIP_ERROR_CONNECTION_ABORTED
 *
 *  @brief
 *    A connection has been aborted.
 *
 */
#define CHIP_ERROR_CONNECTION_ABORTED                          _CHIP_ERROR(2)

/**
 *  @def CHIP_ERROR_INCORRECT_STATE
 *
 *  @brief
 *    An unexpected state was encountered.
 *
 */
#define CHIP_ERROR_INCORRECT_STATE                             _CHIP_ERROR(3)

/**
 *  @def CHIP_ERROR_MESSAGE_TOO_LONG
 *
 *  @brief
 *    A message is too long.
 *
 */
#define CHIP_ERROR_MESSAGE_TOO_LONG                            _CHIP_ERROR(4)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_EXCHANGE_VERSION
 *
 *  @brief
 *    An exchange version is not supported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_EXCHANGE_VERSION                _CHIP_ERROR(5)

/**
 *  @def CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS
 *
 *  @brief
 *    The attempt to register an unsolicited message handler failed because the
 *    unsolicited message handler pool is full.
 *
 */
#define CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS       _CHIP_ERROR(6)

/**
 *  @def CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER
 *
 *  @brief
 *    The attempt to unregister an unsolicited message handler failed because
 *    the target handler was not found in the unsolicited message handler pool.
 *
 */
#define CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER              _CHIP_ERROR(7)

/**
 *  @def CHIP_ERROR_NO_CONNECTION_HANDLER
 *
 *  @brief
 *    No callback has been registered for handling a connection.
 *
 */
#define CHIP_ERROR_NO_CONNECTION_HANDLER                       _CHIP_ERROR(8)

/**
 *  @def CHIP_ERROR_TOO_MANY_PEER_NODES
 *
 *  @brief
 *    The number of peer nodes exceeds the maximum limit of a local node.
 *
 */
#define CHIP_ERROR_TOO_MANY_PEER_NODES                         _CHIP_ERROR(9)

/**
 *  @def CHIP_ERROR_NO_MEMORY
 *
 *  @brief
 *    The attempt to allocate a buffer or object failed due to a lack of memory.
 *
 */
#define CHIP_ERROR_NO_MEMORY                                   _CHIP_ERROR(11)

/**
 *  @def CHIP_ERROR_NO_MESSAGE_HANDLER
 *
 *  @brief
 *    No callback has been registered for handling a message.
 *
 */
#define CHIP_ERROR_NO_MESSAGE_HANDLER                          _CHIP_ERROR(12)

/**
 *  @def CHIP_ERROR_MESSAGE_INCOMPLETE
 *
 *  @brief
 *    A message is incomplete.
 *
 */
#define CHIP_ERROR_MESSAGE_INCOMPLETE                          _CHIP_ERROR(13)

/**
 *  @def CHIP_ERROR_DATA_NOT_ALIGNED
 *
 *  @brief
 *    The data is not aligned.
 *
 */
#define CHIP_ERROR_DATA_NOT_ALIGNED                            _CHIP_ERROR(14)

/**
 *  @def CHIP_ERROR_UNKNOWN_KEY_TYPE
 *
 *  @brief
 *    The encryption key type is unknown.
 *
 */
#define CHIP_ERROR_UNKNOWN_KEY_TYPE                            _CHIP_ERROR(15)

/**
 *  @def CHIP_ERROR_KEY_NOT_FOUND
 *
 *  @brief
 *    The encryption key is not found.
 *
 */
#define CHIP_ERROR_KEY_NOT_FOUND                               _CHIP_ERROR(16)

/**
 *  @def CHIP_ERROR_WRONG_ENCRYPTION_TYPE
 *
 *  @brief
 *    The encryption type is incorrect for the specified key.
 *
 */
#define CHIP_ERROR_WRONG_ENCRYPTION_TYPE                       _CHIP_ERROR(17)

/**
 *  @def CHIP_ERROR_TOO_MANY_KEYS
 *
 *  @brief
 *    The attempt to allocate a key failed because the number of active keys
 *    exceeds the maximum limit.
 *
 */
#define CHIP_ERROR_TOO_MANY_KEYS                               _CHIP_ERROR(18)

/**
 *  @def CHIP_ERROR_INTEGRITY_CHECK_FAILED
 *
 *  @brief
 *    The integrity check in the message does not match the expected integrity
 *    check.
 *
 */
#define CHIP_ERROR_INTEGRITY_CHECK_FAILED                      _CHIP_ERROR(19)

/**
 *  @def CHIP_ERROR_INVALID_SIGNATURE
 *
 *  @brief
 *    Invalid signature.
 *
 */
#define CHIP_ERROR_INVALID_SIGNATURE                           _CHIP_ERROR(20)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_MESSAGE_VERSION
 *
 *  @brief
 *    A message version is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_MESSAGE_VERSION                 _CHIP_ERROR(21)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE
 *
 *  @brief
 *    An encryption type is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE                 _CHIP_ERROR(22)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_SIGNATURE_TYPE
 *
 *  @brief
 *    A signature type is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_SIGNATURE_TYPE                  _CHIP_ERROR(23)

/**
 *  @def CHIP_ERROR_INVALID_MESSAGE_LENGTH
 *
 *  @brief
 *    A message length is invalid.
 *
 */
#define CHIP_ERROR_INVALID_MESSAGE_LENGTH                      _CHIP_ERROR(24)

/**
 *  @def CHIP_ERROR_BUFFER_TOO_SMALL
 *
 *  @brief
 *    A buffer is too small.
 *
 */
#define CHIP_ERROR_BUFFER_TOO_SMALL                            _CHIP_ERROR(25)

/**
 *  @def CHIP_ERROR_DUPLICATE_KEY_ID
 *
 *  @brief
 *    A key id is duplicate.
 *
 */
#define CHIP_ERROR_DUPLICATE_KEY_ID                            _CHIP_ERROR(26)

/**
 *  @def CHIP_ERROR_WRONG_KEY_TYPE
 *
 *  @brief
 *    A key type does not match the expected key type.
 *
 */
#define CHIP_ERROR_WRONG_KEY_TYPE                              _CHIP_ERROR(27)

/**
 *  @def CHIP_ERROR_WELL_UNINITIALIZED
 *
 *  @brief
 *    A requested object is uninitialized.
 *
 */
#define CHIP_ERROR_WELL_UNINITIALIZED                          _CHIP_ERROR(28)

/**
 *  @def CHIP_ERROR_WELL_EMPTY
 *
 *  @brief
 *    A requested object is empty.
 *
 */
#define CHIP_ERROR_WELL_EMPTY                                  _CHIP_ERROR(29)

/**
 *  @def CHIP_ERROR_INVALID_STRING_LENGTH
 *
 *  @brief
 *    A string length is invalid.
 *
 */
#define CHIP_ERROR_INVALID_STRING_LENGTH                       _CHIP_ERROR(30)

/**
 *  @def CHIP_ERROR_INVALID_LIST_LENGTH
 *
 *  @brief
 *    A list length is invalid.
 *
 */
#define CHIP_ERROR_INVALID_LIST_LENGTH                         _CHIP_ERROR(31)

/**
 *  @def CHIP_ERROR_INVALID_INTEGRITY_TYPE
 *
 *  @brief
 *    An integrity type is invalid.
 *
 */
#define CHIP_ERROR_INVALID_INTEGRITY_TYPE                      _CHIP_ERROR(32)

/**
 *  @def CHIP_END_OF_TLV
 *
 *  @brief
 *    The end of a TLV encoding,
 *    or the end of a TLV container element has been reached.
 *
 */
#define CHIP_END_OF_TLV                                        _CHIP_ERROR(33)

/**
 *  @def CHIP_ERROR_TLV_UNDERRUN
 *
 *  @brief
 *    The TLV encoding ended prematurely.
 *
 */
#define CHIP_ERROR_TLV_UNDERRUN                                _CHIP_ERROR(34)

/**
 *  @def CHIP_ERROR_INVALID_TLV_ELEMENT
 *
 *  @brief
 *    A TLV element is invalid.
 *
 */
#define CHIP_ERROR_INVALID_TLV_ELEMENT                         _CHIP_ERROR(35)

/**
 *  @def CHIP_ERROR_INVALID_TLV_TAG
 *
 *  @brief
 *    A TLV tag is invalid.
 *
 */
#define CHIP_ERROR_INVALID_TLV_TAG                             _CHIP_ERROR(36)

/**
 *  @def CHIP_ERROR_UNKNOWN_IMPLICIT_TLV_TAG
 *
 *  @brief
 *    An implicitly encoded TLV tag was encountered,
 *    but an implicit profile id has not been defined.
 *
 */
#define CHIP_ERROR_UNKNOWN_IMPLICIT_TLV_TAG                    _CHIP_ERROR(37)

/**
 *  @def CHIP_ERROR_WRONG_TLV_TYPE
 *
 *  @brief
 *    A TLV type is wrong.
 *
 */
#define CHIP_ERROR_WRONG_TLV_TYPE                              _CHIP_ERROR(38)

/**
 *  @def CHIP_ERROR_TLV_CONTAINER_OPEN
 *
 *  @brief
 *    A TLV container is unexpectedly open.
 *
 */
#define CHIP_ERROR_TLV_CONTAINER_OPEN                          _CHIP_ERROR(39)

/**
 *  @def CHIP_ERROR_INVALID_TRANSFER_MODE
 *
 *  @brief
 *    A transfer mode is invalid.
 *
 */
#define CHIP_ERROR_INVALID_TRANSFER_MODE                       _CHIP_ERROR(40)

/**
 *  @def CHIP_ERROR_INVALID_PROFILE_ID
 *
 *  @brief
 *    A profile id is invalid.
 *
 */
#define CHIP_ERROR_INVALID_PROFILE_ID                          _CHIP_ERROR(41)

/**
 *  @def CHIP_ERROR_INVALID_MESSAGE_TYPE
 *
 *  @brief
 *    A message type is invalid.
 *
 */
#define CHIP_ERROR_INVALID_MESSAGE_TYPE                        _CHIP_ERROR(42)

/**
 *  @def CHIP_ERROR_UNEXPECTED_TLV_ELEMENT
 *
 *  @brief
 *    An unexpected TLV element was encountered.
 *
 */
#define CHIP_ERROR_UNEXPECTED_TLV_ELEMENT                      _CHIP_ERROR(43)

/**
 *  @def CHIP_ERROR_STATUS_REPORT_RECEIVED
 *
 *  @brief
 *    A status report is received from a peer node.
 *
 */
#define CHIP_ERROR_STATUS_REPORT_RECEIVED                      _CHIP_ERROR(44)

/**
 *  @def CHIP_ERROR_NOT_IMPLEMENTED
 *
 *  @brief
 *    A requested function or feature is not implemented.
 *
 */
#define CHIP_ERROR_NOT_IMPLEMENTED                             _CHIP_ERROR(45)

/**
 *  @def CHIP_ERROR_INVALID_ADDRESS
 *
 *  @brief
 *    An address is invalid.
 *
 */
#define CHIP_ERROR_INVALID_ADDRESS                             _CHIP_ERROR(46)

/**
 *  @def CHIP_ERROR_INVALID_ARGUMENT
 *
 *  @brief
 *    An argument is invalid.
 *
 */
#define CHIP_ERROR_INVALID_ARGUMENT                            _CHIP_ERROR(47)

/**
 *  @def CHIP_ERROR_INVALID_PATH_LIST
 *
 *  @brief
 *    A TLV path list is invalid.
 *
 */
#define CHIP_ERROR_INVALID_PATH_LIST                           _CHIP_ERROR(48)

/**
 *  @def CHIP_ERROR_INVALID_DATA_LIST
 *
 *  @brief
 *    A TLV data list is invalid.
 *
 */
#define CHIP_ERROR_INVALID_DATA_LIST                           _CHIP_ERROR(49)

/**
 *  @def CHIP_ERROR_TIMEOUT
 *
 *  @brief
 *    A request timed out.
 *
 */
#define CHIP_ERROR_TIMEOUT                                     _CHIP_ERROR(50)

/**
 *  @def CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR
 *
 *  @brief
 *    A device descriptor is invalid.
 *
 */
#define CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR                   _CHIP_ERROR(51)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_DEVICE_DESCRIPTOR_VERSION
 *
 *  @brief
 *    A device descriptor version is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_DEVICE_DESCRIPTOR_VERSION       _CHIP_ERROR(52)

/**
 *  @def CHIP_END_OF_INPUT
 *
 *  @brief
 *    An input ended.
 *
 */
#define CHIP_END_OF_INPUT                                      _CHIP_ERROR(53)

/**
 *  @def CHIP_ERROR_RATE_LIMIT_EXCEEDED
 *
 *  @brief
 *    A rate limit is exceeded.
 *
 */
#define CHIP_ERROR_RATE_LIMIT_EXCEEDED                         _CHIP_ERROR(54)

/**
 *  @def CHIP_ERROR_SECURITY_MANAGER_BUSY
 *
 *  @brief
 *    A security manager is busy.
 *
 */
#define CHIP_ERROR_SECURITY_MANAGER_BUSY                       _CHIP_ERROR(55)

/**
 *  @def CHIP_ERROR_INVALID_PASE_PARAMETER
 *
 *  @brief
 *    A PASE parameter is invalid.
 *
 */
#define CHIP_ERROR_INVALID_PASE_PARAMETER                      _CHIP_ERROR(56)

/**
 *  @def CHIP_ERROR_PASE_SUPPORTS_ONLY_CONFIG1
 *
 *  @brief
 *    PASE supports only config1.
 *
 */
#define CHIP_ERROR_PASE_SUPPORTS_ONLY_CONFIG1                  _CHIP_ERROR(57)

/**
 *  @def CHIP_ERROR_KEY_CONFIRMATION_FAILED
 *
 *  @brief
 *    A key confirmation failed.
 *
 */
#define CHIP_ERROR_KEY_CONFIRMATION_FAILED                     _CHIP_ERROR(58)

/**
 *  @def CHIP_ERROR_INVALID_USE_OF_SESSION_KEY
 *
 *  @brief
 *    A use of session key is invalid.
 *
 */
#define CHIP_ERROR_INVALID_USE_OF_SESSION_KEY                  _CHIP_ERROR(59)

/**
 *  @def CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY
 *
 *  @brief
 *    A connection is closed unexpectedly.
 *
 */
#define CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY              _CHIP_ERROR(60)

/**
 *  @def CHIP_ERROR_MISSING_TLV_ELEMENT
 *
 *  @brief
 *    A TLV element is missing.
 *
 */
#define CHIP_ERROR_MISSING_TLV_ELEMENT                         _CHIP_ERROR(61)

/**
 *  @def CHIP_ERROR_RANDOM_DATA_UNAVAILABLE
 *
 *  @brief
 *    Secure random data is not available.
 *
 */
#define CHIP_ERROR_RANDOM_DATA_UNAVAILABLE                     _CHIP_ERROR(62)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_HOST_PORT_ELEMENT
 *
 *  @brief
 *    A type in host/port list is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_HOST_PORT_ELEMENT               _CHIP_ERROR(63)

/**
 *  @def CHIP_ERROR_INVALID_HOST_SUFFIX_INDEX
 *
 *  @brief
 *    A suffix index in host/port list is invalid.
 *
 */
#define CHIP_ERROR_INVALID_HOST_SUFFIX_INDEX                   _CHIP_ERROR(64)

/**
 *  @def CHIP_ERROR_HOST_PORT_LIST_EMPTY
 *
 *  @brief
 *    A host/port list is empty.
 *
 */
#define CHIP_ERROR_HOST_PORT_LIST_EMPTY                        _CHIP_ERROR(65)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_AUTH_MODE
 *
 *  @brief
 *    An authentication mode is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_AUTH_MODE                       _CHIP_ERROR(66)

/**
 *  @def CHIP_ERROR_INVALID_SERVICE_EP
 *
 *  @brief
 *    A service endpoint is invalid.
 *
 */
#define CHIP_ERROR_INVALID_SERVICE_EP                          _CHIP_ERROR(67)

/**
 *  @def CHIP_ERROR_INVALID_DIRECTORY_ENTRY_TYPE
 *
 *  @brief
 *    A directory entry type is unknown.
 *
 */
#define CHIP_ERROR_INVALID_DIRECTORY_ENTRY_TYPE                _CHIP_ERROR(68)

/**
 *  @def CHIP_ERROR_FORCED_RESET
 *
 *  @brief
 *    A service manager is forced to reset.
 *
 */
#define CHIP_ERROR_FORCED_RESET                                _CHIP_ERROR(69)

/**
 *  @def CHIP_ERROR_NO_ENDPOINT
 *
 *  @brief
 *    No endpoint is available.
 *
 */
#define CHIP_ERROR_NO_ENDPOINT                                 _CHIP_ERROR(70)

/**
 *  @def CHIP_ERROR_INVALID_DESTINATION_NODE_ID
 *
 *  @brief
 *    A destination node id is invalid.
 *
 */
#define CHIP_ERROR_INVALID_DESTINATION_NODE_ID                 _CHIP_ERROR(71)

/**
 *  @def CHIP_ERROR_NOT_CONNECTED
 *
 *  @brief
 *    The operation cannot be performed because the underlying object is not
 *    connected.
 *
 */
#define CHIP_ERROR_NOT_CONNECTED                               _CHIP_ERROR(72)

/**
 *  @def CHIP_ERROR_NO_SW_UPDATE_AVAILABLE
 *
 *  @brief
 *    No software update is available.
 *
 */
#define CHIP_ERROR_NO_SW_UPDATE_AVAILABLE                      _CHIP_ERROR(73)

/**
 *  @def CHIP_ERROR_CA_CERT_NOT_FOUND
 *
 *  @brief
 *    CA certificate is not found.
 *
 */
#define CHIP_ERROR_CA_CERT_NOT_FOUND                           _CHIP_ERROR(74)

/**
 *  @def CHIP_ERROR_CERT_PATH_LEN_CONSTRAINT_EXCEEDED
 *
 *  @brief
 *    A certificate path length exceeds the constraint.
 *
 */
#define CHIP_ERROR_CERT_PATH_LEN_CONSTRAINT_EXCEEDED           _CHIP_ERROR(75)

/**
 *  @def CHIP_ERROR_CERT_PATH_TOO_LONG
 *
 *  @brief
 *    A certificate path is too long.
 *
 */
#define CHIP_ERROR_CERT_PATH_TOO_LONG                          _CHIP_ERROR(76)

/**
 *  @def CHIP_ERROR_CERT_USAGE_NOT_ALLOWED
 *
 *  @brief
 *    A requested certificate usage is not allowed.
 *
 */
#define CHIP_ERROR_CERT_USAGE_NOT_ALLOWED                      _CHIP_ERROR(77)

/**
 *  @def CHIP_ERROR_CERT_EXPIRED
 *
 *  @brief
 *    A certificate expired.
 *
 */
#define CHIP_ERROR_CERT_EXPIRED                                _CHIP_ERROR(78)

/**
 *  @def CHIP_ERROR_CERT_NOT_VALID_YET
 *
 *  @brief
 *    A certificate is not valid yet.
 *
 */
#define CHIP_ERROR_CERT_NOT_VALID_YET                          _CHIP_ERROR(79)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_CERT_FORMAT
 *
 *  @brief
 *    A certificate format is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_CERT_FORMAT                     _CHIP_ERROR(80)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_ELLIPTIC_CURVE
 *
 *  @brief
 *    An elliptic curve is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_ELLIPTIC_CURVE                  _CHIP_ERROR(81)

/**
 *  @def CHIP_CERT_NOT_USED
 *
 *  @brief
 *    A certificate was not used during the chain validation.
 *
 */
#define CHIP_CERT_NOT_USED                                     _CHIP_ERROR(82)

/**
 *  @def CHIP_ERROR_CERT_NOT_FOUND
 *
 *  @brief
 *    A certificate is not found.
 *
 */
#define CHIP_ERROR_CERT_NOT_FOUND                              _CHIP_ERROR(83)

/**
 *  @def CHIP_ERROR_INVALID_CASE_PARAMETER
 *
 *  @brief
 *    A CASE parameter is invalid.
 *
 */
#define CHIP_ERROR_INVALID_CASE_PARAMETER                      _CHIP_ERROR(84)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_CASE_CONFIGURATION
 *
 *  @brief
 *    A CASE configuration is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_CASE_CONFIGURATION              _CHIP_ERROR(85)

/**
 *  @def CHIP_ERROR_CERT_LOAD_FAILED
 *
 *  @brief
 *    A certificate load failed.
 *
 */
#define CHIP_ERROR_CERT_LOAD_FAILED                             _CHIP_ERROR(86)

/**
 *  @def CHIP_ERROR_CERT_NOT_TRUSTED
 *
 *  @brief
 *    A certificate is not trusted.
 *
 */
#define CHIP_ERROR_CERT_NOT_TRUSTED                            _CHIP_ERROR(87)

/**
 *  @def CHIP_ERROR_INVALID_ACCESS_TOKEN
 *
 *  @brief
 *    An access token is invalid.
 *
 */
#define CHIP_ERROR_INVALID_ACCESS_TOKEN                        _CHIP_ERROR(88)

/**
 *  @def CHIP_ERROR_WRONG_CERT_SUBJECT
 *
 *  @brief
 *    A certificate subject is wrong.
 *
 */
#define CHIP_ERROR_WRONG_CERT_SUBJECT                          _CHIP_ERROR(89)

// deprecated alias
#define CHIP_ERROR_WRONG_CERTIFICATE_SUBJECT CHIP_ERROR_WRONG_CERT_SUBJECT

/**
 *  @def CHIP_ERROR_INVALID_PROVISIONING_BUNDLE
 *
 *  @brief
 *    A provisioning bundle is invalid.
 *
 */
#define CHIP_ERROR_INVALID_PROVISIONING_BUNDLE                 _CHIP_ERROR(90)

/**
 *  @def CHIP_ERROR_PROVISIONING_BUNDLE_DECRYPTION_ERROR
 *
 *  @brief
 *    A provision bundle encountered a decryption error.
 *
 */
#define CHIP_ERROR_PROVISIONING_BUNDLE_DECRYPTION_ERROR        _CHIP_ERROR(91)

/**
 *  @def CHIP_ERROR_WRONG_NODE_ID
 *
 *  @brief
 *    A node id is wrong.
 *
 */
#define CHIP_ERROR_WRONG_NODE_ID                               _CHIP_ERROR(92)

/**
 *  @def CHIP_ERROR_CONN_ACCEPTED_ON_WRONG_PORT
 *
 *  @brief
 *    A connection is accepted on a wrong port.
 *
 */
#define CHIP_ERROR_CONN_ACCEPTED_ON_WRONG_PORT                 _CHIP_ERROR(93)

/**
 *  @def CHIP_ERROR_CALLBACK_REPLACED
 *
 *  @brief
 *    An application callback has been replaced.
 *
 */
#define CHIP_ERROR_CALLBACK_REPLACED                           _CHIP_ERROR(94)

/**
 *  @def CHIP_ERROR_NO_CASE_AUTH_DELEGATE
 *
 *  @brief
 *    No CASE authentication delegate is set.
 *
 */
#define CHIP_ERROR_NO_CASE_AUTH_DELEGATE                       _CHIP_ERROR(95)

/**
 *  @def CHIP_ERROR_DEVICE_LOCATE_TIMEOUT
 *
 *  @brief
 *    The attempt to locate device timed out.
 *
 */
#define CHIP_ERROR_DEVICE_LOCATE_TIMEOUT                       _CHIP_ERROR(96)

/**
 *  @def CHIP_ERROR_DEVICE_CONNECT_TIMEOUT
 *
 *  @brief
 *    The attempt to connect device timed out.
 *
 */
#define CHIP_ERROR_DEVICE_CONNECT_TIMEOUT                      _CHIP_ERROR(97)

/**
 *  @def CHIP_ERROR_DEVICE_AUTH_TIMEOUT
 *
 *  @brief
 *    The attempt to authenticate device timed out.
 *
 */
#define CHIP_ERROR_DEVICE_AUTH_TIMEOUT                         _CHIP_ERROR(98)

/**
 *  @def CHIP_ERROR_MESSAGE_NOT_ACKNOWLEDGED
 *
 *  @brief
 *    A message is not acknowledged after max retries.
 *
 */
#define CHIP_ERROR_MESSAGE_NOT_ACKNOWLEDGED                    _CHIP_ERROR(99)

/**
 *  @def CHIP_ERROR_RETRANS_TABLE_FULL
 *
 *  @brief
 *    A retransmission table is already full.
 *
 */
#define CHIP_ERROR_RETRANS_TABLE_FULL                          _CHIP_ERROR(100)

/**
 *  @def CHIP_ERROR_INVALID_ACK_ID
 *
 *  @brief
 *    An acknowledgment id is invalid.
 *
 */
#define CHIP_ERROR_INVALID_ACK_ID                              _CHIP_ERROR(101)

/**
 *  @def CHIP_ERROR_SEND_THROTTLED
 *
 *  @brief
 *    A send is throttled.
 *
 */
#define CHIP_ERROR_SEND_THROTTLED                              _CHIP_ERROR(102)

/**
 *  @def CHIP_ERROR_WRONG_MSG_VERSION_FOR_EXCHANGE
 *
 *  @brief
 *    A message version is not supported by the current exchange context.
 *
 */
#define CHIP_ERROR_WRONG_MSG_VERSION_FOR_EXCHANGE              _CHIP_ERROR(103)

/**
 *  @def CHIP_ERROR_TRANSACTION_CANCELED
 *
 *  @brief
 *    A transaction is cancelled.
 *
 */
#define CHIP_ERROR_TRANSACTION_CANCELED                        _CHIP_ERROR(104)

/**
 *  @def CHIP_ERROR_LISTENER_ALREADY_STARTED
 *
 *  @brief
 *    A listener has already started.
 *
 */
#define CHIP_ERROR_LISTENER_ALREADY_STARTED                    _CHIP_ERROR(105)

/**
 *  @def CHIP_ERROR_LISTENER_ALREADY_STOPPED
 *
 *  @brief
 *    A listener has already stopped.
 *
 */
#define CHIP_ERROR_LISTENER_ALREADY_STOPPED                    _CHIP_ERROR(106)

/**
 *  @def CHIP_ERROR_UNKNOWN_TOPIC
 *
 *  @brief
 *    A topic ID was unknown to the recipient.
 *
 */
#define CHIP_ERROR_UNKNOWN_TOPIC                               _CHIP_ERROR(107)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
 *
 *  @brief
 *    A CHIP feature is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE                   _CHIP_ERROR(108)

/**
 *  @def CHIP_ERROR_PASE_RECONFIGURE_REQUIRED
 *
 *  @brief
 *    PASE is required to reconfigure.
 *
 */
#define CHIP_ERROR_PASE_RECONFIGURE_REQUIRED                   _CHIP_ERROR(109)

/**
 *  @def CHIP_ERROR_INVALID_PASE_CONFIGURATION
 *
 *  @brief
 *    A PASE configuration is invalid.
 *
 */
#define CHIP_ERROR_INVALID_PASE_CONFIGURATION                  _CHIP_ERROR(110)

/**
 *  @def CHIP_ERROR_NO_COMMON_PASE_CONFIGURATIONS
 *
 *  @brief
 *    No PASE configuration is in common.
 *
 */
#define CHIP_ERROR_NO_COMMON_PASE_CONFIGURATIONS               _CHIP_ERROR(111)

/**
 *  @def CHIP_ERROR_UNSOLICITED_MSG_NO_ORIGINATOR
 *
 *  @brief
 *    An unsolicited message with the originator bit clear.
 *
 */
#define CHIP_ERROR_UNSOLICITED_MSG_NO_ORIGINATOR               _CHIP_ERROR(112)

/**
 *  @def CHIP_ERROR_INVALID_FABRIC_ID
 *
 *  @brief
 *    A fabric id is invalid.
 *
 */
#define CHIP_ERROR_INVALID_FABRIC_ID                           _CHIP_ERROR(113)

/**
 *  @def CHIP_ERROR_DRBG_ENTROPY_SOURCE_FAILED
 *
 *  @brief
 *    DRBG entropy source failed to generate entropy data.
 *
 */
#define CHIP_ERROR_DRBG_ENTROPY_SOURCE_FAILED                  _CHIP_ERROR(117)

/**
 *  @def CHIP_ERROR_TLV_TAG_NOT_FOUND
 *
 *  @brief
 *    A specified TLV tag was not found.
 *
 */
#define CHIP_ERROR_TLV_TAG_NOT_FOUND                           _CHIP_ERROR(118)

/**
 *  @def CHIP_ERROR_INVALID_TOKENPAIRINGBUNDLE
 *
 *  @brief
 *    A token pairing bundle is invalid.
 *
 */
#define CHIP_ERROR_INVALID_TOKENPAIRINGBUNDLE                  _CHIP_ERROR(119)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_TOKENPAIRINGBUNDLE_VERSION
 *
 *  @brief
 *    A token pairing bundle is invalid.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_TOKENPAIRINGBUNDLE_VERSION      _CHIP_ERROR(120)

/**
 *  @def CHIP_ERROR_NO_TAKE_AUTH_DELEGATE
 *
 *  @brief
 *    No TAKE authentication delegate is set.
 *
 */
#define CHIP_ERROR_NO_TAKE_AUTH_DELEGATE                       _CHIP_ERROR(121)

/**
 *  @def CHIP_ERROR_TAKE_RECONFIGURE_REQUIRED
 *
 *  @brief
 *    TAKE requires a reconfigure.
 *
 */
#define CHIP_ERROR_TAKE_RECONFIGURE_REQUIRED                   _CHIP_ERROR(122)

/**
 *  @def CHIP_ERROR_TAKE_REAUTH_POSSIBLE
 *
 *  @brief
 *    TAKE can do a reauthentication.
 *
 */
#define CHIP_ERROR_TAKE_REAUTH_POSSIBLE                        _CHIP_ERROR(123)

/**
 *  @def CHIP_ERROR_INVALID_TAKE_PARAMETER
 *
 *  @brief
 *    Received an invalid TAKE paramter.
 *
 */
#define CHIP_ERROR_INVALID_TAKE_PARAMETER                      _CHIP_ERROR(124)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_TAKE_CONFIGURATION
 *
 *  @brief
 *    This configuration is not supported by TAKE.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_TAKE_CONFIGURATION              _CHIP_ERROR(125)

/**
 *  @def CHIP_ERROR_TAKE_TOKEN_IDENTIFICATION_FAILED
 *
 *  @brief
 *    The TAKE Token Identification failed.
 *
 */
#define CHIP_ERROR_TAKE_TOKEN_IDENTIFICATION_FAILED            _CHIP_ERROR(126)

/**
 *  @def CHIP_ERROR_KEY_NOT_FOUND_FROM_PEER
 *
 *  @brief
 *    The encryption key is not found error received from a peer node.
 *
 */
#define CHIP_ERROR_KEY_NOT_FOUND_FROM_PEER                     _CHIP_ERROR(127)

/**
 *  @def CHIP_ERROR_WRONG_ENCRYPTION_TYPE_FROM_PEER
 *
 *  @brief
 *    The wrong encryption type error received from a peer node.
 *
 */
#define CHIP_ERROR_WRONG_ENCRYPTION_TYPE_FROM_PEER             _CHIP_ERROR(128)

/**
 *  @def CHIP_ERROR_UNKNOWN_KEY_TYPE_FROM_PEER
 *
 *  @brief
 *    The unknown key type error received from a peer node.
 *
 */
#define CHIP_ERROR_UNKNOWN_KEY_TYPE_FROM_PEER                  _CHIP_ERROR(129)

/**
 *  @def CHIP_ERROR_INVALID_USE_OF_SESSION_KEY_FROM_PEER
 *
 *  @brief
 *    The invalid use of session key error received from a peer node.
 *
 */
#define CHIP_ERROR_INVALID_USE_OF_SESSION_KEY_FROM_PEER        _CHIP_ERROR(130)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE_FROM_PEER
 *
 *  @brief
 *    An unsupported encryption type error received from a peer node.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE_FROM_PEER       _CHIP_ERROR(131)

/**
 *  @def CHIP_ERROR_INTERNAL_KEY_ERROR_FROM_PEER
 *
 *  @brief
 *    The internal key error received from a peer node.
 *
 */
#define CHIP_ERROR_INTERNAL_KEY_ERROR_FROM_PEER                _CHIP_ERROR(132)

/**
 *  @def CHIP_ERROR_INVALID_KEY_ID
 *
 *  @brief
 *    A key id is invalid.
 *
 */
#define CHIP_ERROR_INVALID_KEY_ID                              _CHIP_ERROR(133)

/**
 *  @def CHIP_ERROR_INVALID_TIME
 *
 *  @brief
 *    Time has invalid value.
 *
 */
#define CHIP_ERROR_INVALID_TIME                                _CHIP_ERROR(134)

/**
 *  @def CHIP_ERROR_LOCKING_FAILURE
 *
 *  @brief
 *    Failure to acquire or release an OS provided mutex.
 *
 */
#define CHIP_ERROR_LOCKING_FAILURE                             _CHIP_ERROR(135)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_PASSCODE_CONFIG
 *
 *  @brief
 *    A passcode encryption configuration is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_PASSCODE_CONFIG                 _CHIP_ERROR(136)

/**
 *  @def CHIP_ERROR_PASSCODE_AUTHENTICATION_FAILED
 *
 *  @brief
 *    The CHIP passcode authentication failed.
 *
 */
#define CHIP_ERROR_PASSCODE_AUTHENTICATION_FAILED              _CHIP_ERROR(137)

/**
 *  @def CHIP_ERROR_PASSCODE_FINGERPRINT_FAILED
 *
 *  @brief
 *    The CHIP passcode fingerprint failed.
 *
 */
#define CHIP_ERROR_PASSCODE_FINGERPRINT_FAILED                 _CHIP_ERROR(138)

/**
 *  @def CHIP_ERROR_SERIALIZATION_ELEMENT_NULL
 *
 *  @brief
 *      The element of the struct is null.
 *
 */
#define CHIP_ERROR_SERIALIZATION_ELEMENT_NULL                  _CHIP_ERROR(139)

/**
 *  @def CHIP_ERROR_WRONG_CERT_SIGNATURE_ALGORITHM
 *
 *  @brief
 *    The certificate was not signed using the required signature algorithm.
 *
 */
#define CHIP_ERROR_WRONG_CERT_SIGNATURE_ALGORITHM              _CHIP_ERROR(140)

/**
 *  @def CHIP_ERROR_WRONG_CHIP_SIGNATURE_ALGORITHM
 *
 *  @brief
 *    The CHIP signature was not signed using the required signature algorithm.
 *
 */
#define CHIP_ERROR_WRONG_CHIP_SIGNATURE_ALGORITHM             _CHIP_ERROR(141)

/**
 *  @def CHIP_ERROR_SCHEMA_MISMATCH
 *
 *  @brief
 *    A mismatch in schema was encountered.
 *
 */
#define CHIP_ERROR_SCHEMA_MISMATCH                            _CHIP_ERROR(142)

/**
 *  @def CHIP_ERROR_INVALID_INTEGER_VALUE
 *
 *  @brief
 *    An integer does not have the kind of value we expect.
 *
 */
#define CHIP_ERROR_INVALID_INTEGER_VALUE                       _CHIP_ERROR(143)

/**
 *  @def CHIP_ERROR_CASE_RECONFIG_REQUIRED
 *
 *  @brief
 *    CASE is required to reconfigure.
 *
 */
#define CHIP_ERROR_CASE_RECONFIG_REQUIRED                      _CHIP_ERROR(144)

/**
 *  @def CHIP_ERROR_TOO_MANY_CASE_RECONFIGURATIONS
 *
 *  @brief
 *    Too many CASE reconfigurations were received.
 *
 */
#define CHIP_ERROR_TOO_MANY_CASE_RECONFIGURATIONS              _CHIP_ERROR(145)

/**
 *  @def CHIP_ERROR_BAD_REQUEST
 *
 *  @brief
 *    The request cannot be processed or fulfilled
 *
 */
#define CHIP_ERROR_BAD_REQUEST                                 _CHIP_ERROR(146)

/**
 *  @def CHIP_ERROR_INVALID_MESSAGE_FLAG
 *
 *  @brief
 *    One or more message flags have invalid value.
 *
 */
#define CHIP_ERROR_INVALID_MESSAGE_FLAG                        _CHIP_ERROR(147)

/**
 *  @def CHIP_ERROR_KEY_EXPORT_RECONFIGURE_REQUIRED
 *
 *  @brief
 *    Key export protocol required to reconfigure.
 *
 */
#define CHIP_ERROR_KEY_EXPORT_RECONFIGURE_REQUIRED             _CHIP_ERROR(148)

/**
 *  @def CHIP_ERROR_INVALID_KEY_EXPORT_CONFIGURATION
 *
 *  @brief
 *    A key export protocol configuration is invalid.
 *
 */
#define CHIP_ERROR_INVALID_KEY_EXPORT_CONFIGURATION            _CHIP_ERROR(149)

/**
 *  @def CHIP_ERROR_NO_COMMON_KEY_EXPORT_CONFIGURATIONS
 *
 *  @brief
 *    No key export protocol configuration is in common.
 *
 */
#define CHIP_ERROR_NO_COMMON_KEY_EXPORT_CONFIGURATIONS         _CHIP_ERROR(150)

/**
 *  @def CHIP_ERROR_NO_KEY_EXPORT_DELEGATE
 *
 *  @brief
 *    No key export delegate is set.
 *
 */
#define CHIP_ERROR_NO_KEY_EXPORT_DELEGATE                      _CHIP_ERROR(151)

/**
 *  @def CHIP_ERROR_UNAUTHORIZED_KEY_EXPORT_REQUEST
 *
 *  @brief
 *    Unauthorized key export request.
 *
 */
#define CHIP_ERROR_UNAUTHORIZED_KEY_EXPORT_REQUEST             _CHIP_ERROR(152)

/**
 *  @def CHIP_ERROR_UNAUTHORIZED_KEY_EXPORT_RESPONSE
 *
 *  @brief
 *    Unauthorized key export response.
 *
 */
#define CHIP_ERROR_UNAUTHORIZED_KEY_EXPORT_RESPONSE            _CHIP_ERROR(153)

/**
 *  @def CHIP_ERROR_EXPORTED_KEY_AUTHENTICATION_FAILED
 *
 *  @brief
 *    The CHIP exported encrypted key authentication failed.
 *
 */
#define CHIP_ERROR_EXPORTED_KEY_AUTHENTICATION_FAILED          _CHIP_ERROR(154)

/**
 *  @def CHIP_ERROR_TOO_MANY_SHARED_SESSION_END_NODES
 *
 *  @brief
 *    The number of shared secure sessions end nodes exceeds
 *    the maximum limit.
 *
 */
#define CHIP_ERROR_TOO_MANY_SHARED_SESSION_END_NODES           _CHIP_ERROR(155)

/**
 * @def CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_DATA_ELEMENT
 *
 * @brief
 *   The Attribute DataElement is malformed: it either does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_DATA_ELEMENT         _CHIP_ERROR(156)

/**
 * @def CHIP_ERROR_WRONG_CERT_TYPE
 *
 * @brief
 *   The presented certificate was of the wrong type.
 */
#define CHIP_ERROR_WRONG_CERT_TYPE                             _CHIP_ERROR(157)

/**
 * @def CHIP_ERROR_DEFAULT_EVENT_HANDLER_NOT_CALLED
 *
 * @brief
 *   The application's event handler failed to call the default event handler function
 *   when presented with an unknown event.
 */
#define CHIP_ERROR_DEFAULT_EVENT_HANDLER_NOT_CALLED            _CHIP_ERROR(158)

/**
 *  @def CHIP_ERROR_PERSISTED_STORAGE_FAILED
 *
 *  @brief
 *    Persisted storage memory read/write failure.
 *
 */
#define CHIP_ERROR_PERSISTED_STORAGE_FAILED                    _CHIP_ERROR(159)

/**
 *  @def CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND
 *
 *  @brief
 *    The specific value is not found in the persisted storage.
 *
 */
#define CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND           _CHIP_ERROR(160)

/**
 *  @def CHIP_ERROR_PROFILE_STRING_CONTEXT_ALREADY_REGISTERED
 *
 *  @brief
 *    The specified profile string support context is already registered.
 *
 */
#define CHIP_ERROR_PROFILE_STRING_CONTEXT_ALREADY_REGISTERED   _CHIP_ERROR(161)

/**
 *  @def CHIP_ERROR_PROFILE_STRING_CONTEXT_NOT_REGISTERED
 *
 *  @brief
 *    The specified profile string support context is not registered.
 *
 */
#define CHIP_ERROR_PROFILE_STRING_CONTEXT_NOT_REGISTERED       _CHIP_ERROR(162)

/**
 *  @def CHIP_ERROR_INCOMPATIBLE_SCHEMA_VERSION
 *
 *  @brief
 *    Encountered a mismatch in compatibility w.r.t to IDL schema version
 */
#define CHIP_ERROR_INCOMPATIBLE_SCHEMA_VERSION                 _CHIP_ERROR(163)

/**
 *  @def CHIP_ERROR_MISMATCH_UPDATE_REQUIRED_VERSION
 *
 *  @brief
 *    Encountered a mismatch between update required version and current version
 */
#define CHIP_ERROR_MISMATCH_UPDATE_REQUIRED_VERSION            _CHIP_ERROR(164)


/**
 *  @def CHIP_ERROR_ACCESS_DENIED
 *
 *  @brief
 *    The CHIP message is not granted access for further processing.
 */
#define CHIP_ERROR_ACCESS_DENIED                               _CHIP_ERROR(165)

/**
 *  @def CHIP_ERROR_UNKNOWN_RESOURCE_ID
 *
 *  @brief
 *    Unknown resource ID
 *
 */
#define CHIP_ERROR_UNKNOWN_RESOURCE_ID                         _CHIP_ERROR(166)

/**
 * @def CHIP_ERROR_VERSION_MISMATCH
 *
 * @brief
 *   The conditional update of a trait instance path has failed
 *   because the local changes are based on an obsolete version of the
 *   data.
 */
#define CHIP_ERROR_VERSION_MISMATCH                            _CHIP_ERROR(167)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_THREAD_NETWORK_CREATE
 *
 *  @brief
 *    Device doesn't support standalone Thread network creation.
 *    On some legacy devices new Thread network can only be created
 *    together with CHIP Fabric using CrateFabric() message.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_THREAD_NETWORK_CREATE           _CHIP_ERROR(168)

/**
 *  @def CHIP_ERROR_INCONSISTENT_CONDITIONALITY
 *
 *  @brief
 *    A TraitPath was declared updated with a conditionality that
 *    does not match that of other TraitPaths already updated in the
 *    same Trait Instance.
 *
 */
#define CHIP_ERROR_INCONSISTENT_CONDITIONALITY                 _CHIP_ERROR(169)

/**
 *  @def CHIP_ERROR_LOCAL_DATA_INCONSISTENT
 *
 *  @brief
 *    The local data does not match any known version of the
 *    Trait Instance and cannot support the operation requested.
 *
 */
#define CHIP_ERROR_LOCAL_DATA_INCONSISTENT                     _CHIP_ERROR(170)

/**
 * @def CHIP_EVENT_ID_FOUND
 *
 * @brief
 *   Event ID matching the criteria was found
 */
#define CHIP_EVENT_ID_FOUND                                    _CHIP_ERROR(171)

/**
 * @def CHIP_ERROR_INTERNAL
 *
 * @brief
 *   Internal error
 */
#define CHIP_ERROR_INTERNAL                                    _CHIP_ERROR(172)

/**
 * @def CHIP_ERROR_OPEN_FAILED
 *
 * @brief
 *   Open file failed
 */
#define CHIP_ERROR_OPEN_FAILED                                 _CHIP_ERROR(173)

/**
 * @def CHIP_ERROR_READ_FAILED
 *
 * @brief
 *   Read from file failed
 */
#define CHIP_ERROR_READ_FAILED                                 _CHIP_ERROR(174)

/**
 * @def CHIP_ERROR_WRITE_FAILED
 *
 * @brief
 *   Write to file failed
 */
#define CHIP_ERROR_WRITE_FAILED                                _CHIP_ERROR(175)

/**
 * @def CHIP_ERROR_DECODE_FAILED
 *
 * @brief
 *   Decoding failed
 */
#define CHIP_ERROR_DECODE_FAILED                               _CHIP_ERROR(176)


/**
 *  @def CHIP_ERROR_SESSION_KEY_SUSPENDED
 *
 *  @brief
 *    Use of the identified session key is suspended.
 *
 */
#define CHIP_ERROR_SESSION_KEY_SUSPENDED                       _CHIP_ERROR(177)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_WIRELESS_REGULATORY_DOMAIN
 *
 *  @brief
 *    The specified wireless regulatory domain is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_WIRELESS_REGULATORY_DOMAIN      _CHIP_ERROR(178)

/**
 *  @def CHIP_ERROR_UNSUPPORTED_WIRELESS_OPERATING_LOCATION
 *
 *  @brief
 *    The specified wireless operating location is unsupported.
 *
 */
#define CHIP_ERROR_UNSUPPORTED_WIRELESS_OPERATING_LOCATION     _CHIP_ERROR(179)

/**
 *  @def CHIP_ERROR_MDNS_COLLISSION
 *
 *  @brief
 *    The registered service name has collision on the LAN.
 *
 */
#define CHIP_ERROR_MDNS_COLLISSION _CHIP_ERROR(180)

/**
 * @def CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH
 *
 * @brief
 *   The Attribute path is malformed: it either does not contain
 *   the required path
 */
#define CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH                      _CHIP_ERROR(181)

/**
 * @def CHIP_ERROR_IM_MALFORMED_EVENT_PATH
 *
 * @brief
 *   The Attribute Path is malformed: it either does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_EVENT_PATH                         _CHIP_ERROR(182)

/**
 * @def CHIP_ERROR_IM_MALFORMED_COMMAND_PATH
 *
 * @brief
 *   The Attribute DataElement is malformed: it either does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_COMMAND_PATH                     _CHIP_ERROR(183)

/**
 * @def CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_STATUS_ELEMENT
 *
 * @brief
 *   The Attribute DataElement is malformed: it either does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_STATUS_ELEMENT                      _CHIP_ERROR(184)

/**
 * @def CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_ELEMENT
 *
 * @brief
 *   The Attribute DataElement is malformed: it either does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_ELEMENT                      _CHIP_ERROR(185)

/**
 * @def CHIP_ERROR_IM_MALFORMED_EVENT_DATA_ELEMENT
 *
 * @brief
 *   The Event DataElement is malformed: it either does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_EVENT_DATA_ELEMENT                       _CHIP_ERROR(186)

/**
 * @def CHIP_ERROR_IM_MALFORMED_STATUS_CODE
 *
 * @brief
 *   The Attribute DataElement is malformed: it either does not contain
 *   the required elements
 */
#define CHIP_ERROR_IM_MALFORMED_STATUS_CODE                      _CHIP_ERROR(187)

/**
 * @def CHIP_ERROR_PEER_NODE_NOT_FOUND
 *
 * @brief
 *   Unable to find the peer node
 */
#define CHIP_ERROR_PEER_NODE_NOT_FOUND                           _CHIP_ERROR(188)

/**
 *  @}
 */

// !!!!! IMPORTANT !!!!!  If you add new CHIP errors, please update the translation
// of error codes to strings in CHIPError.cpp, and add them to unittest
// in test-apps/TestErrorStr.cpp

// clang-format on

namespace chip {

extern void RegisterCHIPLayerErrorFormatter();
extern bool FormatCHIPError(char * buf, uint16_t bufSize, int32_t err);

} // namespace chip
