/*
 *
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      This file defines error constants for the Nest Weave core
 *      subsystem.
 *
 *      Error types, ranges, and mappings overrides may be made by
 *      defining the appropriate WEAVE_CONFIG_* or _WEAVE_CONFIG_*
 *      macros.
 *
 *  NOTE WELL: On some platforms, this header is included by C-language programs.
 *
 */

#ifndef WEAVE_ERROR_H
#define WEAVE_ERROR_H

#include "WeaveConfig.h"
// clang-format off

/**
 *  @def WEAVE_NO_ERROR
 *
 *  @brief
 *    This defines the Weave error code for success or no error.
 *    This value may be configured via #WEAVE_CONFIG_NO_ERROR."
 *
 */
#define WEAVE_NO_ERROR                                          WEAVE_CONFIG_NO_ERROR

/**
 *  @def WEAVE_ERROR_MIN
 *
 *  @brief
 *    This defines the bottom or minimum Weave error number range.
 *    This value may be configured via #WEAVE_CONFIG_ERROR_MIN.
 *
 */
#define WEAVE_ERROR_MIN                                         WEAVE_CONFIG_ERROR_MIN

/**
 *  @def WEAVE_ERROR_MAX
 *
 *  @brief
 *    This defines the top or maximum Weave error number range.
 *    This value may be configured via #WEAVE_CONFIG_ERROR_MAX.
 *
 */
#define WEAVE_ERROR_MAX                                         WEAVE_CONFIG_ERROR_MAX

/**
 *  @def _WEAVE_ERROR(e)
 *
 *  @brief
 *    This defines a mapping function for Weave errors that allows
 *    mapping such errors into a platform- or system-specific range.
 *    This function may be configured via #_WEAVE_CONFIG_ERROR(e).
 *
 *  @param[in] e       The Weave error to map.
 *
 *  @return    The mapped Weave error.
 *
 *
 */
#define _WEAVE_ERROR(e)                                         _WEAVE_CONFIG_ERROR(e)

/**
 *  The basic type for all Weave errors.
 *
 *  @brief
 *    This is defined to a platform- or system-specific type.
 *
 */
typedef WEAVE_CONFIG_ERROR_TYPE WEAVE_ERROR;

/**
 *  @name Error Definitions
 *
 *  @{
 */

/**
 *  @def WEAVE_ERROR_TOO_MANY_CONNECTIONS
 *
 *  @brief
 *    The attempt to allocate a connection object failed because too many
 *    connections exist.
 *
 */
#define WEAVE_ERROR_TOO_MANY_CONNECTIONS                        _WEAVE_ERROR(0)

/**
 *  @def WEAVE_ERROR_SENDING_BLOCKED
 *
 *  @brief
 *    A message exceeds the sent limit.
 *
 */
#define WEAVE_ERROR_SENDING_BLOCKED                             _WEAVE_ERROR(1)

/**
 *  @def WEAVE_ERROR_CONNECTION_ABORTED
 *
 *  @brief
 *    A connection has been aborted.
 *
 */
#define WEAVE_ERROR_CONNECTION_ABORTED                          _WEAVE_ERROR(2)

/**
 *  @def WEAVE_ERROR_INCORRECT_STATE
 *
 *  @brief
 *    An unexpected state was encountered.
 *
 */
#define WEAVE_ERROR_INCORRECT_STATE                             _WEAVE_ERROR(3)

/**
 *  @def WEAVE_ERROR_MESSAGE_TOO_LONG
 *
 *  @brief
 *    A message is too long.
 *
 */
#define WEAVE_ERROR_MESSAGE_TOO_LONG                            _WEAVE_ERROR(4)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_EXCHANGE_VERSION
 *
 *  @brief
 *    An exchange version is not supported.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_EXCHANGE_VERSION                _WEAVE_ERROR(5)

/**
 *  @def WEAVE_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS
 *
 *  @brief
 *    The attempt to register an unsolicited message handler failed because the
 *    unsolicited message handler pool is full.
 *
 */
#define WEAVE_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS       _WEAVE_ERROR(6)

/**
 *  @def WEAVE_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER
 *
 *  @brief
 *    The attempt to unregister an unsolicited message handler failed because
 *    the target handler was not found in the unsolicited message handler pool.
 *
 */
#define WEAVE_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER              _WEAVE_ERROR(7)

/**
 *  @def WEAVE_ERROR_NO_CONNECTION_HANDLER
 *
 *  @brief
 *    No callback has been registered for handling a connection.
 *
 */
#define WEAVE_ERROR_NO_CONNECTION_HANDLER                       _WEAVE_ERROR(8)

/**
 *  @def WEAVE_ERROR_TOO_MANY_PEER_NODES
 *
 *  @brief
 *    The number of peer nodes exceeds the maximum limit of a local node.
 *
 */
#define WEAVE_ERROR_TOO_MANY_PEER_NODES                         _WEAVE_ERROR(9)

/**
 *  @def WEAVE_ERROR_NO_MEMORY
 *
 *  @brief
 *    The attempt to allocate a buffer or object failed due to a lack of memory.
 *
 */
#define WEAVE_ERROR_NO_MEMORY                                   _WEAVE_ERROR(11)

/**
 *  @def WEAVE_ERROR_NO_MESSAGE_HANDLER
 *
 *  @brief
 *    No callback has been registered for handling a message.
 *
 */
#define WEAVE_ERROR_NO_MESSAGE_HANDLER                          _WEAVE_ERROR(12)

/**
 *  @def WEAVE_ERROR_MESSAGE_INCOMPLETE
 *
 *  @brief
 *    A message is incomplete.
 *
 */
#define WEAVE_ERROR_MESSAGE_INCOMPLETE                          _WEAVE_ERROR(13)

/**
 *  @def WEAVE_ERROR_DATA_NOT_ALIGNED
 *
 *  @brief
 *    The data is not aligned.
 *
 */
#define WEAVE_ERROR_DATA_NOT_ALIGNED                            _WEAVE_ERROR(14)

/**
 *  @def WEAVE_ERROR_UNKNOWN_KEY_TYPE
 *
 *  @brief
 *    The encryption key type is unknown.
 *
 */
#define WEAVE_ERROR_UNKNOWN_KEY_TYPE                            _WEAVE_ERROR(15)

/**
 *  @def WEAVE_ERROR_KEY_NOT_FOUND
 *
 *  @brief
 *    The encryption key is not found.
 *
 */
#define WEAVE_ERROR_KEY_NOT_FOUND                               _WEAVE_ERROR(16)

