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

#include "QRCodeUtil.h"

#include <platform/CHIPDeviceLayer.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

void PrintQRCode(chip::RendezvousInformationFlags rendezvousFlags)
{
    using namespace ::chip::DeviceLayer;

    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::SetupPayload payload;
    payload.version               = 1;
    payload.rendezvousInformation = rendezvousFlags;

    err = ConfigurationMgr().GetSetupPinCode(payload.setUpPINCode);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "ConfigurationMgr().GetSetupPinCode() failed: %s", chip::ErrorStr(err));
    }

    err = ConfigurationMgr().GetSetupDiscriminator(payload.discriminator);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "ConfigurationMgr().GetSetupDiscriminator() failed: %s", chip::ErrorStr(err));
    }

    err = ConfigurationMgr().GetVendorId(payload.vendorID);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "ConfigurationMgr().GetVendorId() failed: %s", chip::ErrorStr(err));
    }

    err = ConfigurationMgr().GetProductId(payload.productID);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "ConfigurationMgr().GetProductId() failed: %s", chip::ErrorStr(err));
    }

    // TODO: Usage of STL will significantly increase the image size, this should be changed to more efficient method for
    // generating payload
    std::string result;
    err = chip::QRCodeSetupPayloadGenerator(payload).payloadBase41Representation(result);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(AppServer, "Failed to generate QR Code"));

    ChipLogProgress(AppServer, "SetupPINCode: [%" PRIu32 "]", payload.setUpPINCode);
    // There might be whitespace in setup QRCode, add brackets to make it clearer.
    ChipLogProgress(AppServer, "SetupQRCode:  [%s]", result.c_str());

exit:
    return;
}
