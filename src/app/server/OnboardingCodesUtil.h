/*
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

#pragma once

#include <lib/support/Span.h>
#include <setup_payload/SetupPayload.h>

void PrintOnboardingCodes(chip::RendezvousInformationFlags aRendezvousFlags);
void PrintOnboardingCodes(const chip::PayloadContents & payload);
void PrintQrCodeURL(const chip::MutableCharSpan qrCode);
void ShareQRCodeOverNFC(chip::RendezvousInformationFlags aRendezvousFlags);

/**
 * Creates a null-terminated QR code from the payload created based on rendezvous flag information.
 *
 * The resulting size of the QR code span will be the size of data written and not including the null terminator.
 */
CHIP_ERROR GetQRCode(chip::MutableCharSpan & aQRCode, chip::RendezvousInformationFlags aRendezvousFlags);

/**
 * Creates a null-terminated QR code based on the provided payload.
 *
 * The resulting size of the QR code span will be the size of data written and not including the null terminator.
 */
CHIP_ERROR GetQRCode(chip::MutableCharSpan & aQRCode, const chip::PayloadContents & payload);

/**
 * Creates a null-terminated QR code url.
 */
CHIP_ERROR GetQRCodeUrl(char * aQRCodeUrl, size_t aUrlMaxSize, const chip::CharSpan & aQRCode);

/**
 * Creates a null-terminated manual pairing code from the payload created based on rendezvous flag information.
 *
 * The resulting size of the manual pairing code span will be the size of data written and not including the null terminator.
 */
CHIP_ERROR GetManualPairingCode(chip::MutableCharSpan & aManualPairingCode, chip::RendezvousInformationFlags aRendezvousFlags);

/**
 * Creates a null-terminated manual pairing code based on the provided payload.
 *
 * The resulting size of the manual pairing code span will be the size of data written and not including the null terminator.
 */
CHIP_ERROR GetManualPairingCode(chip::MutableCharSpan & aManualPairingCode, const chip::PayloadContents & payload);

CHIP_ERROR GetPayloadContents(chip::PayloadContents & aPayload, chip::RendezvousInformationFlags aRendezvousFlags);

/**
 * Initialize DataModelHandler and start CHIP datamodel server, the server
 * assumes the platform's networking has been setup already.
 *
 * Method verifies if every character of the QR Code is valid for the url encoding
 * and otherwise it encodes the invalid character using available ones.
 *
 * @param aQRCode address of the array storing QR Code to encode.
 * @param aLen length of the given QR Code.
 * @param aUrl address of the location where encoded url should be stored.
 * @param aMaxSize maximal size of the array where encoded url should be stored.
 * @return CHIP_NO_ERROR on success and other values on error.
 */
CHIP_ERROR EncodeQRCodeToUrl(const char * aQRCode, size_t aLen, char * aUrl, size_t aMaxSize);
