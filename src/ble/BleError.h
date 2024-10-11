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

#ifndef _CHIP_BLE_BLE_H
#error "Please include <ble/Ble.h> instead!"
#endif

#include <cstdint>

#include <lib/core/CHIPError.h>

#define CHIP_BLE_ERROR(e) CHIP_SDK_ERROR(::chip::ChipError::SdkPart::kBLE, (e))

// clang-format off

/**
 *  @name Error Definitions
 *
 *  @{
 */

/**
 *  @def BLE_ERROR_ADAPTER_UNAVAILABLE
 *
 *  @brief
 *    Bluetooth LE adapter is (currently) unavailable.
 *
 */
#define BLE_ERROR_ADAPTER_UNAVAILABLE                      CHIP_BLE_ERROR(0x01)

// unused                                                  CHIP_BLE_ERROR(0x02)

/**
 *  @def BLE_ERROR_NO_CONNECTION_RECEIVED_CALLBACK
 *
 *  @brief
 *    No callback was registered to receive a BLE Transport Protocol (BTP)
 *    connection.
 *
 */
#define BLE_ERROR_NO_CONNECTION_RECEIVED_CALLBACK          CHIP_BLE_ERROR(0x03)

/**
 *  @def BLE_ERROR_CENTRAL_UNSUBSCRIBED
 *
 *  @brief
 *    A BLE central device unsubscribed from a peripheral device's BLE
 *    Transport Protocol (BTP) transmit characteristic.
 *
 */
#define BLE_ERROR_CENTRAL_UNSUBSCRIBED                     CHIP_BLE_ERROR(0x04)

/**
 *  @def BLE_ERROR_GATT_SUBSCRIBE_FAILED
 *
 *  @brief
 *    A BLE central device failed to subscribe to a peripheral device's BLE
 *    Transport Protocol (BTP) transmit characteristic.
 *
 */
#define BLE_ERROR_GATT_SUBSCRIBE_FAILED                    CHIP_BLE_ERROR(0x05)

/**
 *  @def BLE_ERROR_GATT_UNSUBSCRIBE_FAILED
 *
 *  @brief
 *    A BLE central device failed to unsubscribe from a peripheral device's
 *    BLE Transport Protocol (BTP) transmit characteristic.
 *
 */
#define BLE_ERROR_GATT_UNSUBSCRIBE_FAILED                  CHIP_BLE_ERROR(0x06)

/**
 *  @def BLE_ERROR_GATT_WRITE_FAILED
 *
 *  @brief
 *    A General Attribute Profile (GATT) write operation failed.
 *
 */
#define BLE_ERROR_GATT_WRITE_FAILED                        CHIP_BLE_ERROR(0x07)

/**
 *  @def BLE_ERROR_GATT_INDICATE_FAILED
 *
 *  @brief
 *    A General Attribute Profile (GATT) indicate operation failed.
 *
 */
#define BLE_ERROR_GATT_INDICATE_FAILED                     CHIP_BLE_ERROR(0x08)

// unused                                                  CHIP_BLE_ERROR(0x09)
// unused                                                  CHIP_BLE_ERROR(0x0a)

/**
 *  @def BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT
 *
 *  @brief
 *    A BLE Transport Protocol (BTP) error was encountered.
 *
 */
#define BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT                  CHIP_BLE_ERROR(0x0b)

/**
 *  @def BLE_ERROR_REMOTE_DEVICE_DISCONNECTED
 *
 *  @brief
 *    A remote BLE connection peer disconnected, either actively or due to the
 *    expiration of a BLE connection supervision timeout.
 *
 */
#define BLE_ERROR_REMOTE_DEVICE_DISCONNECTED               CHIP_BLE_ERROR(0x0c)

/**
 *  @def BLE_ERROR_APP_CLOSED_CONNECTION
 *
 *  @brief
 *    The local application closed a BLE connection, and has informed BleLayer.
 *
 */
#define BLE_ERROR_APP_CLOSED_CONNECTION                    CHIP_BLE_ERROR(0x0d)

// unused                                                  CHIP_BLE_ERROR(0x0e)

/**
 *  @def BLE_ERROR_NOT_CHIP_DEVICE
 *
 *  @brief
 *    A BLE peripheral device did not expose the General Attribute Profile
 *    (GATT) service required by the Bluetooth Transport Protocol (BTP).
 *
 */
#define BLE_ERROR_NOT_CHIP_DEVICE                         CHIP_BLE_ERROR(0x0f)

/**
 *  @def BLE_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS
 *
 *  @brief
 *    A remote device does not offer a compatible version of the Bluetooth
 *    Transport Protocol (BTP).
 *
 */
#define BLE_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS           CHIP_BLE_ERROR(0x10)

// unused                                                  CHIP_BLE_ERROR(0x11)
// unused                                                  CHIP_BLE_ERROR(0x12)

