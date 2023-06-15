/*
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements the utility class for the Matter Checkin protocol.
 */

#pragma once

#include <crypto/CHIPCryptoPAL.h>
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif
#include <lib/support/Base64.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMessageDispatch.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/PairingSession.h>
#include <protocols/secure_channel/SessionEstablishmentExchangeDispatch.h>
#include <system/SystemPacketBuffer.h>
#include <transport/CryptoContext.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace Protocols {
namespace SecureChannel {
using namespace Crypto;

#define CHIP_CHECK_IN_APP_DATA_MAX_SIZE 1024

/**
 * @brief   Implement section 4.18.2 of the spec regarding
 *          Check-in message payload
 *
 */
class DLL_EXPORT CheckinMessage
{
public:
    ~CheckinMessage();
    /**
     * @brief Generate Check-in Message payload
     *
     * @param key       Key with which to encrypt the check-in payload
     * @param counter   Check-in counter
     * @param appData   Optionnal : Application Data to incorporate within the Check-in message
     * @param output    Buffer in Which to store the generated payload. SUFFICIENT SPACE MUST BE ALLOCATED by the caller
     *                  Required Buffer Size is : User Data + CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES + SizeOf(uint32_t) +
     * CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES
     * @param payloadSize (Optionnal) Total size of the generated payload
     * @return CHIP_ERROR
     */
    static CHIP_ERROR GenerateCheckingMessagePayload(Crypto::Aes128KeyHandle & key, uint32_t counter, ByteSpan appData,
                                                     uint8_t * output, uint16_t * payloadSize = nullptr);

    static CHIP_ERROR ParseCheckingMessagePayload(Crypto::Aes128KeyHandle & key, ByteSpan & payload, uint32_t & counter,
                                                  uint8_t * appData);

    static uint16_t GetAppDataSize(ByteSpan & payload);
};

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip
