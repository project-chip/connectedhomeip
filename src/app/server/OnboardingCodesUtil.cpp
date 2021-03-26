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

#include <app/server/OnboardingCodesUtil.h>

#include <inttypes.h>

#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <support/CodeUtils.h>
#include <support/ScopedBuffer.h>
#include <support/logging/CHIPLogging.h>

constexpr char qrCodeBaseUrl[]                   = "https://dhrishi.github.io/connectedhomeip/qrcode.html";
constexpr char specialCharsUnreservedInRfc3986[] = "-._~";

using namespace ::chip::DeviceLayer;

void PrintOnboardingCodes(chip::RendezvousInformationFlags rendezvousFlags)
{
    std::string QRCode;
    std::string manualPairingCode;

    if (GetQRCode(QRCode, rendezvousFlags) == CHIP_NO_ERROR)
    {
        chip::Platform::ScopedMemoryBuffer<char> qrCodeBuffer;
        const size_t qrCodeBufferMaxSize = 3 * QRCode.size() + 1;
        qrCodeBuffer.Alloc(qrCodeBufferMaxSize);

        ChipLogProgress(AppServer, "SetupQRCode: [%s]", QRCode.c_str());
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

    if (GetManualPairingCode(manualPairingCode, rendezvousFlags) == CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "Manual pairing code: [%s]", manualPairingCode.c_str());
    }
    else
    {
        ChipLogError(AppServer, "Getting manual pairing code failed!");
    }
}

static CHIP_ERROR GetSetupPayload(chip::SetupPayload & setupPayload, chip::RendezvousInformationFlags rendezvousFlags)
{
    CHIP_ERROR err                     = CHIP_NO_ERROR;
    setupPayload.version               = 0;
    setupPayload.rendezvousInformation = rendezvousFlags;

    err = ConfigurationMgr().GetSetupPinCode(setupPayload.setUpPINCode);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogProgress(AppServer, "ConfigurationMgr().GetSetupPinCode() failed: %s", chip::ErrorStr(err)));

    err = ConfigurationMgr().GetSetupDiscriminator(setupPayload.discriminator);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogProgress(AppServer, "ConfigurationMgr().GetSetupDiscriminator() failed: %s", chip::ErrorStr(err)));

    err = ConfigurationMgr().GetVendorId(setupPayload.vendorID);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogProgress(AppServer, "ConfigurationMgr().GetVendorId() failed: %s", chip::ErrorStr(err)));

    err = ConfigurationMgr().GetProductId(setupPayload.productID);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogProgress(AppServer, "ConfigurationMgr().GetProductId() failed: %s", chip::ErrorStr(err)));

exit:
    return err;
}

CHIP_ERROR GetQRCode(std::string & QRCode, chip::RendezvousInformationFlags rendezvousFlags)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::SetupPayload payload;

    err = GetSetupPayload(payload, rendezvousFlags);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogProgress(AppServer, "GetSetupPayload() failed: %s", chip::ErrorStr(err)));

    // TODO: Usage of STL will significantly increase the image size, this should be changed to more efficient method for
    // generating payload
    err = chip::QRCodeSetupPayloadGenerator(payload).payloadBase41Representation(QRCode);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogProgress(AppServer, "Generating QR Code failed: %s", chip::ErrorStr(err)));

exit:
    return err;
}

CHIP_ERROR GetManualPairingCode(std::string & manualPairingCode, chip::RendezvousInformationFlags rendezvousFlags)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::SetupPayload payload;

    err = GetSetupPayload(payload, rendezvousFlags);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogProgress(AppServer, "GetSetupPayload() failed: %s", chip::ErrorStr(err)));

    err = chip::ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(manualPairingCode);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogProgress(AppServer, "Generating Manual Pairing Code failed: %s", chip::ErrorStr(err)));

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
