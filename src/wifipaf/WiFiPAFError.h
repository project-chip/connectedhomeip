/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      This file defines constants for the CHIP Wi-Fi PAF subsystem.
 *
 *  NOTE WELL: On some platforms, this header is included by C-language programs.
 *
 */

#pragma once

#include <cstdint>
#include <lib/core/CHIPError.h>

// Reuse the BLE error code space since there is no available space
#define CHIP_WIFIPAF_ERROR(e) CHIP_SDK_ERROR(::chip::ChipError::SdkPart::kBLE, (e))

// clang-format off

/**
 *  @name Error Definitions
 *
 */

// unused                                                  CHIP_WIFIPAF_ERROR(0x01)
// unused                                                  CHIP_WIFIPAF_ERROR(0x02)

/**
 *  @def WIFIPAF_ERROR_NO_CONNECTION_RECEIVED_CALLBACK
 *
 *  @brief
 *    No callback was registered to receive a Wi-Fi PAF Transport Protocol (PAFTP)
 *    connection.
 *
 */
#define WIFIPAF_ERROR_NO_CONNECTION_RECEIVED_CALLBACK          CHIP_WIFIPAF_ERROR(0x03)

// unused                                                  CHIP_WIFIPAF_ERROR(0x04)
// unused                                                  CHIP_WIFIPAF_ERROR(0x05)
// unused                                                  CHIP_WIFIPAF_ERROR(0x06)
// unused                                                  CHIP_WIFIPAF_ERROR(0x07)
// unused                                                  CHIP_WIFIPAF_ERROR(0x08)
// unused                                                  CHIP_WIFIPAF_ERROR(0x09)
// unused                                                  CHIP_WIFIPAF_ERROR(0x0a)


/**
 *  @def WIFIPAF_ERROR_CHIPPAF_PROTOCOL_ABORT
 *
 *  @brief
 *    A Wi-Fi PAF Transport Protocol (PAFTP) error was encountered.
 *
 */
#define WIFIPAF_ERROR_CHIPPAF_PROTOCOL_ABORT                  CHIP_WIFIPAF_ERROR(0x0b)

/**
 *  @def WIFIPAF_ERROR_REMOTE_DEVICE_DISCONNECTED
 *
 *  @brief
 *    A remote WIFIPAF connection peer disconnected, either actively or due to the
 *    expiration of a WIFIPAF connection supervision timeout.
 *
 */
#define WIFIPAF_ERROR_REMOTE_DEVICE_DISCONNECTED               CHIP_WIFIPAF_ERROR(0x0c)

/**
 *  @def WIFIPAF_ERROR_APP_CLOSED_CONNECTION
 *
 *  @brief
 *    The local application closed a WIFIPAF connection, and has informed WIFIPAFLayer.
 *
 */
#define WIFIPAF_ERROR_APP_CLOSED_CONNECTION                    CHIP_WIFIPAF_ERROR(0x0d)

// unused                                                  CHIP_WIFIPAF_ERROR(0x0e)
// unused                                                  CHIP_WIFIPAF_ERROR(0x0f)


/**
 *  @def WIFIPAF_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS
 *
 *  @brief
 *    A remote device does not offer a compatible version of the Wi-Fi PAF
 *    Transport Protocol (PAFTP).
 *
 */
#define WIFIPAF_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS           CHIP_WIFIPAF_ERROR(0x10)

// unused                                                  CHIP_WIFIPAF_ERROR(0x11)
// unused                                                  CHIP_WIFIPAF_ERROR(0x12)

/**
 *  @def WIFIPAF_ERROR_INVALID_FRAGMENT_SIZE
 *
 *  @brief
 *    A remote device selected in invalid Wi-Fi PAF Transport Protocol (PAFTP)
 *    fragment size.
 *
 */
#define WIFIPAF_ERROR_INVALID_FRAGMENT_SIZE                    CHIP_WIFIPAF_ERROR(0x13)

/**
 *  @def WIFIPAF_ERROR_START_TIMER_FAILED
 *
 *  @brief
 *    A timer failed to start within WIFIPAFLayer.
 *
 */
#define WIFIPAF_ERROR_START_TIMER_FAILED                       CHIP_WIFIPAF_ERROR(0x14)

/**
 *  @def WIFIPAF_ERROR_CONNECT_TIMED_OUT
 *
 *  @brief
 *    A remote WIFIPAF peripheral device's Wi-Fi PAF Transport Protocol (PAFTP)
 *    connect handshake response timed out.
 *
 */
