/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <esp_encrypted_img.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {

/**
 * @brief Adds pre-encrypted OTA support to a sub-image processor (esp_encrypted_img).
 *
 * Inherited by a processor that handles encrypted images: the key is supplied once via the public
 * InitEncryptedOTA(), and the protected hooks decrypt each chunk during the transfer. The mechanics
 * are protected so inheriting the helper does not widen the processor's public interface.
 */
class EncryptedOTAHelper
{
public:
    /*
     * @brief Initialize the encrypted OTA helper with the RSA private key.
     * @param rsaPrivKey The RSA private key in PEM format.
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_INVALID_ARGUMENT if the key is empty, or CHIP_ERROR_INCORRECT_STATE if the key
     * is already configured.
     */
    CHIP_ERROR InitEncryptedOTA(CharSpan rsaPrivKey);

protected:
    /*
     * @brief Check if the encrypted OTA helper is enabled.
     * @return True if the encrypted OTA helper is enabled, false otherwise.
     */
    bool IsEncryptedOTAEnabled() const { return !mKey.empty(); }

    /*
     * @brief Check if a decryption session is currently open.
     * @return True if a decryption session is currently open, false otherwise.
     */
    bool IsDecrypting() const { return mHandle != nullptr; }

    /*
     * @brief Open a decryption session using the configured key.
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_INTERNAL if the decryption session cannot be opened.
     */
    CHIP_ERROR DecryptStart();

    /**
     * @brief Decrypt one chunk.
     * @param in  Ciphertext bytes received off the wire.
     * @param out Set to the plaintext produced by this call; may be empty while the decryptor
     *            buffers across chunk boundaries. Owned by the helper, valid until the next call.
     */
    CHIP_ERROR Decrypt(ByteSpan in, ByteSpan & out);

    /*
     * @brief Finish the session and verify the image footer.
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_INTERNAL if the decryption session cannot be finished.
     */
    CHIP_ERROR DecryptEnd();

    /*
     * @brief Tear down the session without verification (error/cancel path).
     */
    void DecryptAbort();

private:
    // Release the caller-owned buffer returned by the last Decrypt(), if any.
    void FreeDecryptedBuffer();

    // PEM RSA private key. Not copied — the caller's buffer must outlive this helper.
    CharSpan mKey;
    esp_decrypt_handle_t mHandle = nullptr;
    void * mDecryptedBuffer      = nullptr;
};

} // namespace chip
