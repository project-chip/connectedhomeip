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

#include <app/server/QRCodeUtil.h>

#include <inttypes.h>

#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <support/CodeUtils.h>
#include <support/ScopedBuffer.h>
#include <support/logging/CHIPLogging.h>

constexpr char qrCodeBaseUrl[]                   = "https://dhrishi.github.io/connectedhomeip/qrcode.html";
constexpr char specialCharsUnreservedInRfc3986[] = "-._~";

void PrintQRCode(chip::RendezvousInformationFlags rendezvousFlags)
{
    uint32_t setupPinCode;
    std::string QRCode;

    if (GetQRCode(setupPinCode, QRCode, rendezvousFlags) == CHIP_NO_ERROR)
    {
        chip::Platform::ScopedMemoryBuffer<char> qrCodeBuffer;
        const size_t qrCodeBufferMaxSize = 3 * QRCode.size() + 1;
        qrCodeBuffer.Alloc(qrCodeBufferMaxSize);

        ChipLogProgress(AppServer, "SetupPINCode: [%" PRIu32 "]", setupPinCode);
        ChipLogProgress(AppServer, "SetupQRCode:  [%s]", QRCode.c_str());
        if (EncodeQRCodeToUrl(QRCode.c_str(), QRCode.size(), &qrCodeBuffer[0], qrCodeBufferMaxSize) == CHIP_NO_ERROR)
        {
            ChipLogProgress(AppServer, "Copy/paste the below URL in a browser to see the QR Code:");
            ChipLogProgress(AppServer, "%s?data=%s", qrCodeBaseUrl, &qrCodeBuffer[0]);
        }
    }
    else
    {
        ChipLogError(AppServer, "Getting QR code failed!");
    }
}

CHIP_ERROR GetQRCode(uint32_t & setupPinCode, std::string & QRCode, chip::RendezvousInformationFlags rendezvousFlags)
{
    using namespace ::chip::DeviceLayer;

    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::SetupPayload payload;

    payload.version               = 1;
    payload.rendezvousInformation = rendezvousFlags;

    err = ConfigurationMgr().GetSetupPinCode(payload.setUpPINCode);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogProgress(AppServer, "ConfigurationMgr().GetSetupPinCode() failed: %s", chip::ErrorStr(err)));
    setupPinCode = payload.setUpPINCode;

    err = ConfigurationMgr().GetSetupDiscriminator(payload.discriminator);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogProgress(AppServer, "ConfigurationMgr().GetSetupDiscriminator() failed: %s", chip::ErrorStr(err)));

    err = ConfigurationMgr().GetVendorId(payload.vendorID);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogProgress(AppServer, "ConfigurationMgr().GetVendorId() failed: %s", chip::ErrorStr(err)));

    err = ConfigurationMgr().GetProductId(payload.productID);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogProgress(AppServer, "ConfigurationMgr().GetProductId() failed: %s", chip::ErrorStr(err)));

    // TODO: Usage of STL will significantly increase the image size, this should be changed to more efficient method for
    // generating payload
    err = chip::QRCodeSetupPayloadGenerator(payload).payloadBase41Representation(QRCode);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogProgress(AppServer, "Generating QR Code failed: %s", chip::ErrorStr(err)));

exit:
    return err;
}

static inline bool isCharUnreservedInRfc3986(const char c)
{
    return isalpha(c) || isdigit(c) || (strchr(specialCharsUnreservedInRfc3986, c) != nullptr);
}

CHIP_ERROR EncodeQRCodeToUrl(const char * QRCode, size_t len, char * url, size_t maxSize)
{
    const char upperHexDigits[] = "0123456789ABCDEF";
    CHIP_ERROR err              = CHIP_NO_ERROR;
    size_t i = 0, j = 0;

    VerifyOrExit((QRCode != nullptr) && (url != nullptr), err = CHIP_ERROR_INVALID_ARGUMENT);

    for (i = 0; i < len; ++i)
    {
        unsigned char c = QRCode[i];
        if (isCharUnreservedInRfc3986(c))
        {

            VerifyOrExit((j + 1) < maxSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);

            url[j++] = c;
        }
        else
        {

            VerifyOrExit((j + 3) < maxSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);

            url[j++] = '%';
            url[j++] = upperHexDigits[(c & 0xf0) >> 4];
            url[j++] = upperHexDigits[(c & 0x0f)];
        }
    }

    url[j] = '\0';

exit:
    return err;
}
