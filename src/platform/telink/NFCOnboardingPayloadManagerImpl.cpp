/*
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/NFCOnboardingPayloadManager.h>

#include <lib/support/logging/CHIPLogging.h>

#include <zephyr/drivers/nfc/nfc_tag.h>
#include <zephyr/kernel.h>

void nfc_callback(const struct device * dev, enum nfc_tag_event event, const uint8_t * data, size_t data_len)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(data);
    ARG_UNUSED(data_len);
}

namespace chip {
namespace DeviceLayer {

NFCOnboardingPayloadManagerImpl NFCOnboardingPayloadManagerImpl::sInstance;

CHIP_ERROR NFCOnboardingPayloadManagerImpl::_Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR NFCOnboardingPayloadManagerImpl::_StartTagEmulation(const char * payload, size_t payloadLength)
{
    /* Set up NFC driver*/
    uint8_t ndef_msg_buf[NDEF_MSG_BUF_SIZE];
    uint32_t len = sizeof(ndef_msg_buf);

    int err;

    if (device_is_ready(dev))
    {
        ChipLogError(DeviceLayer, "NFC device is not ready for use");
        return CHIP_ERROR_INTERNAL;
    }

    err = device_init(dev);
    if (err != 0)
    {
        ChipLogError(DeviceLayer, "Failed to initialize NFC device: %d", err);
        return CHIP_ERROR_INTERNAL;
    }

    err = nfc_tag_init(dev, nfc_callback);
    if (err != 0)
    {
        ChipLogError(DeviceLayer, "Cannot setup NFC subsys!, err=%d", err);
        return CHIP_ERROR_INTERNAL;
    }

    /* Set up Tag mode */
    err = nfc_tag_set_type(dev, NFC_TAG_TYPE_T5T);
    if (err != 0)
    {
        ChipLogError(DeviceLayer, "Cannot setup NFC Tag mode!");
        return CHIP_ERROR_INTERNAL;
    }

    size_t encoded_len = _EncodeNDEFURI((const uint8_t *) payload, payloadLength, ndef_msg_buf);
    if (encoded_len == 0)
    {
        ChipLogError(DeviceLayer, "Cannot encode message!");
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogDetail(DeviceLayer, "NDEF URI of %d length encoded!", encoded_len);
    ChipLogDetail(DeviceLayer, "Payload: %s", payload);

    err = nfc_tag_set_ndef(dev, ndef_msg_buf, encoded_len);
    if (err != 0)
    {
        ChipLogError(DeviceLayer, "Cannot set payload! err=%d", err);
        return CHIP_ERROR_INTERNAL;
    }

    err = nfc_tag_start(dev);
    if (err != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    sInstance.mIsStarted = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR NFCOnboardingPayloadManagerImpl::_StopTagEmulation()
{
    if (nfc_tag_stop(dev))
    {
        return CHIP_ERROR_INTERNAL;
    }
    sInstance.mIsStarted = false;
    return CHIP_NO_ERROR;
}

size_t NFCOnboardingPayloadManagerImpl::_EncodeNDEFURI(const uint8_t * uri, size_t uri_len, uint8_t * out_buf)
{
    size_t max_len = 128;
    // NDEF Record Header for short record, TNF = Well Known, Type = 'U'
    const uint8_t type_field     = 'U';
    const uint8_t tnf            = 0xD1; // MB=1, ME=1, SR=1, TNF=1
    const uint8_t payload_prefix = 0x00; // No prefix, full URI follows

    if (max_len < 5 + uri_len)
    {
        ChipLogError(DeviceLayer, "NDEF message is too long, (5 + uri_len) required for TNF URI NDEF record");
        return 0;
    }

    size_t offset     = 0;
    out_buf[offset++] = tnf;
    out_buf[offset++] = 1;           // Type length
    out_buf[offset++] = uri_len + 1; // Payload length (1 prefix + uri)
    out_buf[offset++] = type_field;
    out_buf[offset++] = payload_prefix;
    memcpy(out_buf + offset, uri, uri_len);
    offset += uri_len;

    return offset;
}

} // namespace DeviceLayer
} // namespace chip
