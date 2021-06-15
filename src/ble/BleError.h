/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
 *      This file defines constants for the CHIP BleLayer subsystem.
 *
 *      Error types, ranges, and mappings overrides may be made by
 *      defining the appropriate BLE_CONFIG_* or _BLE_CONFIG_*
 *      macros.
 *
 *  NOTE WELL: On some platforms, this header is included by C-language programs.
 *
 */

#pragma once

#include "BleConfig.h"

// clang-format off

/**
 *  @def BLE_NO_ERROR
 *
 *  @brief
 *    This defines the BleLayer error code for success or no
 *    error. This value may be configured via #BLE_CONFIG_NO_ERROR.
 *
 */
#define BLE_NO_ERROR                   BLE_CONFIG_NO_ERROR

/**
 *  @def BLE_ERROR_MIN
 *
 *  @brief
 *    This defines the base or minimum value of the BleLayer error number
 *    range. This value may be configured via #BLE_CONFIG_ERROR_MIN.
 *
 */
#define BLE_ERROR_MIN                  BLE_CONFIG_ERROR_MIN

/**
 *  @def BLE_ERROR_MAX
 *
 *  @brief
 *    This defines the top or maximum value of the BleLayer error number
 *    range. This value may be configured via #BLE_CONFIG_ERROR_MAX.
 *
 */
#define BLE_ERROR_MAX                  BLE_CONFIG_ERROR_MAX

/**
 *  @def _BLE_ERROR(e)
 *
 *  @brief
 *    This defines a mapping function for BleLayer errors that allows
 *    mapping such errors into a platform- or system-specific
 *    range. This function may be configured via
 *    #_BLE_CONFIG_ERROR.
 *
 *  @param[in]  e  The BleLayer error to map.
 *
 *  @return The mapped BleLayer error.
 *
 */
#define _BLE_ERROR(e)                  _BLE_CONFIG_ERROR(e)

/**
 *  @typedef BLE_ERROR
 *
 *  The basic type for all BleLayer errors.
 *
 *  This is defined to a platform- or system-specific type.
 *
 */
typedef BLE_CONFIG_ERROR_TYPE BLE_ERROR;

/**
 *  Format specifier for the BLE_ERROR type.
 *
 */
#define BleErrorFormat BLE_CONFIG_ERROR_FORMAT

/**
 *  @name Error Definitions
 *
 *  @{
 */

/**
 *  @def BLE_ERROR_BAD_ARGS
 *
 *  @brief
 *    An invalid argument or arguments were supplied.
 *
 */
#define BLE_ERROR_BAD_ARGS                                 _BLE_ERROR(0)

/**
 *  @def BLE_ERROR_INCORRECT_STATE
 *
 *  @brief
 *    An unexpected state was encountered.
 *
 */
#define BLE_ERROR_INCORRECT_STATE                          _BLE_ERROR(1)

/**
 *  @def BLE_ERROR_NO_ENDPOINTS
 *
 *  @brief
 *    No BLE endpoint is available.
 *
 */
#define BLE_ERROR_NO_ENDPOINTS                             _BLE_ERROR(2)

/**
 *  @def BLE_ERROR_NO_CONNECTION_RECEIVED_CALLBACK
 *
 *  @brief
 *    No callback was registered to receive a BLE Transport Protocol (BTP)
 *    connection.
 *
 */
#define BLE_ERROR_NO_CONNECTION_RECEIVED_CALLBACK          _BLE_ERROR(3)

/**
 *  @def BLE_ERROR_CENTRAL_UNSUBSCRIBED
 *
 *  @brief
 *    A BLE central device unsubscribed from a peripheral device's BLE
 *    Transport Protocol (BTP) transmit characteristic.
 *
 */
#define BLE_ERROR_CENTRAL_UNSUBSCRIBED                     _BLE_ERROR(4)

/**
 *  @def BLE_ERROR_GATT_SUBSCRIBE_FAILED
 *
 *  @brief
 *    A BLE central device failed to subscribe to a peripheral device's BLE
 *    Transport Protocol (BTP) transmit characteristic.
 *
 */
#define BLE_ERROR_GATT_SUBSCRIBE_FAILED                    _BLE_ERROR(5)

