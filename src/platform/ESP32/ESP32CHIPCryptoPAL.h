/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
