/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines functions for encoding and decoding CHIP messages.
 *      The encoded messages contain CHIP packet header, encrypted payload
 *      header, encrypted payload and message authentication code, as per
 *      CHIP specifications.
 *
 */

#pragma once

#include <transport/CryptoContext.h>
#include <transport/SecureSession.h>

namespace chip {

namespace SecureMessageCodec {

/**
 * @brief
 *  Attach payload header to the message and encrypt the message buffer using
 *  key from the secure session.
 *
 * @param session       The secure session context with the peer node
 * @param payloadHeader Reference to the payload header that should be inserted in
 *                      the message
 * @param packetHeader  Reference to the packet header that contains unencrypted
 *                      portion of the message header
 * @param msgBuf        The message buffer that contains the unencrypted message. If
 *                      the operation is successful, this buffer will be mutated to contain
 *                      the encrypted message.
 * @return A CHIP_ERROR value consistent with the result of the encryption operation
 */
CHIP_ERROR Encrypt(const CryptoContext & context, CryptoContext::ConstNonceView nonce, PayloadHeader & payloadHeader,
                   PacketHeader & packetHeader, System::PacketBufferHandle & msgBuf);

/**
 * @brief
 *  Decrypt the message, perform message integrity check, and decode the payload header,
 *  consuming the header from the packet in doing so.
 *
 * @param session       The secure session context with the peer node
 * @param payloadHeader Reference to the payload header that will be recovered from the message
 * @param packetHeader  Reference to the packet header that contains unencrypted
 *                      portion of the message header
 * @param msgBuf        The message buffer that contains the encrypted message. If
 *                      the operation is successful, this buffer will be mutated to contain
 *                      the decrypted message.
 * @return A CHIP_ERROR value consistent with the result of the decryption operation
 */
CHIP_ERROR Decrypt(const CryptoContext & context, CryptoContext::ConstNonceView nonce, PayloadHeader & payloadHeader,
                   const PacketHeader & packetHeader, System::PacketBufferHandle & msgBuf);

} // namespace SecureMessageCodec

} // namespace chip
