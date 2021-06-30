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

#include <core/CHIPError.h>

// clang-format off

/**
 *  @def CHIP_BLE_ERROR(e)
 *
 *  @brief
 *    This defines a mapping function for BleLayer errors that allows
 *    mapping such errors into a platform- or system-specific
 *    range. This function may be configured via
 *    #BLE_CONFIG_ERROR.
 *
 *  @param[in]  e  The BleLayer error to map.
 *
 *  @return The mapped BleLayer error.
 *
 */
#define CHIP_BLE_ERROR(e)                                  BLE_CONFIG_ERROR(e)

/**
 *  @name Error Definitions
 *
 *  @{
 */

// unused                                                  CHIP_BLE_ERROR(0)
// unused                                                  CHIP_BLE_ERROR(1)
// unused                                                  CHIP_BLE_ERROR(2)

/**
 *  @def BLE_ERROR_NO_CONNECTION_RECEIVED_CALLBACK
 *
 *  @brief
 *    No callback was registered to receive a BLE Transport Protocol (BTP)
 *    connection.
 *
 */
#define BLE_ERROR_NO_CONNECTION_RECEIVED_CALLBACK          CHIP_BLE_ERROR(3)

/**
 *  @def BLE_ERROR_CENTRAL_UNSUBSCRIBED
 *
 *  @brief
 *    A BLE central device unsubscribed from a peripheral device's BLE
 *    Transport Protocol (BTP) transmit characteristic.
 *
 */
#define BLE_ERROR_CENTRAL_UNSUBSCRIBED                     CHIP_BLE_ERROR(4)

/**
 *  @def BLE_ERROR_GATT_SUBSCRIBE_FAILED
 *
 *  @brief
 *    A BLE central device failed to subscribe to a peripheral device's BLE
 *    Transport Protocol (BTP) transmit characteristic.
 *
 */
#define BLE_ERROR_GATT_SUBSCRIBE_FAILED                    CHIP_BLE_ERROR(5)

/**
 *  @def BLE_ERROR_GATT_UNSUBSCRIBE_FAILED
 *
 *  @brief
 *    A BLE central device failed to unsubscribe from a peripheral device's
 *    BLE Transport Protocol (BTP) transmit characteristic.
 *
 */
#define BLE_ERROR_GATT_UNSUBSCRIBE_FAILED                  CHIP_BLE_ERROR(6)

/**
 *  @def BLE_ERROR_GATT_WRITE_FAILED
 *
 *  @brief
 *    A General Attribute Profile (GATT) write operation failed.
 *
 */
#define BLE_ERROR_GATT_WRITE_FAILED                        CHIP_BLE_ERROR(7)

/**
 *  @def BLE_ERROR_GATT_INDICATE_FAILED
 *
 *  @brief
 *    A General Attribute Profile (GATT) indicate operation failed.
 *
 */
#define BLE_ERROR_GATT_INDICATE_FAILED                     CHIP_BLE_ERROR(8)

// unused                                                  CHIP_BLE_ERROR(9)
// unused                                                  CHIP_BLE_ERROR(10)

/**
 *  @def BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT
 *
 *  @brief
 *    A BLE Transport Protocol (BTP) error was encountered.
 *
 */
#define BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT                  CHIP_BLE_ERROR(11)

/**
 *  @def BLE_ERROR_REMOTE_DEVICE_DISCONNECTED
 *
 *  @brief
 *    A remote BLE connection peer disconnected, either actively or due to the
 *    expiration of a BLE connection supervision timeout.
 *
 */
#define BLE_ERROR_REMOTE_DEVICE_DISCONNECTED               CHIP_BLE_ERROR(12)

/**
 *  @def BLE_ERROR_APP_CLOSED_CONNECTION
 *
 *  @brief
 *    The local application closed a BLE connection, and has informed BleLayer.
 *
 */
#define BLE_ERROR_APP_CLOSED_CONNECTION                    CHIP_BLE_ERROR(13)

// unused                                                  CHIP_BLE_ERROR(14)

/**
 *  @def BLE_ERROR_NOT_CHIP_DEVICE
 *
 *  @brief
 *    A BLE peripheral device did not expose the General Attribute Profile
 *    (GATT) service required by the Bluetooth Transport Protocol (BTP).
 *
 */
#define BLE_ERROR_NOT_CHIP_DEVICE                         CHIP_BLE_ERROR(15)

/**
 *  @def BLE_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS
 *
 *  @brief
 *    A remote device does not offer a compatible version of the Bluetooth
 *    Transport Protocol (BTP).
 *
 */
#define BLE_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS           CHIP_BLE_ERROR(16)

// unused                                                  CHIP_BLE_ERROR(17)
// unused                                                  CHIP_BLE_ERROR(18)

/**
 *  @def BLE_ERROR_INVALID_FRAGMENT_SIZE
 *
 *  @brief
 *    A remote device selected in invalid Bluetooth Transport Protocol (BTP)
 *    fragment size.
 *
 */
#define BLE_ERROR_INVALID_FRAGMENT_SIZE                    CHIP_BLE_ERROR(19)

/**
 *  @def BLE_ERROR_START_TIMER_FAILED
 *
 *  @brief
 *    A timer failed to start within BleLayer.
 *
 */
#define BLE_ERROR_START_TIMER_FAILED                       CHIP_BLE_ERROR(20)