/**
 *  @def WEAVE_ERROR_WRONG_ENCRYPTION_TYPE
 *
 *  @brief
 *    The encryption type is incorrect for the specified key.
 *
 */
#define WEAVE_ERROR_WRONG_ENCRYPTION_TYPE                       _WEAVE_ERROR(17)

/**
 *  @def WEAVE_ERROR_TOO_MANY_KEYS
 *
 *  @brief
 *    The attempt to allocate a key failed because the number of active keys
 *    exceeds the maximum limit.
 *
 */
#define WEAVE_ERROR_TOO_MANY_KEYS                               _WEAVE_ERROR(18)

/**
 *  @def WEAVE_ERROR_INTEGRITY_CHECK_FAILED
 *
 *  @brief
 *    The integrity check in the message does not match the expected integrity
 *    check.
 *
 */
#define WEAVE_ERROR_INTEGRITY_CHECK_FAILED                      _WEAVE_ERROR(19)

/**
 *  @def WEAVE_ERROR_INVALID_SIGNATURE
 *
 *  @brief
 *    Invalid signature.
 *
 */
#define WEAVE_ERROR_INVALID_SIGNATURE                           _WEAVE_ERROR(20)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_MESSAGE_VERSION
 *
 *  @brief
 *    A message version is unsupported.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_MESSAGE_VERSION                 _WEAVE_ERROR(21)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_ENCRYPTION_TYPE
 *
 *  @brief
 *    An encryption type is unsupported.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_ENCRYPTION_TYPE                 _WEAVE_ERROR(22)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_SIGNATURE_TYPE
 *
 *  @brief
 *    A signature type is unsupported.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_SIGNATURE_TYPE                  _WEAVE_ERROR(23)

/**
 *  @def WEAVE_ERROR_INVALID_MESSAGE_LENGTH
 *
 *  @brief
 *    A message length is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_MESSAGE_LENGTH                      _WEAVE_ERROR(24)

/**
 *  @def WEAVE_ERROR_BUFFER_TOO_SMALL
 *
 *  @brief
 *    A buffer is too small.
 *
 */
#define WEAVE_ERROR_BUFFER_TOO_SMALL                            _WEAVE_ERROR(25)

/**
 *  @def WEAVE_ERROR_DUPLICATE_KEY_ID
 *
 *  @brief
 *    A key id is duplicate.
 *
 */
#define WEAVE_ERROR_DUPLICATE_KEY_ID                            _WEAVE_ERROR(26)

/**
 *  @def WEAVE_ERROR_WRONG_KEY_TYPE
 *
 *  @brief
 *    A key type does not match the expected key type.
 *
 */
#define WEAVE_ERROR_WRONG_KEY_TYPE                              _WEAVE_ERROR(27)

/**
 *  @def WEAVE_ERROR_WELL_UNINITIALIZED
 *
 *  @brief
 *    A requested object is uninitialized.
 *
 */
#define WEAVE_ERROR_WELL_UNINITIALIZED                          _WEAVE_ERROR(28)

/**
 *  @def WEAVE_ERROR_WELL_EMPTY
 *
 *  @brief
 *    A requested object is empty.
 *
 */
#define WEAVE_ERROR_WELL_EMPTY                                  _WEAVE_ERROR(29)

/**
 *  @def WEAVE_ERROR_INVALID_STRING_LENGTH
 *
 *  @brief
 *    A string length is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_STRING_LENGTH                       _WEAVE_ERROR(30)

/**
 *  @def WEAVE_ERROR_INVALID_LIST_LENGTH
 *
 *  @brief
 *    A list length is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_LIST_LENGTH                         _WEAVE_ERROR(31)

/**
 *  @def WEAVE_ERROR_INVALID_INTEGRITY_TYPE
 *
 *  @brief
 *    An integrity type is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_INTEGRITY_TYPE                      _WEAVE_ERROR(32)

/**
 *  @def WEAVE_END_OF_TLV
 *
 *  @brief
 *    The end of a TLV encoding,
 *    or the end of a TLV container element has been reached.
 *
 */
#define WEAVE_END_OF_TLV                                        _WEAVE_ERROR(33)

/**
 *  @def WEAVE_ERROR_TLV_UNDERRUN
 *
 *  @brief
 *    The TLV encoding ended prematurely.
 *
 */
#define WEAVE_ERROR_TLV_UNDERRUN                                _WEAVE_ERROR(34)

/**
 *  @def WEAVE_ERROR_INVALID_TLV_ELEMENT
 *
 *  @brief
 *    A TLV element is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_TLV_ELEMENT                         _WEAVE_ERROR(35)

/**
 *  @def WEAVE_ERROR_INVALID_TLV_TAG
 *
 *  @brief
 *    A TLV tag is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_TLV_TAG                             _WEAVE_ERROR(36)

/**
 *  @def WEAVE_ERROR_UNKNOWN_IMPLICIT_TLV_TAG
 *
 *  @brief
 *    An implicitly encoded TLV tag was encountered,
 *    but an implicit profile id has not been defined.
 *
 */
#define WEAVE_ERROR_UNKNOWN_IMPLICIT_TLV_TAG                    _WEAVE_ERROR(37)

/**
 *  @def WEAVE_ERROR_WRONG_TLV_TYPE
 *
 *  @brief
 *    A TLV type is wrong.
 *
 */
#define WEAVE_ERROR_WRONG_TLV_TYPE                              _WEAVE_ERROR(38)

/**
 *  @def WEAVE_ERROR_TLV_CONTAINER_OPEN
 *
 *  @brief
 *    A TLV container is unexpectedly open.
 *
 */
#define WEAVE_ERROR_TLV_CONTAINER_OPEN                          _WEAVE_ERROR(39)

/**
 *  @def WEAVE_ERROR_INVALID_TRANSFER_MODE
 *
 *  @brief
 *    A transfer mode is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_TRANSFER_MODE                       _WEAVE_ERROR(40)

/**
 *  @def WEAVE_ERROR_INVALID_PROFILE_ID
 *
 *  @brief
 *    A profile id is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_PROFILE_ID                          _WEAVE_ERROR(41)

/**
 *  @def WEAVE_ERROR_INVALID_MESSAGE_TYPE
 *
 *  @brief
 *    A message type is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_MESSAGE_TYPE                        _WEAVE_ERROR(42)

/**
 *  @def WEAVE_ERROR_UNEXPECTED_TLV_ELEMENT
 *
 *  @brief
 *    An unexpected TLV element was encountered.
 *
 */