/**
 *  @def BLE_ERROR_INVALID_FRAGMENT_SIZE
 *
 *  @brief
 *    A remote device selected in invalid Bluetooth Transport Protocol (BTP)
 *    fragment size.
 *
 */
#define BLE_ERROR_INVALID_FRAGMENT_SIZE                    CHIP_BLE_ERROR(0x13)

/**
 *  @def BLE_ERROR_START_TIMER_FAILED
 *
 *  @brief
 *    A timer failed to start within BleLayer.
 *
 */
#define BLE_ERROR_START_TIMER_FAILED                       CHIP_BLE_ERROR(0x14)

/**
 *  @def BLE_ERROR_CONNECT_TIMED_OUT
 *
 *  @brief
 *    A remote BLE peripheral device's Bluetooth Transport Protocol (BTP)
 *    connect handshake response timed out.
 *
 */
#define BLE_ERROR_CONNECT_TIMED_OUT                        CHIP_BLE_ERROR(0x15)

/**
 *  @def BLE_ERROR_RECEIVE_TIMED_OUT
 *
 *  @brief
 *    A remote BLE central device's Bluetooth Transport Protocol (BTP) connect
 *    handshake timed out.
 *
 */
#define BLE_ERROR_RECEIVE_TIMED_OUT                        CHIP_BLE_ERROR(0x16)

/**
 *  @def BLE_ERROR_INVALID_MESSAGE
 *
 *  @brief
 *    An invalid Bluetooth Transport Protocol (BTP) message was received.
 *
 */
#define BLE_ERROR_INVALID_MESSAGE                          CHIP_BLE_ERROR(0x17)

/**
 *  @def BLE_ERROR_FRAGMENT_ACK_TIMED_OUT
 *
 *  @brief
 *    Receipt of an expected Bluetooth Transport Protocol (BTP) fragment
 *    acknowledgement timed out.
 *
 */
#define BLE_ERROR_FRAGMENT_ACK_TIMED_OUT                   CHIP_BLE_ERROR(0x18)

/**
 *  @def BLE_ERROR_KEEP_ALIVE_TIMED_OUT
 *
 *  @brief
 *    Receipt of an expected Bluetooth Transport Protocol (BTP) keep-alive
 *    fragment timed out.
 *
 */
#define BLE_ERROR_KEEP_ALIVE_TIMED_OUT                     CHIP_BLE_ERROR(0x19)

/**
 *  @def BLE_ERROR_NO_CONNECT_COMPLETE_CALLBACK
 *
 *  @brief
 *    No callback was registered to handle Bluetooth Transport Protocol (BTP)
 *    connect completion.
 *
 */
#define BLE_ERROR_NO_CONNECT_COMPLETE_CALLBACK            CHIP_BLE_ERROR(0x1a)

/**
 *  @def BLE_ERROR_INVALID_ACK
 *
 *  @brief
 *    A Bluetooth Transport Protcol (BTP) fragment acknowledgement was invalid.
 *
 */
#define BLE_ERROR_INVALID_ACK                              CHIP_BLE_ERROR(0x1b)

/**
 *  @def BLE_ERROR_REASSEMBLER_MISSING_DATA
 *
 *  @brief
 *    A Bluetooth Transport Protocol (BTP) end-of-message fragment was
 *    received, but the total size of the received fragments is less than
 *    the indicated size of the original fragmented message.
 *
 */
#define BLE_ERROR_REASSEMBLER_MISSING_DATA                 CHIP_BLE_ERROR(0x1c)

/**
 *  @def BLE_ERROR_INVALID_BTP_HEADER_FLAGS
 *
 *  @brief
 *    A set of Bluetooth Transport Protocol (BTP) header flags is invalid.
 *
 */
#define BLE_ERROR_INVALID_BTP_HEADER_FLAGS                 CHIP_BLE_ERROR(0x1d)

/**
 *  @def BLE_ERROR_INVALID_BTP_SEQUENCE_NUMBER
 *
 *  @brief
 *    A Bluetooth Transport Protocol (BTP) fragment sequence number is invalid.
 *
 */
#define BLE_ERROR_INVALID_BTP_SEQUENCE_NUMBER              CHIP_BLE_ERROR(0x1e)

/**
 *  @def BLE_ERROR_REASSEMBLER_INCORRECT_STATE
 *
 *  @brief
 *    The Bluetooth Transport Protocol (BTP) message reassembly engine
 *    encountered an unexpected state.
 *
 */
#define BLE_ERROR_REASSEMBLER_INCORRECT_STATE              CHIP_BLE_ERROR(0x1f)

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
extern bool FormatLayerError(char * buf, uint16_t bufSize, CHIP_ERROR err);

} /* namespace Ble */
} /* namespace chip */