/**
 *  @def BLE_ERROR_GATT_UNSUBSCRIBE_FAILED
 *
 *  @brief
 *    A BLE central device failed to unsubscribe from a peripheral device's
 *    BLE Transport Protocol (BTP) transmit characteristic.
 *
 */
#define BLE_ERROR_GATT_UNSUBSCRIBE_FAILED                  _BLE_ERROR(6)

/**
 *  @def BLE_ERROR_GATT_WRITE_FAILED
 *
 *  @brief
 *    A General Attribute Profile (GATT) write operation failed.
 *
 */
#define BLE_ERROR_GATT_WRITE_FAILED                        _BLE_ERROR(7)

/**
 *  @def BLE_ERROR_GATT_INDICATE_FAILED
 *
 *  @brief
 *    A General Attribute Profile (GATT) indicate operation failed.
 *
 */
#define BLE_ERROR_GATT_INDICATE_FAILED                     _BLE_ERROR(8)

/**
 *  @def BLE_ERROR_NOT_IMPLEMENTED
 *
 *  @brief
 *    A requested function or feature is not implemented.
 *
 */
#define BLE_ERROR_NOT_IMPLEMENTED                          _BLE_ERROR(9)

/*
 * Unused                                                  _BLE_ERROR(10)
 */

/**
 *  @def BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT
 *
 *  @brief
 *    A BLE Transport Protocol (BTP) error was encountered.
 *
 */
#define BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT                     _BLE_ERROR(11)

/**
 *  @def BLE_ERROR_REMOTE_DEVICE_DISCONNECTED
 *
 *  @brief
 *    A remote BLE connection peer disconnected, either actively or due to the
 *    expiration of a BLE connection supervision timeout.
 *
 */
#define BLE_ERROR_REMOTE_DEVICE_DISCONNECTED               _BLE_ERROR(12)

/**
 *  @def BLE_ERROR_APP_CLOSED_CONNECTION
 *
 *  @brief
 *    The local application closed a BLE connection, and has informed BleLayer.
 *
 */
#define BLE_ERROR_APP_CLOSED_CONNECTION                    _BLE_ERROR(13)

/**
 *  @def BLE_ERROR_OUTBOUND_MESSAGE_TOO_BIG
 *
 *  @brief
 *    More outbound message data is pending than available buffer space
 *    available to copy it.
 *
 */
#define BLE_ERROR_OUTBOUND_MESSAGE_TOO_BIG                 _BLE_ERROR(14)

/**
 *  @def BLE_ERROR_NOT_CHIP_DEVICE
 *
 *  @brief
 *    A BLE peripheral device did not expose the General Attribute Profile
 *    (GATT) service required by the Bluetooth Transport Protocol (BTP).
 *
 */
#define BLE_ERROR_NOT_CHIP_DEVICE                         _BLE_ERROR(15)

/**
 *  @def BLE_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS
 *
 *  @brief
 *    A remote device does not offer a compatible version of the Bluetooth
 *    Transport Protocol (BTP).
 *
 */
#define BLE_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS           _BLE_ERROR(16)

/**
 *  @def BLE_ERROR_NO_MEMORY
 *
 *  @brief
 *    A request for memory could not be fulfilled.
 *
 */
#define BLE_ERROR_NO_MEMORY                                _BLE_ERROR(17)

/**
 *  @def BLE_ERROR_MESSAGE_INCOMPLETE
 *
 *  @brief
 *    A received Bluetooth Transport Protocol (BTP) message was incomplete.
 *
 */
#define BLE_ERROR_MESSAGE_INCOMPLETE                       _BLE_ERROR(18)

/**
 *  @def BLE_ERROR_INVALID_FRAGMENT_SIZE
 *
 *  @brief
 *    A remote device selected in invalid Bluetooth Transport Protocol (BTP)
 *    fragment size.
 *
 */
#define BLE_ERROR_INVALID_FRAGMENT_SIZE                    _BLE_ERROR(19)

/**
 *  @def BLE_ERROR_START_TIMER_FAILED
 *
 *  @brief
 *    A timer failed to start within BleLayer.
 *
 */
#define BLE_ERROR_START_TIMER_FAILED                       _BLE_ERROR(20)

/**
 *  @def BLE_ERROR_CONNECT_TIMED_OUT
 *
 *  @brief
 *    A remote BLE peripheral device's Bluetooth Transport Protocol (BTP)
 *    connect handshake response timed out.
 *
 */