#define WEAVE_ERROR_UNEXPECTED_TLV_ELEMENT                      _WEAVE_ERROR(43)

/**
 *  @def WEAVE_ERROR_STATUS_REPORT_RECEIVED
 *
 *  @brief
 *    A status report is received from a peer node.
 *
 */
#define WEAVE_ERROR_STATUS_REPORT_RECEIVED                      _WEAVE_ERROR(44)

/**
 *  @def WEAVE_ERROR_NOT_IMPLEMENTED
 *
 *  @brief
 *    A requested function or feature is not implemented.
 *
 */
#define WEAVE_ERROR_NOT_IMPLEMENTED                             _WEAVE_ERROR(45)

/**
 *  @def WEAVE_ERROR_INVALID_ADDRESS
 *
 *  @brief
 *    An address is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_ADDRESS                             _WEAVE_ERROR(46)

/**
 *  @def WEAVE_ERROR_INVALID_ARGUMENT
 *
 *  @brief
 *    An argument is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_ARGUMENT                            _WEAVE_ERROR(47)

/**
 *  @def WEAVE_ERROR_INVALID_PATH_LIST
 *
 *  @brief
 *    A TLV path list is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_PATH_LIST                           _WEAVE_ERROR(48)

/**
 *  @def WEAVE_ERROR_INVALID_DATA_LIST
 *
 *  @brief
 *    A TLV data list is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_DATA_LIST                           _WEAVE_ERROR(49)

/**
 *  @def WEAVE_ERROR_TIMEOUT
 *
 *  @brief
 *    A request timed out.
 *
 */
#define WEAVE_ERROR_TIMEOUT                                     _WEAVE_ERROR(50)

/**
 *  @def WEAVE_ERROR_INVALID_DEVICE_DESCRIPTOR
 *
 *  @brief
 *    A device descriptor is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_DEVICE_DESCRIPTOR                   _WEAVE_ERROR(51)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_DEVICE_DESCRIPTOR_VERSION
 *
 *  @brief
 *    A device descriptor version is unsupported.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_DEVICE_DESCRIPTOR_VERSION       _WEAVE_ERROR(52)

/**
 *  @def WEAVE_END_OF_INPUT
 *
 *  @brief
 *    An input ended.
 *
 */
#define WEAVE_END_OF_INPUT                                      _WEAVE_ERROR(53)

/**
 *  @def WEAVE_ERROR_RATE_LIMIT_EXCEEDED
 *
 *  @brief
 *    A rate limit is exceeded.
 *
 */
#define WEAVE_ERROR_RATE_LIMIT_EXCEEDED                         _WEAVE_ERROR(54)

/**
 *  @def WEAVE_ERROR_SECURITY_MANAGER_BUSY
 *
 *  @brief
 *    A security manager is busy.
 *
 */
#define WEAVE_ERROR_SECURITY_MANAGER_BUSY                       _WEAVE_ERROR(55)

/**
 *  @def WEAVE_ERROR_INVALID_PASE_PARAMETER
 *
 *  @brief
 *    A PASE parameter is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_PASE_PARAMETER                      _WEAVE_ERROR(56)

/**
 *  @def WEAVE_ERROR_PASE_SUPPORTS_ONLY_CONFIG1
 *
 *  @brief
 *    PASE supports only config1.
 *
 */
#define WEAVE_ERROR_PASE_SUPPORTS_ONLY_CONFIG1                  _WEAVE_ERROR(57)

/**
 *  @def WEAVE_ERROR_KEY_CONFIRMATION_FAILED
 *
 *  @brief
 *    A key confirmation failed.
 *
 */
#define WEAVE_ERROR_KEY_CONFIRMATION_FAILED                     _WEAVE_ERROR(58)

/**
 *  @def WEAVE_ERROR_INVALID_USE_OF_SESSION_KEY
 *
 *  @brief
 *    A use of session key is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_USE_OF_SESSION_KEY                  _WEAVE_ERROR(59)

/**
 *  @def WEAVE_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY
 *
 *  @brief
 *    A connection is closed unexpectedly.
 *
 */
#define WEAVE_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY              _WEAVE_ERROR(60)

/**
 *  @def WEAVE_ERROR_MISSING_TLV_ELEMENT
 *
 *  @brief
 *    A TLV element is missing.
 *
 */
#define WEAVE_ERROR_MISSING_TLV_ELEMENT                         _WEAVE_ERROR(61)

/**
 *  @def WEAVE_ERROR_RANDOM_DATA_UNAVAILABLE
 *
 *  @brief
 *    Secure random data is not available.
 *
 */
#define WEAVE_ERROR_RANDOM_DATA_UNAVAILABLE                     _WEAVE_ERROR(62)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_HOST_PORT_ELEMENT
 *
 *  @brief
 *    A type in host/port list is unsupported.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_HOST_PORT_ELEMENT               _WEAVE_ERROR(63)

/**
 *  @def WEAVE_ERROR_INVALID_HOST_SUFFIX_INDEX
 *
 *  @brief
 *    A suffix index in host/port list is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_HOST_SUFFIX_INDEX                   _WEAVE_ERROR(64)

/**
 *  @def WEAVE_ERROR_HOST_PORT_LIST_EMPTY
 *
 *  @brief
 *    A host/port list is empty.
 *
 */
#define WEAVE_ERROR_HOST_PORT_LIST_EMPTY                        _WEAVE_ERROR(65)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_AUTH_MODE
 *
 *  @brief
 *    An authentication mode is unsupported.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_AUTH_MODE                       _WEAVE_ERROR(66)

/**
 *  @def WEAVE_ERROR_INVALID_SERVICE_EP
 *
 *  @brief
 *    A service endpoint is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_SERVICE_EP                          _WEAVE_ERROR(67)

/**
 *  @def WEAVE_ERROR_INVALID_DIRECTORY_ENTRY_TYPE
 *
 *  @brief
 *    A directory entry type is unknown.
 *
 */
#define WEAVE_ERROR_INVALID_DIRECTORY_ENTRY_TYPE                _WEAVE_ERROR(68)

/**
 *  @def WEAVE_ERROR_FORCED_RESET
 *
 *  @brief
 *    A service manager is forced to reset.
 *
 */
#define WEAVE_ERROR_FORCED_RESET                                _WEAVE_ERROR(69)

/**
 *  @def WEAVE_ERROR_NO_ENDPOINT
 *
 *  @brief
 *    No endpoint is available.
 *
 */
#define WEAVE_ERROR_NO_ENDPOINT                                 _WEAVE_ERROR(70)

