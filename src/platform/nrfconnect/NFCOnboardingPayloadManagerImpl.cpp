/*
 *
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/NFCOnboardingPayloadManager.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

#include <nfc/ndef/uri_msg.h>
#include <nfc/ndef/uri_rec.h>
#include <nfc_t2t_lib.h>
#include <zephyr/kernel.h>

namespace chip {
namespace DeviceLayer {
namespace {
void nfcCallback(void * /* context */, nfc_t2t_event_t, const uint8_t * /* data */, size_t /* data_length */) {}
} // namespace

NFCOnboardingPayloadManagerImpl NFCOnboardingPayloadManagerImpl::sInstance;

CHIP_ERROR NFCOnboardingPayloadManagerImpl::_Init()
{
    mIsStarted = false;
    return nfc_t2t_setup(nfcCallback, nullptr) ? CHIP_ERROR_INTERNAL : CHIP_NO_ERROR;
}

CHIP_ERROR NFCOnboardingPayloadManagerImpl::_StartTagEmulation(const char * payload, size_t payloadLength)
{
    CHIP_ERROR error    = CHIP_NO_ERROR;
    int result          = 0;
    uint32_t ndefLength = sizeof(mNdefBuffer);

    VerifyOrExit(!mIsStarted, error = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(CanCastTo<uint8_t>(payloadLength), error = CHIP_ERROR_BUFFER_TOO_SMALL);

    result = nfc_ndef_uri_msg_encode(NFC_URI_NONE, reinterpret_cast<const uint8_t *>(payload), static_cast<uint8_t>(payloadLength),
                                     mNdefBuffer, &ndefLength);

    if (result)
    {
        ChipLogError(DeviceLayer, "nfc_ndef_uri_msg_encode() failed: %d", result);
        ExitNow(error = CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    result = nfc_t2t_payload_set(mNdefBuffer, ndefLength);

    if (result)
    {
        ChipLogError(DeviceLayer, "nfc_t2t_payload_set() failed: %d", result);
        ExitNow(error = CHIP_ERROR_INTERNAL);
    }

    result = nfc_t2t_emulation_start();

    if (result)
    {
        ChipLogError(DeviceLayer, "nfc_t2t_emulation_start() failed: %d", result);
        ExitNow(error = CHIP_ERROR_INTERNAL);
    }

    ChipLogProgress(DeviceLayer, "NFC Tag emulation started");
    mIsStarted = true;

exit:
    if (error != CHIP_NO_ERROR)
        ChipLogError(DeviceLayer, "Starting NFC Tag emulation failed: %" CHIP_ERROR_FORMAT, error.Format());
    return error;
}

CHIP_ERROR NFCOnboardingPayloadManagerImpl::_StopTagEmulation()
{
    VerifyOrReturnError(mIsStarted, CHIP_NO_ERROR);

    int result = nfc_t2t_emulation_stop();

    if (result)
    {
        ChipLogError(DeviceLayer, "Stopping NFC Tag emulation failed: %d", result);
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogProgress(DeviceLayer, "NFC Tag emulation stopped");
    mIsStarted = false;
    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