#define BLE_ERROR_CONNECT_TIMED_OUT                        _BLE_ERROR(21)

/**
 *  @def BLE_ERROR_RECEIVE_TIMED_OUT
 *
 *  @brief
 *    A remote BLE central device's Bluetooth Transport Protocol (BTP) connect
 *    handshake timed out.
 *
 */
#define BLE_ERROR_RECEIVE_TIMED_OUT                        _BLE_ERROR(22)

/**
 *  @def BLE_ERROR_INVALID_MESSAGE
 *
 *  @brief
 *    An invalid Bluetooth Transport Protocol (BTP) message was received.
 *
 */
#define BLE_ERROR_INVALID_MESSAGE                          _BLE_ERROR(23)

/**
 *  @def BLE_ERROR_FRAGMENT_ACK_TIMED_OUT
 *
 *  @brief
 *    Receipt of an expected Bluetooth Transport Protocol (BTP) fragment
 *    acknowledgement timed out.
 *
 */
#define BLE_ERROR_FRAGMENT_ACK_TIMED_OUT                   _BLE_ERROR(24)

/**
 *  @def BLE_ERROR_KEEP_ALIVE_TIMED_OUT
 *
 *  @brief
 *    Receipt of an expected Bluetooth Transport Protocol (BTP) keep-alive
 *    fragment timed out.
 *
 */
#define BLE_ERROR_KEEP_ALIVE_TIMED_OUT                     _BLE_ERROR(25)

/**
 *  @def BLE_ERROR_NO_CONNECT_COMPLETE_CALLBACK
 *
 *  @brief
 *    No callback was registered to handle Bluetooth Transport Protocol (BTP)
 *    connect completion.
 *
 */
#define BLE_ERROR_NO_CONNECT_COMPLETE_CALLBACK            _BLE_ERROR(26)

/**
 *  @def BLE_ERROR_INVALID_ACK
 *
 *  @brief
 *    A Bluetooth Transport Protcol (BTP) fragment acknowledgement was invalid.
 *
 */
#define BLE_ERROR_INVALID_ACK                              _BLE_ERROR(27)

/**
 *  @def BLE_ERROR_REASSEMBLER_MISSING_DATA
 *
 *  @brief
 *    A Bluetooth Transport Protocol (BTP) end-of-message fragment was
 *    received, but the total size of the received fragments is less than
 *    the indicated size of the original fragmented message.
 *
 */
#define BLE_ERROR_REASSEMBLER_MISSING_DATA                 _BLE_ERROR(28)

/**
 *  @def BLE_ERROR_INVALID_BTP_HEADER_FLAGS
 *
 *  @brief
 *    A set of Bluetooth Transport Protocol (BTP) header flags is invalid.
 *
 */
#define BLE_ERROR_INVALID_BTP_HEADER_FLAGS                 _BLE_ERROR(29)

/**
 *  @def BLE_ERROR_INVALID_BTP_SEQUENCE_NUMBER
 *
 *  @brief
 *    A Bluetooth Transport Protocol (BTP) fragment sequence number is invalid.
 *
 */
#define BLE_ERROR_INVALID_BTP_SEQUENCE_NUMBER              _BLE_ERROR(30)

/**
 *  @def BLE_ERROR_REASSEMBLER_INCORRECT_STATE
 *
 *  @brief
 *    The Bluetooth Transport Protocol (BTP) message reassembly engine
 *    encountered an unexpected state.
 *
 */
#define BLE_ERROR_REASSEMBLER_INCORRECT_STATE              _BLE_ERROR(31)

/**
 *  @def BLE_ERROR_RECEIVED_MESSAGE_TOO_BIG
 *
 *  @brief
 *    More inbound message data is pending than available buffer space
 *    available to copy it.
 *
 */
#define BLE_ERROR_RECEIVED_MESSAGE_TOO_BIG                 _BLE_ERROR(32)

// !!!!! IMPORTANT !!!!!  If you add new Ble errors, please update the translation
// of error codes to strings in BleError.cpp, and add them to unittest
// in test-apps/TestErrorStr.cpp

/**
 *  @}
 */

// clang-format on

namespace chip {
namespace Ble {

extern void RegisterLayerErrorFormatter();
extern bool FormatLayerError(char * buf, uint16_t bufSize, int32_t err);

} /* namespace Ble */
} /* namespace chip */
