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

#include "EncryptedOTAHelper.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <stdlib.h>

namespace chip {

CHIP_ERROR EncryptedOTAHelper::InitEncryptedOTA(CharSpan rsaPrivKey)
{
    VerifyOrReturnError(!rsaPrivKey.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mKey.empty(), CHIP_ERROR_INCORRECT_STATE);
    mKey = rsaPrivKey;
    return CHIP_NO_ERROR;
}

CHIP_ERROR EncryptedOTAHelper::DecryptStart()
{
    VerifyOrReturnError(!mKey.empty(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mHandle == nullptr, CHIP_ERROR_INCORRECT_STATE);

    const esp_decrypt_cfg_t cfg = {
        .rsa_priv_key     = mKey.data(),
        .rsa_priv_key_len = mKey.size(),
    };
    mHandle = esp_encrypted_img_decrypt_start(&cfg);
    VerifyOrReturnError(mHandle != nullptr, CHIP_ERROR_INTERNAL,
                        ChipLogError(SoftwareUpdate, "esp_encrypted_img_decrypt_start failed"));
    return CHIP_NO_ERROR;
}

CHIP_ERROR EncryptedOTAHelper::Decrypt(ByteSpan in, ByteSpan & out)
{
    VerifyOrReturnError(mHandle != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // esp_encrypted_img_decrypt_data allocates a fresh output buffer each call and hands ownership to
    // the caller. Release the previous chunk's buffer (the caller consumed it synchronously last
    // call) before requesting the next.
    FreeDecryptedBuffer();

    pre_enc_decrypt_arg_t args = {
        .data_in      = reinterpret_cast<const char *>(in.data()),
        .data_in_len  = in.size(),
        .data_out     = nullptr,
        .data_out_len = 0,
    };

    esp_err_t err = esp_encrypted_img_decrypt_data(mHandle, &args);
    VerifyOrReturnError(err == ESP_OK || err == ESP_ERR_NOT_FINISHED, CHIP_ERROR_INTERNAL,
                        ChipLogError(SoftwareUpdate, "esp_encrypted_img_decrypt_data failed: %d", err));

    mDecryptedBuffer = args.data_out; // owned here; freed next call or in DecryptEnd/DecryptAbort
    out              = (args.data_out != nullptr && args.data_out_len > 0)
                     ? ByteSpan(reinterpret_cast<const uint8_t *>(args.data_out), args.data_out_len)
                     : ByteSpan();
    return CHIP_NO_ERROR;
}

CHIP_ERROR EncryptedOTAHelper::DecryptEnd()
{
    VerifyOrReturnError(mHandle != nullptr, CHIP_ERROR_INCORRECT_STATE);
    esp_err_t err = esp_encrypted_img_decrypt_end(mHandle);
    mHandle       = nullptr;
    FreeDecryptedBuffer();
    VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_INTERNAL,
                        ChipLogError(SoftwareUpdate, "esp_encrypted_img_decrypt_end failed: %d", err));
    return CHIP_NO_ERROR;
}

void EncryptedOTAHelper::DecryptAbort()
{
    if (mHandle != nullptr)
    {
        esp_encrypted_img_decrypt_abort(mHandle);
        mHandle = nullptr;
    }
    FreeDecryptedBuffer();
}

void EncryptedOTAHelper::FreeDecryptedBuffer()
{
    if (mDecryptedBuffer != nullptr)
    {
        free(mDecryptedBuffer);
        mDecryptedBuffer = nullptr;
    }
}

} // namespace chip
