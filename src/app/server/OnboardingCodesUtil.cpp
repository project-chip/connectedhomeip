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

#include <algorithm>
#include <inttypes.h>

#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <support/CodeUtils.h>
#include <support/ScopedBuffer.h>
#include <support/logging/CHIPLogging.h>

constexpr char kQrCodeBaseUrl[]                   = "https://dhrishi.github.io/connectedhomeip/qrcode.html";
constexpr char kUrlDataAssignmentPhrase[]         = "?data=";
constexpr char kSpecialCharsUnreservedInRfc3986[] = "-._~";

using namespace ::chip::DeviceLayer;

void PrintOnboardingCodes(chip::RendezvousInformationFlags aRendezvousFlags)
{
    std::string QRCode;
    std::string manualPairingCode;

    if (GetQRCode(QRCode, aRendezvousFlags) == CHIP_NO_ERROR)
    {
        chip::Platform::ScopedMemoryBuffer<char> qrCodeBuffer;
        const size_t qrCodeBufferMaxSize = strlen(kQrCodeBaseUrl) + strlen(kUrlDataAssignmentPhrase) + 3 * QRCode.size() + 1;
        qrCodeBuffer.Alloc(qrCodeBufferMaxSize);

        ChipLogProgress(AppServer, "SetupQRCode: [%s]", QRCode.c_str());
        if (GetQRCodeUrl(&qrCodeBuffer[0], qrCodeBufferMaxSize, QRCode) == CHIP_NO_ERROR)
        {
            ChipLogProgress(AppServer, "Copy/paste the below URL in a browser to see the QR Code:");
            ChipLogProgress(AppServer, "%s", &qrCodeBuffer[0]);
        }
    }
    else
    {
        ChipLogError(AppServer, "Getting QR code failed!");
    }

    if (GetManualPairingCode(manualPairingCode, aRendezvousFlags) == CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "Manual pairing code: [%s]", manualPairingCode.c_str());
    }
    else
    {
        ChipLogError(AppServer, "Getting manual pairing code failed!");
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_NFC
void ShareQRCodeOverNFC(chip::RendezvousInformationFlags aRendezvousFlags)
{
    // Get QR Code and emulate its content using NFC tag
    std::string QRCode;
    ReturnOnFailure(GetQRCode(QRCode, chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE)));

    ReturnOnFailure(NFCMgr().StartTagEmulation(QRCode.c_str(), QRCode.size()));
}
#endif

CHIP_ERROR GetSetupPayload(chip::SetupPayload & aSetupPayload, chip::RendezvousInformationFlags aRendezvousFlags)
{
    CHIP_ERROR err                      = CHIP_NO_ERROR;
    aSetupPayload.version               = 0;
    aSetupPayload.rendezvousInformation = aRendezvousFlags;

    err = ConfigurationMgr().GetSetupPinCode(aSetupPayload.setUpPINCode);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "ConfigurationMgr().GetSetupPinCode() failed: %s", chip::ErrorStr(err));
        return err;
    }

    err = ConfigurationMgr().GetSetupDiscriminator(aSetupPayload.discriminator);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "ConfigurationMgr().GetSetupDiscriminator() failed: %s", chip::ErrorStr(err));
        return err;
    }

    err = ConfigurationMgr().GetVendorId(aSetupPayload.vendorID);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "ConfigurationMgr().GetVendorId() failed: %s", chip::ErrorStr(err));
        return err;
    }

    err = ConfigurationMgr().GetProductId(aSetupPayload.productID);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "ConfigurationMgr().GetProductId() failed: %s", chip::ErrorStr(err));
        return err;
    }

    return err;
}

CHIP_ERROR GetQRCode(std::string & aQRCode, chip::RendezvousInformationFlags aRendezvousFlags)
{
    chip::SetupPayload payload;

    CHIP_ERROR err = GetSetupPayload(payload, aRendezvousFlags);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "GetSetupPayload() failed: %s", chip::ErrorStr(err));
        return err;
    }

    // TODO: Usage of STL will significantly increase the image size, this should be changed to more efficient method for
    // generating payload
    err = chip::QRCodeSetupPayloadGenerator(payload).payloadBase38Representation(aQRCode);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "Generating QR Code failed: %s", chip::ErrorStr(err));
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR GetQRCodeUrl(char * aQRCodeUrl, size_t aUrlMaxSize, const std::string & aQRCode)
{
    VerifyOrReturnError(aQRCodeUrl, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aUrlMaxSize >= (strlen(kQrCodeBaseUrl) + strlen(kUrlDataAssignmentPhrase) + aQRCode.size() + 1),
                        CHIP_ERROR_BUFFER_TOO_SMALL);

    const int writtenDataSize = snprintf(aQRCodeUrl, aUrlMaxSize, "%s%s", kQrCodeBaseUrl, kUrlDataAssignmentPhrase);
    VerifyOrReturnError((writtenDataSize > 0) && (static_cast<size_t>(writtenDataSize) < aUrlMaxSize),
                        CHIP_ERROR_INVALID_STRING_LENGTH);

    return EncodeQRCodeToUrl(aQRCode.c_str(), aQRCode.size(), aQRCodeUrl + writtenDataSize,
                             aUrlMaxSize - static_cast<size_t>(writtenDataSize));
}

CHIP_ERROR GetManualPairingCode(std::string & aManualPairingCode, chip::RendezvousInformationFlags aRendezvousFlags)
{
    chip::SetupPayload payload;

    CHIP_ERROR err = GetSetupPayload(payload, aRendezvousFlags);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "GetSetupPayload() failed: %s", chip::ErrorStr(err));
        return err;
    }

    err = chip::ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(aManualPairingCode);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "Generating Manual Pairing Code failed: %s", chip::ErrorStr(err));
        return err;
    }

    return CHIP_NO_ERROR;
}

static inline bool isCharUnreservedInRfc3986(const char c)
{
    return isalpha(c) || isdigit(c) || (strchr(kSpecialCharsUnreservedInRfc3986, c) != nullptr);
}

CHIP_ERROR EncodeQRCodeToUrl(const char * aQRCode, size_t aLen, char * aUrl, size_t aMaxSize)
{
    const char upperHexDigits[] = "0123456789ABCDEF";
    size_t i = 0, j = 0;

    VerifyOrReturnError((aQRCode != nullptr) && (aUrl != nullptr), CHIP_ERROR_INVALID_ARGUMENT);

    for (i = 0; i < aLen; ++i)
    {
        char c = aQRCode[i];
        if (isCharUnreservedInRfc3986(c))
        {

            VerifyOrReturnError((j + 1) < aMaxSize, CHIP_ERROR_BUFFER_TOO_SMALL);

            aUrl[j++] = c;
        }
        else
        {

            VerifyOrReturnError((j + 3) < aMaxSize, CHIP_ERROR_BUFFER_TOO_SMALL);

            aUrl[j++] = '%';
            aUrl[j++] = upperHexDigits[(c & 0xf0) >> 4];
            aUrl[j++] = upperHexDigits[(c & 0x0f)];
        }
    }

    aUrl[j] = '\0';

    return CHIP_NO_ERROR;
}
