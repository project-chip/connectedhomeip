/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *  key from the crypto context.
 *
 * @param[in] context         Reference to the crypto context used for encryption, from the secure session.
 * @param[in] nonce           The nonce to be used for encryption.
 * @param[in] payloadHeader   Reference to the payload header that should be inserted in
 *                            the message.
 * @param[in] packetHeader    Reference to the packet header that contains unencrypted
 *                            portion of the message header.
 * @param[in,out] msgBuf      The message buffer that contains the unencrypted message. If
 *                            the operation is successful, this buffer will be mutated to contain
 *                            the encrypted message and any trailing MIC generated.
 * @return A CHIP_ERROR value consistent with the result of the encryption operation.
 */
CHIP_ERROR Encrypt(const CryptoContext & context, CryptoContext::ConstNonceView nonce, PayloadHeader & payloadHeader,
                   PacketHeader & packetHeader, System::PacketBufferHandle & msgBuf);

/**
 * @brief
 *  Decrypt the message, perform message integrity check, and decode the payload header,
 *  consuming the header from the packet in doing so.
 *
 * @param[in] context         Reference to the crypto context used for encryption, from the secure session.
 * @param[in] nonce           The nonce from the message to be used during decryption.
 * @param[out] payloadHeader  Reference to the payload header that will be recovered from the message.
 * @param[in] packetHeader    Reference to the packet header that contains unencrypted
 *                            portion of the message header.
 * @param[in,out] msgBuf      The message buffer that contains the encrypted message. If
 *                            the operation is successful, this buffer will be mutated to contain
 *                            the decrypted message and any trailing MIC will no longer be present.
 * @return A CHIP_ERROR value consistent with the result of the decryption operation.
 */
CHIP_ERROR Decrypt(const CryptoContext & context, CryptoContext::ConstNonceView nonce, PayloadHeader & payloadHeader,
                   const PacketHeader & packetHeader, System::PacketBufferHandle & msgBuf);

} // namespace SecureMessageCodec

} // namespace chip
