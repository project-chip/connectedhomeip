/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Provides implementations for the OpenWeave AES BlockCipher classes
 *          on the ESP32 platform.
 */

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>

#include <string.h>

#include <Weave/Support/crypto/WeaveCrypto.h>
#include <Weave/Support/crypto/AESBlockCipher.h>

#include <hwcrypto/aes.h>

namespace nl {
namespace Weave {
namespace Platform {
namespace Security {

using namespace nl::Weave::Crypto;

AES128BlockCipher::AES128BlockCipher()
{
    memset(&mKey, 0, sizeof(mKey));
}

AES128BlockCipher::~AES128BlockCipher()
{
    Reset();
}

void AES128BlockCipher::Reset()
{
    ClearSecretData((uint8_t *)&mKey, sizeof(mKey));
}

void AES128BlockCipherEnc::SetKey(const uint8_t *key)
{
    memcpy(mKey, key, kKeyLength);
}

void AES128BlockCipherEnc::EncryptBlock(const uint8_t *inBlock, uint8_t *outBlock)
{
    esp_aes_context ctx;

    esp_aes_init(&ctx);
    esp_aes_setkey(&ctx, mKey, kKeyLengthBits);
    esp_aes_encrypt(&ctx, inBlock, outBlock);
    esp_aes_free(&ctx);
}

void AES128BlockCipherDec::SetKey(const uint8_t *key)
{
    memcpy(mKey, key, kKeyLength);
}

void AES128BlockCipherDec::DecryptBlock(const uint8_t *inBlock, uint8_t *outBlock)
{
    esp_aes_context ctx;

    esp_aes_init(&ctx);
    esp_aes_setkey(&ctx, mKey, kKeyLengthBits);
    esp_aes_decrypt(&ctx, inBlock, outBlock);
    esp_aes_free(&ctx);
}

AES256BlockCipher::AES256BlockCipher()
{
    memset(&mKey, 0, sizeof(mKey));
}

AES256BlockCipher::~AES256BlockCipher()
{
    Reset();
}

void AES256BlockCipher::Reset()
{
    ClearSecretData((uint8_t *)&mKey, sizeof(mKey));
}

void AES256BlockCipherEnc::SetKey(const uint8_t *key)
{
    memcpy(mKey, key, kKeyLength);
}

void AES256BlockCipherEnc::EncryptBlock(const uint8_t *inBlock, uint8_t *outBlock)
{
    esp_aes_context ctx;

    esp_aes_init(&ctx);
    esp_aes_setkey(&ctx, mKey, kKeyLengthBits);
    esp_aes_encrypt(&ctx, inBlock, outBlock);
    esp_aes_free(&ctx);
}

void AES256BlockCipherDec::SetKey(const uint8_t *key)
{
    memcpy(mKey, key, kKeyLength);
}

void AES256BlockCipherDec::DecryptBlock(const uint8_t *inBlock, uint8_t *outBlock)
{
    esp_aes_context ctx;

    esp_aes_init(&ctx);
    esp_aes_setkey(&ctx, mKey, kKeyLengthBits);
    esp_aes_decrypt(&ctx, inBlock, outBlock);
    esp_aes_free(&ctx);
}

} // namespace Security
} // namespace Platform
} // namespace Weave
} // namespace nl