#define WIFIPAF_ERROR_CONNECT_TIMED_OUT                        CHIP_WIFIPAF_ERROR(0x15)

/**
 *  @def WIFIPAF_ERROR_RECEIVE_TIMED_OUT
 *
 *  @brief
 *    A remote WIFIPAF central device's Wi-Fi PAF Transport Protocol (PAFTP) connect
 *    handshake timed out.
 *
 */
#define WIFIPAF_ERROR_RECEIVE_TIMED_OUT                        CHIP_WIFIPAF_ERROR(0x16)

/**
 *  @def WIFIPAF_ERROR_INVALID_MESSAGE
 *
 *  @brief
 *    An invalid Wi-Fi PAF Transport Protocol (PAFTP) message was received.
 *
 */
#define WIFIPAF_ERROR_INVALID_MESSAGE                          CHIP_WIFIPAF_ERROR(0x17)

/**
 *  @def WIFIPAF_ERROR_FRAGMENT_ACK_TIMED_OUT
 *
 *  @brief
 *    Receipt of an expected Wi-Fi PAF Transport Protocol (PAFTP) fragment
 *    acknowledgement timed out.
 *
 */
#define WIFIPAF_ERROR_FRAGMENT_ACK_TIMED_OUT                   CHIP_WIFIPAF_ERROR(0x18)

/**
 *  @def WIFIPAF_ERROR_KEEP_ALIVE_TIMED_OUT
 *
 *  @brief
 *    Receipt of an expected Wi-Fi PAF Transport Protocol (PAFTP) keep-alive
 *    fragment timed out.
 *
 */
#define WIFIPAF_ERROR_KEEP_ALIVE_TIMED_OUT                     CHIP_WIFIPAF_ERROR(0x19)

/**
 *  @def WIFIPAF_ERROR_NO_CONNECT_COMPLETE_CALLBACK
 *
 *  @brief
 *    No callback was registered to handle Wi-Fi PAF Transport Protocol (PAFTP)
 *    connect completion.
 *
 */
#define WIFIPAF_ERROR_NO_CONNECT_COMPLETE_CALLBACK            CHIP_WIFIPAF_ERROR(0x1a)

/**
 *  @def WIFIPAF_ERROR_INVALID_ACK
 *
 *  @brief
 *    A Bluetooth Transport Protcol (PAFTP) fragment acknowledgement was invalid.
 *
 */
#define WIFIPAF_ERROR_INVALID_ACK                              CHIP_WIFIPAF_ERROR(0x1b)

/**
 *  @def WIFIPAF_ERROR_REASSEMBLER_MISSING_DATA
 *
 *  @brief
 *    A Wi-Fi PAF Transport Protocol (PAFTP) end-of-message fragment was
 *    received, but the total size of the received fragments is less than
 *    the indicated size of the original fragmented message.
 *
 */
#define WIFIPAF_ERROR_REASSEMBLER_MISSING_DATA                 CHIP_WIFIPAF_ERROR(0x1c)

/**
 *  @def WIFIPAF_ERROR_INVALID_PAFP_HEADER_FLAGS
 *
 *  @brief
 *    A set of Wi-Fi PAF Transport Protocol (PAFTP) header flags is invalid.
 *
 */
#define WIFIPAF_ERROR_INVALID_PAFTP_HEADER_FLAGS                 CHIP_WIFIPAF_ERROR(0x1d)

/**
 *  @def WIFIPAF_ERROR_INVALID_PAFTP_SEQUENCE_NUMBER
 *
 *  @brief
 *    A Wi-Fi PAF Transport Protocol (PAFTP) fragment sequence number is invalid.
 *
 */
#define WIFIPAF_ERROR_INVALID_PAFTP_SEQUENCE_NUMBER              CHIP_WIFIPAF_ERROR(0x1e)

/**
 *  @def WIFIPAF_ERROR_REASSEMBLER_INCORRECT_STATE
 *
 *  @brief
 *    The Wi-Fi PAF Transport Protocol (PAFTP) message reassembly engine
 *    encountered an unexpected state.
 *
 */
#define WIFIPAF_ERROR_REASSEMBLER_INCORRECT_STATE              CHIP_WIFIPAF_ERROR(0x1f)

/**
 *  @}
 */
// clang-format on

namespace chip {
namespace WiFiPAF {

extern void RegisterLayerErrorFormatter();
extern bool FormatLayerError(char * buf, uint16_t bufSize, CHIP_ERROR err);

} /* namespace WiFiPAF */
} /* namespace chip */
