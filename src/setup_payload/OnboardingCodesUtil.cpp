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

#include <setup_payload/OnboardingCodesUtil.h>

#include <algorithm>
#include <inttypes.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

constexpr char kQrCodeBaseUrl[]                   = "https://project-chip.github.io/connectedhomeip/qrcode.html";
constexpr char kUrlDataAssignmentPhrase[]         = "?data=";
constexpr char kSpecialCharsUnreservedInRfc3986[] = "-._~";

using namespace ::chip::DeviceLayer;

void PrintOnboardingCodes(chip::RendezvousInformationFlags aRendezvousFlags)
{
    chip::PayloadContents payload;

    CHIP_ERROR err = GetPayloadContents(payload, aRendezvousFlags);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "GetPayloadContents() failed: %" CHIP_ERROR_FORMAT, err.Format());
    }

    PrintOnboardingCodes(payload);
}

void PrintOnboardingCodes(const chip::PayloadContents & payload)
{
    char payloadBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan qrCode(payloadBuffer);

    if (GetQRCode(qrCode, payload) == CHIP_NO_ERROR)
    {
        PrintQrCodeURL(qrCode);
    }
    else
    {
        ChipLogError(AppServer, "Getting QR code failed!");
    }

    chip::MutableCharSpan manualPairingCode(payloadBuffer);
    if (GetManualPairingCode(manualPairingCode, payload) == CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "Manual pairing code: [%s]", manualPairingCode.data());
    }
    else
    {
        ChipLogError(AppServer, "Getting manual pairing code failed!");
    }
}

void PrintQrCodeURL(const chip::MutableCharSpan qrCode)
{
    chip::Platform::ScopedMemoryBuffer<char> qrCodeBuffer;
    const size_t qrCodeBufferMaxSize = strlen(kQrCodeBaseUrl) + strlen(kUrlDataAssignmentPhrase) + 3 * qrCode.size() + 1;
    qrCodeBuffer.Alloc(qrCodeBufferMaxSize);

    ChipLogProgress(AppServer, "SetupQRCode: [%s]", qrCode.data());
    if (GetQRCodeUrl(qrCodeBuffer.Get(), qrCodeBufferMaxSize, qrCode) == CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "Copy/paste the below URL in a browser to see the QR Code:");
        ChipLogProgress(AppServer, "%s", qrCodeBuffer.Get());
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_NFC_ONBOARDING_PAYLOAD
void ShareQRCodeOverNFC(chip::RendezvousInformationFlags aRendezvousFlags)
{
    // Get QR Code and emulate its content using NFC tag
    char payloadBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan qrCode(payloadBuffer);

    ReturnOnFailure(GetQRCode(qrCode, chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE)));

    ReturnOnFailure(NFCOnboardingPayloadMgr().StartTagEmulation(qrCode.data(), qrCode.size()));
}
#endif

CHIP_ERROR GetPayloadContents(chip::PayloadContents & aPayload, chip::RendezvousInformationFlags aRendezvousFlags)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    aPayload.version = 0;
    aPayload.rendezvousInformation.SetValue(aRendezvousFlags);

    err = GetCommissionableDataProvider()->GetSetupPasscode(aPayload.setUpPINCode);
    if (err != CHIP_NO_ERROR)
    {
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
        ChipLogProgress(AppServer, "*** Using default EXAMPLE passcode %u ***",
                        static_cast<unsigned>(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE));
        aPayload.setUpPINCode = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE;
#else
        ChipLogError(AppServer, "GetCommissionableDataProvider()->GetSetupPasscode() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
#endif
    }

    uint16_t discriminator = 0;
    err                    = GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "GetCommissionableDataProvider()->GetSetupDiscriminator() failed: %" CHIP_ERROR_FORMAT,
                     err.Format());
        return err;
    }
    aPayload.discriminator.SetLongValue(discriminator);

    err = chip::DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(aPayload.vendorID);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "GetDeviceInstanceInfoProvider()->GetVendorId() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    err = chip::DeviceLayer::GetDeviceInstanceInfoProvider()->GetProductId(aPayload.productID);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "GetDeviceInstanceInfoProvider()->GetProductId() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    return err;
}

CHIP_ERROR GetQRCode(chip::MutableCharSpan & aQRCode, chip::RendezvousInformationFlags aRendezvousFlags)
{
    chip::PayloadContents payload;

    CHIP_ERROR err = GetPayloadContents(payload, aRendezvousFlags);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "GetPayloadContents() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    return GetQRCode(aQRCode, payload);
}

CHIP_ERROR GetQRCode(chip::MutableCharSpan & aQRCode, const chip::PayloadContents & payload)
{
    CHIP_ERROR err = chip::QRCodeBasicSetupPayloadGenerator(payload).payloadBase38Representation(aQRCode);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Generating QR Code failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR GetQRCodeUrl(char * aQRCodeUrl, size_t aUrlMaxSize, const chip::CharSpan & aQRCode)
{
    VerifyOrReturnError(aQRCodeUrl, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aUrlMaxSize >= (strlen(kQrCodeBaseUrl) + strlen(kUrlDataAssignmentPhrase) + aQRCode.size() + 1),
                        CHIP_ERROR_BUFFER_TOO_SMALL);

    const int writtenDataSize = snprintf(aQRCodeUrl, aUrlMaxSize, "%s%s", kQrCodeBaseUrl, kUrlDataAssignmentPhrase);
    VerifyOrReturnError((writtenDataSize > 0) && (static_cast<size_t>(writtenDataSize) < aUrlMaxSize),
                        CHIP_ERROR_INVALID_STRING_LENGTH);

    return EncodeQRCodeToUrl(aQRCode.data(), aQRCode.size(), aQRCodeUrl + writtenDataSize,
                             aUrlMaxSize - static_cast<size_t>(writtenDataSize));
}

CHIP_ERROR GetManualPairingCode(chip::MutableCharSpan & aManualPairingCode, chip::RendezvousInformationFlags aRendezvousFlags)
{
    chip::PayloadContents payload;

    CHIP_ERROR err = GetPayloadContents(payload, aRendezvousFlags);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "GetPayloadContents() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    return GetManualPairingCode(aManualPairingCode, payload);
}

CHIP_ERROR GetManualPairingCode(chip::MutableCharSpan & aManualPairingCode, const chip::PayloadContents & payload)
{
    CHIP_ERROR err = chip::ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(aManualPairingCode);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Generating Manual Pairing Code failed: %" CHIP_ERROR_FORMAT, err.Format());
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
    static const char upperHexDigits[] = "0123456789ABCDEF";
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
