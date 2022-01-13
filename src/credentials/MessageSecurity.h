/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 * @brief Defines the message encryption and decryption.
 */

#pragma once

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/DataModelTypes.h>
#include <transport/raw/MessageHeader.h>

namespace chip {

namespace MessageSecurity {

CHIP_ERROR Encrypt(const ByteSpan & encryptionKey, chip::PayloadHeader & payloadHeader, chip::PacketHeader & packetHeader,
                   chip::System::PacketBufferHandle & msgBuf);

CHIP_ERROR Decrypt(const ByteSpan & encryptionKey, chip::PayloadHeader & payloadHeader, const chip::PacketHeader & packetHeader,
                   chip::System::PacketBufferHandle & msg);

namespace Groups {

using OperationKeyType = uint8_t[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];

CHIP_ERROR DeriveOperationalKey(chip::FabricIndex fabric_index, chip::GroupId group_id, OperationKeyType out_key);
CHIP_ERROR DeriveOperationalKey(const chip::ByteSpan & epoch_key, OperationKeyType out_key);

} // namespace Groups

} // namespace MessageSecurity

} // namespace chip
