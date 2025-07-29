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

#include <zephyr/kernel.h>
#include <zephyr/nfc/ndef/uri_msg.h>
#include <zephyr/nfc/nfc_tag.h>

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

    err = nfc_tag_init(dev, nfc_callback);
    if (err != 0)
    {
        ChipLogError(DeviceLayer, "Cannot setup NFC subsys!");
        return CHIP_ERROR_INTERNAL;
    }

    /* Set up Tag mode */
    err = nfc_tag_set_type(dev, NFC_TAG_TYPE_T5T);
    if (err != 0)
    {
        ChipLogError(DeviceLayer, "Cannot setup NFC Tag mode!");
        return CHIP_ERROR_INTERNAL;
    }

    err = nfc_ndef_uri_msg_encode(NFC_URI_NONE, (const uint8_t *) payload, payloadLength, ndef_msg_buf, &len);
    if (err != 0)
    {
        ChipLogError(DeviceLayer, "Cannot encode message!");
        return CHIP_ERROR_INTERNAL;
    }

    err = nfc_tag_set_ndef(dev, ndef_msg_buf, len);
    if (err != 0)
    {
        ChipLogError(DeviceLayer, "Cannot set payload!");
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

} // namespace DeviceLayer
} // namespace chip