/**
 *  @def WEAVE_ERROR_INVALID_DESTINATION_NODE_ID
 *
 *  @brief
 *    A destination node id is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_DESTINATION_NODE_ID                 _WEAVE_ERROR(71)

/**
 *  @def WEAVE_ERROR_NOT_CONNECTED
 *
 *  @brief
 *    The operation cannot be performed because the underlying object is not
 *    connected.
 *
 */
#define WEAVE_ERROR_NOT_CONNECTED                               _WEAVE_ERROR(72)

/**
 *  @def WEAVE_ERROR_NO_SW_UPDATE_AVAILABLE
 *
 *  @brief
 *    No software update is available.
 *
 */
#define WEAVE_ERROR_NO_SW_UPDATE_AVAILABLE                      _WEAVE_ERROR(73)

/**
 *  @def WEAVE_ERROR_CA_CERT_NOT_FOUND
 *
 *  @brief
 *    CA certificate is not found.
 *
 */
#define WEAVE_ERROR_CA_CERT_NOT_FOUND                           _WEAVE_ERROR(74)

/**
 *  @def WEAVE_ERROR_CERT_PATH_LEN_CONSTRAINT_EXCEEDED
 *
 *  @brief
 *    A certificate path length exceeds the constraint.
 *
 */
#define WEAVE_ERROR_CERT_PATH_LEN_CONSTRAINT_EXCEEDED           _WEAVE_ERROR(75)

/**
 *  @def WEAVE_ERROR_CERT_PATH_TOO_LONG
 *
 *  @brief
 *    A certificate path is too long.
 *
 */
#define WEAVE_ERROR_CERT_PATH_TOO_LONG                          _WEAVE_ERROR(76)

/**
 *  @def WEAVE_ERROR_CERT_USAGE_NOT_ALLOWED
 *
 *  @brief
 *    A requested certificate usage is not allowed.
 *
 */
#define WEAVE_ERROR_CERT_USAGE_NOT_ALLOWED                      _WEAVE_ERROR(77)

/**
 *  @def WEAVE_ERROR_CERT_EXPIRED
 *
 *  @brief
 *    A certificate expired.
 *
 */
#define WEAVE_ERROR_CERT_EXPIRED                                _WEAVE_ERROR(78)

/**
 *  @def WEAVE_ERROR_CERT_NOT_VALID_YET
 *
 *  @brief
 *    A certificate is not valid yet.
 *
 */
#define WEAVE_ERROR_CERT_NOT_VALID_YET                          _WEAVE_ERROR(79)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_CERT_FORMAT
 *
 *  @brief
 *    A certificate format is unsupported.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_CERT_FORMAT                     _WEAVE_ERROR(80)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_ELLIPTIC_CURVE
 *
 *  @brief
 *    An elliptic curve is unsupported.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_ELLIPTIC_CURVE                  _WEAVE_ERROR(81)

/**
 *  @def WEAVE_CERT_NOT_USED
 *
 *  @brief
 *    A certificate was not used during the chain validation.
 *
 */
#define WEAVE_CERT_NOT_USED                                     _WEAVE_ERROR(82)

/**
 *  @def WEAVE_ERROR_CERT_NOT_FOUND
 *
 *  @brief
 *    A certificate is not found.
 *
 */
#define WEAVE_ERROR_CERT_NOT_FOUND                              _WEAVE_ERROR(83)

/**
 *  @def WEAVE_ERROR_INVALID_CASE_PARAMETER
 *
 *  @brief
 *    A CASE parameter is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_CASE_PARAMETER                      _WEAVE_ERROR(84)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_CASE_CONFIGURATION
 *
 *  @brief
 *    A CASE configuration is unsupported.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_CASE_CONFIGURATION              _WEAVE_ERROR(85)

/**
 *  @def WEAVE_ERROR_CERT_LOAD_FAIL
 *
 *  @brief
 *    A certificate load failed.
 *
 */
#define WEAVE_ERROR_CERT_LOAD_FAIL                              _WEAVE_ERROR(86)

/**
 *  @def WEAVE_ERROR_CERT_NOT_TRUSTED
 *
 *  @brief
 *    A certificate is not trusted.
 *
 */
#define WEAVE_ERROR_CERT_NOT_TRUSTED                            _WEAVE_ERROR(87)

/**
 *  @def WEAVE_ERROR_INVALID_ACCESS_TOKEN
 *
 *  @brief
 *    An access token is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_ACCESS_TOKEN                        _WEAVE_ERROR(88)

/**
 *  @def WEAVE_ERROR_WRONG_CERT_SUBJECT
 *
 *  @brief
 *    A certificate subject is wrong.
 *
 */
#define WEAVE_ERROR_WRONG_CERT_SUBJECT                          _WEAVE_ERROR(89)

// deprecated alias
#define WEAVE_ERROR_WRONG_CERTIFICATE_SUBJECT WEAVE_ERROR_WRONG_CERT_SUBJECT

/**
 *  @def WEAVE_ERROR_INVALID_PROVISIONING_BUNDLE
 *
 *  @brief
 *    A provisioning bundle is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_PROVISIONING_BUNDLE                 _WEAVE_ERROR(90)

/**
 *  @def WEAVE_ERROR_PROVISIONING_BUNDLE_DECRYPTION_ERROR
 *
 *  @brief
 *    A provision bundle encountered a decryption error.
 *
 */
#define WEAVE_ERROR_PROVISIONING_BUNDLE_DECRYPTION_ERROR        _WEAVE_ERROR(91)

/**
 *  @def WEAVE_ERROR_WRONG_NODE_ID
 *
 *  @brief
 *    A node id is wrong.
 *
 */
#define WEAVE_ERROR_WRONG_NODE_ID                               _WEAVE_ERROR(92)

/**
 *  @def WEAVE_ERROR_CONN_ACCEPTED_ON_WRONG_PORT
 *
 *  @brief
 *    A connection is accepted on a wrong port.
 *
 */
#define WEAVE_ERROR_CONN_ACCEPTED_ON_WRONG_PORT                 _WEAVE_ERROR(93)

/**
 *  @def WEAVE_ERROR_CALLBACK_REPLACED
 *
 *  @brief
 *    An application callback has been replaced.
 *
 */
#define WEAVE_ERROR_CALLBACK_REPLACED                           _WEAVE_ERROR(94)

/**
 *  @def WEAVE_ERROR_NO_CASE_AUTH_DELEGATE
 *
 *  @brief
 *    No CASE authentication delegate is set.
 *
 */
#define WEAVE_ERROR_NO_CASE_AUTH_DELEGATE                       _WEAVE_ERROR(95)

