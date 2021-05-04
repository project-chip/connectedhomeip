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
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogProgress(AppServer, "ConfigurationMgr().GetSetupPinCode() failed: %s", chip::ErrorStr(err)));

    err = ConfigurationMgr().GetSetupDiscriminator(aSetupPayload.discriminator);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogProgress(AppServer, "ConfigurationMgr().GetSetupDiscriminator() failed: %s", chip::ErrorStr(err)));

    err = ConfigurationMgr().GetVendorId(aSetupPayload.vendorID);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogProgress(AppServer, "ConfigurationMgr().GetVendorId() failed: %s", chip::ErrorStr(err)));

    err = ConfigurationMgr().GetProductId(aSetupPayload.productID);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogProgress(AppServer, "ConfigurationMgr().GetProductId() failed: %s", chip::ErrorStr(err)));

exit:
    return err;
}

CHIP_ERROR GetQRCode(std::string & aQRCode, chip::RendezvousInformationFlags aRendezvousFlags)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::SetupPayload payload;

    err = GetSetupPayload(payload, aRendezvousFlags);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogProgress(AppServer, "GetSetupPayload() failed: %s", chip::ErrorStr(err)));

    // TODO: Usage of STL will significantly increase the image size, this should be changed to more efficient method for
    // generating payload
    err = chip::QRCodeSetupPayloadGenerator(payload).payloadBase38Representation(aQRCode);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogProgress(AppServer, "Generating QR Code failed: %s", chip::ErrorStr(err)));

exit:
    return err;
}

CHIP_ERROR GetQRCodeUrl(char * aQRCodeUrl, size_t aUrlMaxSize, const std::string & aQRCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int writtenDataSize;

    VerifyOrExit(aQRCodeUrl, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(aUrlMaxSize >= (strlen(kQrCodeBaseUrl) + strlen(kUrlDataAssignmentPhrase) + aQRCode.size() + 1),
                 err = CHIP_ERROR_BUFFER_TOO_SMALL);

    writtenDataSize = snprintf(aQRCodeUrl, aUrlMaxSize, "%s%s", kQrCodeBaseUrl, kUrlDataAssignmentPhrase);
    VerifyOrExit((writtenDataSize > 0) && (writtenDataSize < static_cast<int>(aUrlMaxSize)),
                 err = CHIP_ERROR_INVALID_STRING_LENGTH);

    err = EncodeQRCodeToUrl(aQRCode.c_str(), aQRCode.size(), aQRCodeUrl + writtenDataSize, aUrlMaxSize - writtenDataSize);
    VerifyOrExit(err == CHIP_NO_ERROR, );

exit:
    return err;
}

CHIP_ERROR GetManualPairingCode(std::string & aManualPairingCode, chip::RendezvousInformationFlags aRendezvousFlags)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::SetupPayload payload;

    err = GetSetupPayload(payload, aRendezvousFlags);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogProgress(AppServer, "GetSetupPayload() failed: %s", chip::ErrorStr(err)));

    err = chip::ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(aManualPairingCode);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogProgress(AppServer, "Generating Manual Pairing Code failed: %s", chip::ErrorStr(err)));

exit:
    return err;
}

static inline bool isCharUnreservedInRfc3986(const char c)
{
    return isalpha(c) || isdigit(c) || (strchr(kSpecialCharsUnreservedInRfc3986, c) != nullptr);
}

CHIP_ERROR EncodeQRCodeToUrl(const char * aQRCode, size_t aLen, char * aUrl, size_t aMaxSize)
{
    const char upperHexDigits[] = "0123456789ABCDEF";
    CHIP_ERROR err              = CHIP_NO_ERROR;
    size_t i = 0, j = 0;

    VerifyOrExit((aQRCode != nullptr) && (aUrl != nullptr), err = CHIP_ERROR_INVALID_ARGUMENT);

    for (i = 0; i < aLen; ++i)
    {
        unsigned char c = aQRCode[i];
        if (isCharUnreservedInRfc3986(c))
        {

            VerifyOrExit((j + 1) < aMaxSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);

            aUrl[j++] = c;
        }
        else
        {

            VerifyOrExit((j + 3) < aMaxSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);

            aUrl[j++] = '%';
            aUrl[j++] = upperHexDigits[(c & 0xf0) >> 4];
            aUrl[j++] = upperHexDigits[(c & 0x0f)];
        }
    }

    aUrl[j] = '\0';

exit:
    return err;
}
