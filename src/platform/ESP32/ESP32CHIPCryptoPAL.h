/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
 *      This files overrides few APIs from mbedTLS based implementation of CHIP crypto primitives
 *      for using ECDSA peripheral on ESP32 chips.
 */
#pragma once

#include <crypto/CHIPCryptoPAL.h>

namespace chip {
namespace Crypto {

class ESP32P256Keypair : public P256Keypair
{
public:
    /**
     * @brief Initialize the keypair with efuse block key
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Initialize(ECPKeyTarget keyTarget, int efuseBlock);

    CHIP_ERROR ECDSA_sign_msg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature) const override;
};

} // namespace Crypto
} // namespace chip