/**
 *  @def WEAVE_ERROR_DEVICE_LOCATE_TIMEOUT
 *
 *  @brief
 *    The attempt to locate device timed out.
 *
 */
#define WEAVE_ERROR_DEVICE_LOCATE_TIMEOUT                       _WEAVE_ERROR(96)

/**
 *  @def WEAVE_ERROR_DEVICE_CONNECT_TIMEOUT
 *
 *  @brief
 *    The attempt to connect device timed out.
 *
 */
#define WEAVE_ERROR_DEVICE_CONNECT_TIMEOUT                      _WEAVE_ERROR(97)

/**
 *  @def WEAVE_ERROR_DEVICE_AUTH_TIMEOUT
 *
 *  @brief
 *    The attempt to authenticate device timed out.
 *
 */
#define WEAVE_ERROR_DEVICE_AUTH_TIMEOUT                         _WEAVE_ERROR(98)

/**
 *  @def WEAVE_ERROR_MESSAGE_NOT_ACKNOWLEDGED
 *
 *  @brief
 *    A message is not acknowledged after max retries.
 *
 */
#define WEAVE_ERROR_MESSAGE_NOT_ACKNOWLEDGED                    _WEAVE_ERROR(99)

/**
 *  @def WEAVE_ERROR_RETRANS_TABLE_FULL
 *
 *  @brief
 *    A retransmission table is already full.
 *
 */
#define WEAVE_ERROR_RETRANS_TABLE_FULL                          _WEAVE_ERROR(100)

/**
 *  @def WEAVE_ERROR_INVALID_ACK_ID
 *
 *  @brief
 *    An acknowledgment id is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_ACK_ID                              _WEAVE_ERROR(101)

/**
 *  @def WEAVE_ERROR_SEND_THROTTLED
 *
 *  @brief
 *    A send is throttled.
 *
 */
#define WEAVE_ERROR_SEND_THROTTLED                              _WEAVE_ERROR(102)

/**
 *  @def WEAVE_ERROR_WRONG_MSG_VERSION_FOR_EXCHANGE
 *
 *  @brief
 *    A message version is not supported by the current exchange context.
 *
 */
#define WEAVE_ERROR_WRONG_MSG_VERSION_FOR_EXCHANGE              _WEAVE_ERROR(103)

/**
 *  @def WEAVE_ERROR_TRANSACTION_CANCELED
 *
 *  @brief
 *    A transaction is cancelled.
 *
 */
#define WEAVE_ERROR_TRANSACTION_CANCELED                        _WEAVE_ERROR(104)

/**
 *  @def WEAVE_ERROR_LISTENER_ALREADY_STARTED
 *
 *  @brief
 *    A listener has already started.
 *
 */
#define WEAVE_ERROR_LISTENER_ALREADY_STARTED                    _WEAVE_ERROR(105)

/**
 *  @def WEAVE_ERROR_LISTENER_ALREADY_STOPPED
 *
 *  @brief
 *    A listener has already stopped.
 *
 */
#define WEAVE_ERROR_LISTENER_ALREADY_STOPPED                    _WEAVE_ERROR(106)

/**
 *  @def WEAVE_ERROR_UNKNOWN_TOPIC
 *
 *  @brief
 *    A topic ID was unknown to the recipient.
 *
 */
#define WEAVE_ERROR_UNKNOWN_TOPIC                               _WEAVE_ERROR(107)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_WEAVE_FEATURE
 *
 *  @brief
 *    A Weave feature is unsupported.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_WEAVE_FEATURE                   _WEAVE_ERROR(108)

/**
 *  @def WEAVE_ERROR_PASE_RECONFIGURE_REQUIRED
 *
 *  @brief
 *    PASE is required to reconfigure.
 *
 */
#define WEAVE_ERROR_PASE_RECONFIGURE_REQUIRED                   _WEAVE_ERROR(109)

/**
 *  @def WEAVE_ERROR_INVALID_PASE_CONFIGURATION
 *
 *  @brief
 *    A PASE configuration is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_PASE_CONFIGURATION                  _WEAVE_ERROR(110)

/**
 *  @def WEAVE_ERROR_NO_COMMON_PASE_CONFIGURATIONS
 *
 *  @brief
 *    No PASE configuration is in common.
 *
 */
#define WEAVE_ERROR_NO_COMMON_PASE_CONFIGURATIONS               _WEAVE_ERROR(111)

/**
 *  @def WEAVE_ERROR_UNSOLICITED_MSG_NO_ORIGINATOR
 *
 *  @brief
 *    An unsolicited message with the originator bit clear.
 *
 */
#define WEAVE_ERROR_UNSOLICITED_MSG_NO_ORIGINATOR               _WEAVE_ERROR(112)

/**
 *  @def WEAVE_ERROR_INVALID_FABRIC_ID
 *
 *  @brief
 *    A fabric id is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_FABRIC_ID                           _WEAVE_ERROR(113)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_TUNNEL_VERSION
 *
 *  @brief
 *    A tunnel version is unsupported.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_TUNNEL_VERSION                  _WEAVE_ERROR(114)

/**
 *  @def WEAVE_ERROR_TUNNEL_NEXTHOP_TABLE_FULL
 *
 *  @brief
 *    A tunnel nexthop table is full.
 *
 */
#define WEAVE_ERROR_TUNNEL_NEXTHOP_TABLE_FULL                   _WEAVE_ERROR(115)

/**
 *  @def WEAVE_ERROR_TUNNEL_SERVICE_QUEUE_FULL
 *
 *  @brief
 *    A tunnel service queue is full.
 *
 */
#define WEAVE_ERROR_TUNNEL_SERVICE_QUEUE_FULL                   _WEAVE_ERROR(116)

/**
 *  @def WEAVE_ERROR_DRBG_ENTROPY_SOURCE_FAILED
 *
 *  @brief
 *    DRBG entropy source failed to generate entropy data.
 *
 */
#define WEAVE_ERROR_DRBG_ENTROPY_SOURCE_FAILED                  _WEAVE_ERROR(117)

/**
 *  @def WEAVE_ERROR_TLV_TAG_NOT_FOUND
 *
 *  @brief
 *    A specified TLV tag was not found.
 *
 */
#define WEAVE_ERROR_TLV_TAG_NOT_FOUND                           _WEAVE_ERROR(118)

/**
 *  @def WEAVE_ERROR_INVALID_TOKENPAIRINGBUNDLE
 *
 *  @brief
 *    A token pairing bundle is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_TOKENPAIRINGBUNDLE                  _WEAVE_ERROR(119)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_TOKENPAIRINGBUNDLE_VERSION
 *
 *  @brief
 *    A token pairing bundle is invalid.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_TOKENPAIRINGBUNDLE_VERSION      _WEAVE_ERROR(120)

/**
 *  @def WEAVE_ERROR_NO_TAKE_AUTH_DELEGATE
 *
 *  @brief
 *    No TAKE authentication delegate is set.
 *
 */
#define WEAVE_ERROR_NO_TAKE_AUTH_DELEGATE                       _WEAVE_ERROR(121)

/**
 *  @def WEAVE_ERROR_TAKE_RECONFIGURE_REQUIRED
 *
 *  @brief
 *    TAKE requires a reconfigure.
 *
 */
#define WEAVE_ERROR_TAKE_RECONFIGURE_REQUIRED                   _WEAVE_ERROR(122)

/**
 *  @def WEAVE_ERROR_TAKE_REAUTH_POSSIBLE
 *
 *  @brief
 *    TAKE can do a reauthentication.
 *
 */
#define WEAVE_ERROR_TAKE_REAUTH_POSSIBLE                        _WEAVE_ERROR(123)

/**
 *  @def WEAVE_ERROR_INVALID_TAKE_PARAMETER
 *
 *  @brief
 *    Received an invalid TAKE paramter.
 *
 */
#define WEAVE_ERROR_INVALID_TAKE_PARAMETER                      _WEAVE_ERROR(124)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_TAKE_CONFIGURATION
 *
 *  @brief
 *    This configuration is not supported by TAKE.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_TAKE_CONFIGURATION              _WEAVE_ERROR(125)

/**
 *  @def WEAVE_ERROR_TAKE_TOKEN_IDENTIFICATION_FAILED
 *
 *  @brief
 *    The TAKE Token Identification failed.
 *
 */
#define WEAVE_ERROR_TAKE_TOKEN_IDENTIFICATION_FAILED            _WEAVE_ERROR(126)

/**
 *  @def WEAVE_ERROR_KEY_NOT_FOUND_FROM_PEER
 *
 *  @brief
 *    The encryption key is not found error received from a peer node.
 *
 */
#define WEAVE_ERROR_KEY_NOT_FOUND_FROM_PEER                     _WEAVE_ERROR(127)

/**
 *  @def WEAVE_ERROR_WRONG_ENCRYPTION_TYPE_FROM_PEER
 *
 *  @brief
 *    The wrong encryption type error received from a peer node.
 *
 */
#define WEAVE_ERROR_WRONG_ENCRYPTION_TYPE_FROM_PEER             _WEAVE_ERROR(128)

/**
 *  @def WEAVE_ERROR_UNKNOWN_KEY_TYPE_FROM_PEER
 *
 *  @brief
 *    The unknown key type error received from a peer node.
 *
 */
#define WEAVE_ERROR_UNKNOWN_KEY_TYPE_FROM_PEER                  _WEAVE_ERROR(129)

/**
 *  @def WEAVE_ERROR_INVALID_USE_OF_SESSION_KEY_FROM_PEER
 *
 *  @brief
 *    The invalid use of session key error received from a peer node.
 *
 */
#define WEAVE_ERROR_INVALID_USE_OF_SESSION_KEY_FROM_PEER        _WEAVE_ERROR(130)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_ENCRYPTION_TYPE_FROM_PEER
 *
 *  @brief
 *    An unsupported encryption type error received from a peer node.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_ENCRYPTION_TYPE_FROM_PEER       _WEAVE_ERROR(131)

/**
 *  @def WEAVE_ERROR_INTERNAL_KEY_ERROR_FROM_PEER
 *
 *  @brief
 *    The internal key error received from a peer node.
 *
 */
#define WEAVE_ERROR_INTERNAL_KEY_ERROR_FROM_PEER                _WEAVE_ERROR(132)

/**
 *  @def WEAVE_ERROR_INVALID_KEY_ID
 *
 *  @brief
 *    A key id is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_KEY_ID                              _WEAVE_ERROR(133)

/**
 *  @def WEAVE_ERROR_INVALID_TIME
 *
 *  @brief
 *    Time has invalid value.
 *
 */
#define WEAVE_ERROR_INVALID_TIME                                _WEAVE_ERROR(134)

/**
 *  @def WEAVE_ERROR_TUNNEL_PEER_ENTRY_NOT_FOUND
 *
 *  @brief
 *    A tunnel shortcut peer entry not found in the cache.
 *
 */
#define WEAVE_ERROR_TUNNEL_PEER_ENTRY_NOT_FOUND                 _WEAVE_ERROR(135)

/**
 *  @def WEAVE_ERROR_LOCKING_FAILURE
 *
 *  @brief
 *    Failure to acquire or release an OS provided mutex.
 *
 */
#define WEAVE_ERROR_LOCKING_FAILURE                             _WEAVE_ERROR(136)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_PASSCODE_CONFIG
 *
 *  @brief
 *    A passcode encryption configuration is unsupported.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_PASSCODE_CONFIG                 _WEAVE_ERROR(137)

/**
 *  @def WEAVE_ERROR_PASSCODE_AUTHENTICATION_FAILED
 *
 *  @brief
 *    The Weave passcode authentication failed.
 *
 */
#define WEAVE_ERROR_PASSCODE_AUTHENTICATION_FAILED              _WEAVE_ERROR(138)

/**
 *  @def WEAVE_ERROR_PASSCODE_FINGERPRINT_FAILED
 *
 *  @brief
 *    The Weave passcode fingerprint failed.
 *
 */
#define WEAVE_ERROR_PASSCODE_FINGERPRINT_FAILED                 _WEAVE_ERROR(139)

/**
 *  @def WEAVE_ERROR_TUNNEL_FORCE_ABORT
 *
 *  @brief
 *    The Weave error code to be used with the API for stopping
 *    the tunnel to enforce it to abort its TCP connection and return
 *    synchronously to the caller.
 *
 */
#define WEAVE_ERROR_TUNNEL_FORCE_ABORT                          _WEAVE_ERROR(140)

/**
 *  @def WEAVE_ERROR_SERIALIZATION_ELEMENT_NULL
 *
 *  @brief
 *      The element of the struct is null.
 *
 */
#define WEAVE_ERROR_SERIALIZATION_ELEMENT_NULL                  _WEAVE_ERROR(141)

/**
 *  @def WEAVE_ERROR_WRONG_CERT_SIGNATURE_ALGORITHM
 *
 *  @brief
 *    The certificate was not signed using the required signature algorithm.
 *
 */
