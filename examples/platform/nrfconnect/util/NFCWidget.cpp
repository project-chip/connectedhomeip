/*
 *
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
#include "NFCWidget.h"

#include <nfc/ndef/uri_msg.h>
#include <nfc/ndef/uri_rec.h>
#include <nfc_t2t_lib.h>
#include <zephyr.h>

int NFCWidget::Init(chip::DeviceLayer::ConnectivityManager & mgr)
{
    mIsTagStarted = false;
    return nfc_t2t_setup(FieldDetectionHandler, &mgr);
}

int NFCWidget::StartTagEmulation(const char * tagPayload, uint8_t tagPayloadLength)
{
    uint32_t len = sizeof(mNdefBuffer);
    int result   = 0;

    result =
        nfc_ndef_uri_msg_encode(NFC_URI_NONE, reinterpret_cast<const uint8_t *>(tagPayload), tagPayloadLength, mNdefBuffer, &len);
    VerifyOrExit(result >= 0, ChipLogProgress(AppServer, "nfc_ndef_uri_msg_encode failed: %d", result));

    result = nfc_t2t_payload_set(mNdefBuffer, len);
    VerifyOrExit(result >= 0, ChipLogProgress(AppServer, "nfc_t2t_payload_set failed: %d", result));

    result = nfc_t2t_emulation_start();
    VerifyOrExit(result >= 0, ChipLogProgress(AppServer, "nfc_t2t_emulation_start failed: %d", result));

    mIsTagStarted = true;

exit:
    return result;
}

int NFCWidget::StopTagEmulation()
{
    int result = nfc_t2t_emulation_stop();

    VerifyOrExit(result >= 0, ChipLogProgress(AppServer, "nfc_t2t_emulation_stop failed: %d", result));

    memset(mNdefBuffer, 0, sizeof(mNdefBuffer));

    mIsTagStarted = false;

exit:
    return result;
}

bool NFCWidget::IsTagEmulationStarted() const
{
    return mIsTagStarted;
}

void NFCWidget::FieldDetectionHandler(void * context, enum nfc_t2t_event event, const uint8_t * data, size_t data_length)
{
    ARG_UNUSED(context);
    ARG_UNUSED(event);
    ARG_UNUSED(data);
    ARG_UNUSED(data_length);
}