/**
 *  @def BLE_ERROR_CONNECT_TIMED_OUT
 *
 *  @brief
 *    A remote BLE peripheral device's Bluetooth Transport Protocol (BTP)
 *    connect handshake response timed out.
 *
 */
#define BLE_ERROR_CONNECT_TIMED_OUT                        CHIP_BLE_ERROR(21)

/**
 *  @def BLE_ERROR_RECEIVE_TIMED_OUT
 *
 *  @brief
 *    A remote BLE central device's Bluetooth Transport Protocol (BTP) connect
 *    handshake timed out.
 *
 */
#define BLE_ERROR_RECEIVE_TIMED_OUT                        CHIP_BLE_ERROR(22)

/**
 *  @def BLE_ERROR_INVALID_MESSAGE
 *
 *  @brief
 *    An invalid Bluetooth Transport Protocol (BTP) message was received.
 *
 */
#define BLE_ERROR_INVALID_MESSAGE                          CHIP_BLE_ERROR(23)

/**
 *  @def BLE_ERROR_FRAGMENT_ACK_TIMED_OUT
 *
 *  @brief
 *    Receipt of an expected Bluetooth Transport Protocol (BTP) fragment
 *    acknowledgement timed out.
 *
 */
#define BLE_ERROR_FRAGMENT_ACK_TIMED_OUT                   CHIP_BLE_ERROR(24)

/**
 *  @def BLE_ERROR_KEEP_ALIVE_TIMED_OUT
 *
 *  @brief
 *    Receipt of an expected Bluetooth Transport Protocol (BTP) keep-alive
 *    fragment timed out.
 *
 */
#define BLE_ERROR_KEEP_ALIVE_TIMED_OUT                     CHIP_BLE_ERROR(25)

/**
 *  @def BLE_ERROR_NO_CONNECT_COMPLETE_CALLBACK
 *
 *  @brief
 *    No callback was registered to handle Bluetooth Transport Protocol (BTP)
 *    connect completion.
 *
 */
#define BLE_ERROR_NO_CONNECT_COMPLETE_CALLBACK            CHIP_BLE_ERROR(26)

/**
 *  @def BLE_ERROR_INVALID_ACK
 *
 *  @brief
 *    A Bluetooth Transport Protcol (BTP) fragment acknowledgement was invalid.
 *
 */
#define BLE_ERROR_INVALID_ACK                              CHIP_BLE_ERROR(27)

/**
 *  @def BLE_ERROR_REASSEMBLER_MISSING_DATA
 *
 *  @brief
 *    A Bluetooth Transport Protocol (BTP) end-of-message fragment was
 *    received, but the total size of the received fragments is less than
 *    the indicated size of the original fragmented message.
 *
 */
#define BLE_ERROR_REASSEMBLER_MISSING_DATA                 CHIP_BLE_ERROR(28)

/**
 *  @def BLE_ERROR_INVALID_BTP_HEADER_FLAGS
 *
 *  @brief
 *    A set of Bluetooth Transport Protocol (BTP) header flags is invalid.
 *
 */
#define BLE_ERROR_INVALID_BTP_HEADER_FLAGS                 CHIP_BLE_ERROR(29)

/**
 *  @def BLE_ERROR_INVALID_BTP_SEQUENCE_NUMBER
 *
 *  @brief
 *    A Bluetooth Transport Protocol (BTP) fragment sequence number is invalid.
 *
 */
#define BLE_ERROR_INVALID_BTP_SEQUENCE_NUMBER              CHIP_BLE_ERROR(30)

/**
 *  @def BLE_ERROR_REASSEMBLER_INCORRECT_STATE
 *
 *  @brief
 *    The Bluetooth Transport Protocol (BTP) message reassembly engine
 *    encountered an unexpected state.
 *
 */
#define BLE_ERROR_REASSEMBLER_INCORRECT_STATE              CHIP_BLE_ERROR(31)

// !!!!! IMPORTANT !!!!!  If you add new Ble errors, please update the translation
// of error codes to strings in BleError.cpp, and add them to unittest
// in test-apps/TestErrorStr.cpp

/**
 *  @}
 */

// !!!!! IMPORTANT !!!!!
// These definitions are present temporarily in order to reduce breakage for PRs in flight.
// TODO: remove compatibility definitions
#define BLE_ERROR                               CHIP_ERROR
#define BLE_NO_ERROR                            CHIP_NO_ERROR
#define BLE_ERROR_BAD_ARGS                      CHIP_ERROR_INVALID_ARGUMENT
#define BLE_ERROR_INCORRECT_STATE               CHIP_ERROR_INCORRECT_STATE
#define BLE_ERROR_MESSAGE_INCOMPLETE            CHIP_ERROR_MESSAGE_INCOMPLETE
#define BLE_ERROR_NOT_IMPLEMENTED               CHIP_ERROR_NOT_IMPLEMENTED
#define BLE_ERROR_NO_ENDPOINTS                  CHIP_ERROR_ENDPOINT_POOL_FULL
#define BLE_ERROR_NO_MEMORY                     CHIP_ERROR_NO_MEMORY
#define BLE_ERROR_OUTBOUND_MESSAGE_TOO_BIG      CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG
#define BLE_ERROR_RECEIVED_MESSAGE_TOO_BIG      CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG

// clang-format on

namespace chip {
namespace Ble {

extern void RegisterLayerErrorFormatter();
extern bool FormatLayerError(char * buf, uint16_t bufSize, CHIP_ERROR err);

} /* namespace Ble */
} /* namespace chip */