#define WEAVE_ERROR_WRONG_CERT_SIGNATURE_ALGORITHM              _WEAVE_ERROR(142)

/**
 *  @def WEAVE_ERROR_WRONG_WEAVE_SIGNATURE_ALGORITHM
 *
 *  @brief
 *    The Weave signature was not signed using the required signature algorithm.
 *
 */
#define WEAVE_ERROR_WRONG_WEAVE_SIGNATURE_ALGORITHM             _WEAVE_ERROR(143)

/**
 *  @def WEAVE_ERROR_WDM_SCHEMA_MISMATCH
 *
 *  @brief
 *    A mismatch in schema was encountered.
 *
 */
#define WEAVE_ERROR_WDM_SCHEMA_MISMATCH                         _WEAVE_ERROR(144)

/**
 *  @def WEAVE_ERROR_INVALID_INTEGER_VALUE
 *
 *  @brief
 *    An integer does not have the kind of value we expect.
 *
 */
#define WEAVE_ERROR_INVALID_INTEGER_VALUE                       _WEAVE_ERROR(145)

/**
 *  @def WEAVE_ERROR_CASE_RECONFIG_REQUIRED
 *
 *  @brief
 *    CASE is required to reconfigure.
 *
 */
#define WEAVE_ERROR_CASE_RECONFIG_REQUIRED                      _WEAVE_ERROR(146)

/**
 *  @def WEAVE_ERROR_TOO_MANY_CASE_RECONFIGURATIONS
 *
 *  @brief
 *    Too many CASE reconfigurations were received.
 *
 */
#define WEAVE_ERROR_TOO_MANY_CASE_RECONFIGURATIONS              _WEAVE_ERROR(147)

/**
 *  @def WEAVE_ERROR_BAD_REQUEST
 *
 *  @brief
 *    The request cannot be processed or fulfilled
 *
 */
#define WEAVE_ERROR_BAD_REQUEST                                 _WEAVE_ERROR(148)

/**
 *  @def WEAVE_ERROR_INVALID_MESSAGE_FLAG
 *
 *  @brief
 *    One or more message flags have invalid value.
 *
 */
#define WEAVE_ERROR_INVALID_MESSAGE_FLAG                        _WEAVE_ERROR(149)

/**
 *  @def WEAVE_ERROR_KEY_EXPORT_RECONFIGURE_REQUIRED
 *
 *  @brief
 *    Key export protocol required to reconfigure.
 *
 */
#define WEAVE_ERROR_KEY_EXPORT_RECONFIGURE_REQUIRED             _WEAVE_ERROR(150)

/**
 *  @def WEAVE_ERROR_INVALID_KEY_EXPORT_CONFIGURATION
 *
 *  @brief
 *    A key export protocol configuration is invalid.
 *
 */
#define WEAVE_ERROR_INVALID_KEY_EXPORT_CONFIGURATION            _WEAVE_ERROR(151)

/**
 *  @def WEAVE_ERROR_NO_COMMON_KEY_EXPORT_CONFIGURATIONS
 *
 *  @brief
 *    No key export protocol configuration is in common.
 *
 */
#define WEAVE_ERROR_NO_COMMON_KEY_EXPORT_CONFIGURATIONS         _WEAVE_ERROR(152)

/**
 *  @def WEAVE_ERROR_NO_KEY_EXPORT_DELEGATE
 *
 *  @brief
 *    No key export delegate is set.
 *
 */
#define WEAVE_ERROR_NO_KEY_EXPORT_DELEGATE                      _WEAVE_ERROR(153)

/**
 *  @def WEAVE_ERROR_UNAUTHORIZED_KEY_EXPORT_REQUEST
 *
 *  @brief
 *    Unauthorized key export request.
 *
 */
#define WEAVE_ERROR_UNAUTHORIZED_KEY_EXPORT_REQUEST             _WEAVE_ERROR(154)

/**
 *  @def WEAVE_ERROR_UNAUTHORIZED_KEY_EXPORT_RESPONSE
 *
 *  @brief
 *    Unauthorized key export response.
 *
 */
#define WEAVE_ERROR_UNAUTHORIZED_KEY_EXPORT_RESPONSE            _WEAVE_ERROR(155)

/**
 *  @def WEAVE_ERROR_EXPORTED_KEY_AUTHENTICATION_FAILED
 *
 *  @brief
 *    The Weave exported encrypted key authentication failed.
 *
 */
#define WEAVE_ERROR_EXPORTED_KEY_AUTHENTICATION_FAILED          _WEAVE_ERROR(156)

/**
 *  @def WEAVE_ERROR_TOO_MANY_SHARED_SESSION_END_NODES
 *
 *  @brief
 *    The number of shared secure sessions end nodes exceeds
 *    the maximum limit.
 *
 */
#define WEAVE_ERROR_TOO_MANY_SHARED_SESSION_END_NODES           _WEAVE_ERROR(157)

/**
 * @def WEAVE_ERROR_WDM_MALFORMED_DATA_ELEMENT
 *
 * @brief
 *   The WDM DataElement is malformed: it either does not contain
 *   the required elements, or it contais both the MergeData element
 *   and DeletedDictionaryKeyList.
 */
#define WEAVE_ERROR_WDM_MALFORMED_DATA_ELEMENT                  _WEAVE_ERROR(158)

/**
 * @def WEAVE_ERROR_WRONG_CERT_TYPE
 *
 * @brief
 *   The presented certificate was of the wrong type.
 */
#define WEAVE_ERROR_WRONG_CERT_TYPE                             _WEAVE_ERROR(159)

/**
 * @def WEAVE_ERROR_DEFAULT_EVENT_HANDLER_NOT_CALLED
 *
 * @brief
 *   The application's event handler failed to call the default event handler function
 *   when presented with an unknown event.
 */
#define WEAVE_ERROR_DEFAULT_EVENT_HANDLER_NOT_CALLED            _WEAVE_ERROR(162)

/**
 *  @def WEAVE_ERROR_PERSISTED_STORAGE_FAIL
 *
 *  @brief
 *    Persisted storage memory read/write failure.
 *
 */
#define WEAVE_ERROR_PERSISTED_STORAGE_FAIL                      _WEAVE_ERROR(163)

/**
 *  @def WEAVE_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND
 *
 *  @brief
 *    The specific value is not found in the persisted storage.
 *
 */
#define WEAVE_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND           _WEAVE_ERROR(164)

/**
 *  @def WEAVE_ERROR_PROFILE_STRING_CONTEXT_ALREADY_REGISTERED
 *
 *  @brief
 *    The specified profile string support context is already registered.
 *
 */
#define WEAVE_ERROR_PROFILE_STRING_CONTEXT_ALREADY_REGISTERED   _WEAVE_ERROR(165)

/**
 *  @def WEAVE_ERROR_PROFILE_STRING_CONTEXT_NOT_REGISTERED
 *
 *  @brief
 *    The specified profile string support context is not registered.
 *
 */
#define WEAVE_ERROR_PROFILE_STRING_CONTEXT_NOT_REGISTERED       _WEAVE_ERROR(166)

/**
 *  @def WEAVE_ERROR_INCOMPATIBLE_SCHEMA_VERSION
 *
 *  @brief
 *    Encountered a mismatch in compatibility w.r.t to IDL schema version
 */
#define WEAVE_ERROR_INCOMPATIBLE_SCHEMA_VERSION                 _WEAVE_ERROR(167)

/**
 *  @def WEAVE_ERROR_TUNNEL_ROUTING_RESTRICTED
 *
 *  @brief
 *    Indicates that the Tunnel can only be used by the border gateway
 *    for itself and, it cannot forward packets for any other device.
 *
 */
#define WEAVE_ERROR_TUNNEL_ROUTING_RESTRICTED                   _WEAVE_ERROR(168)

/**
 *  @def WEAVE_ERROR_TUNNEL_RESET_RECONNECT_ALREADY_ARMED
 *
 *  @brief
 *    The Tunnel reset reconnect timer is already armed
 */
#define WEAVE_ERROR_TUNNEL_RESET_RECONNECT_ALREADY_ARMED        _WEAVE_ERROR(169)

/**
 *  @def WEAVE_ERROR_MISMATCH_UPDATE_REQUIRED_VERSION
 *
 *  @brief
 *    Encountered a mismatch between wdm update required version and current version
 */
#define WEAVE_ERROR_MISMATCH_UPDATE_REQUIRED_VERSION            _WEAVE_ERROR(170)

/**
 * @def WEAVE_ERROR_WDM_MALFORMED_STATUS_ELEMENT
 *
 * @brief
 *   The WDM StatusElement is malformed: it does not contain
 *   either the profile id or the status code.
 */
#define WEAVE_ERROR_WDM_MALFORMED_STATUS_ELEMENT                _WEAVE_ERROR(171)

/**
 * @def WEAVE_ERROR_WDM_SUBSCRIPTIONLESS_NOTIFY_PARTIAL
 *
 * @brief
 *   The WDM Subscriptionless Notify is partial.
 */
#define WEAVE_ERROR_WDM_SUBSCRIPTIONLESS_NOTIFY_PARTIAL         _WEAVE_ERROR(172)

/**
 *  @def WEAVE_ERROR_ACCESS_DENIED
 *
 *  @brief
 *    The Weave message is not granted access for further processing.
 */
#define WEAVE_ERROR_ACCESS_DENIED                               _WEAVE_ERROR(173)

/**
 *  @def WEAVE_ERROR_UNKNOWN_RESOURCE_ID
 *
 *  @brief
 *    Unknown resource ID
 *
 */
#define WEAVE_ERROR_UNKNOWN_RESOURCE_ID                          _WEAVE_ERROR(174)

/**
 * @def WEAVE_ERROR_WDM_MALFORMED_UPDATE_RESPONSE
 *
 * @brief
 *   The WDM UpdateResponse payload is malformed: it does not contain
 *   either the StatusList or the VersionList.
 */
#define WEAVE_ERROR_WDM_MALFORMED_UPDATE_RESPONSE                _WEAVE_ERROR(175)

/**
 * @def WEAVE_ERROR_WDM_VERSION_MISMATCH
 *
 * @brief
 *   The conditional update of a trait instance path has failed
 *   because the local changes are based on an obsolete version of the
 *   data.
 */
#define WEAVE_ERROR_WDM_VERSION_MISMATCH                         _WEAVE_ERROR(176)

/**
 * @def WEAVE_ERROR_WDM_POTENTIAL_DATA_LOSS
 *
 * @brief
 *   A potential data loss was detected for a Trait Instance.
 */
#define WEAVE_ERROR_WDM_POTENTIAL_DATA_LOSS                      _WEAVE_ERROR(177)

/**
 *  @def WEAVE_ERROR_UNSUPPORTED_THREAD_NETWORK_CREATE
 *
 *  @brief
 *    Device doesn't support standalone Thread network creation.
 *    On some legacy Nest devices new Thread network can only be created
 *    together with Weave Fabric using CrateFabric() message.
 *
 */
#define WEAVE_ERROR_UNSUPPORTED_THREAD_NETWORK_CREATE            _WEAVE_ERROR(178)

/**
 *  @def WEAVE_ERROR_WDM_INCONSISTENT_CONDITIONALITY
 *
 *  @brief
 *    A TraitPath was declared updated with a conditionality that
 *    does not match that of other TraitPaths already updated in the
 *    same Trait Instance.
 *
 */
#define WEAVE_ERROR_WDM_INCONSISTENT_CONDITIONALITY              _WEAVE_ERROR(179)

/**
 *  @def WEAVE_ERROR_WDM_LOCAL_DATA_INCONSISTENT
 *
 *  @brief
 *    The local data does not match any known version of the
 *    Trait Instance and cannot support the operation requested.
 *
 */
#define WEAVE_ERROR_WDM_LOCAL_DATA_INCONSISTENT                  _WEAVE_ERROR(180)

/**
 *  @def WEAVE_ERROR_WDM_PATH_STORE_FULL
 *
 *  @brief
 *    WDM cannot store a TraitPath for lack of memory.
 *
 */
#define WEAVE_ERROR_WDM_PATH_STORE_FULL                          _WEAVE_ERROR(181)
/**
 * @def WEAVE_EVENT_ID_FOUND
 *
 * @brief
 *   Event ID matching the criteria was found
 */
#define WEAVE_EVENT_ID_FOUND                                     _WEAVE_ERROR(182)

/**
 *  @}
 */

// !!!!! IMPORTANT !!!!!  If you add new Weave errors, please update the translation
// of error codes to strings in WeaveError.cpp, and add them to unittest
// in test-apps/TestErrorStr.cpp

// clang-format on

namespace nl {
namespace Weave {

extern bool FormatWeaveError(char * buf, uint16_t bufSize, int32_t err);

} // namespace Weave
} // namespace nl

#endif // WEAVE_ERROR_H
