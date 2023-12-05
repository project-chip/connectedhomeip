/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

package matter.onboardingpayload

/** Parser for scanned QR code or Manual Pairing Code. */
class OnboardingPayloadParser {
  /**
   * Returns [OnboardingPayload] parsed from the QR code string. If an invalid element is included
   * in the QRCode Parse result, OnboardingPayloadException occurs. Refer to [OnboardingPayload] for
   * the description of the invalid element.
   */
  @Throws(UnrecognizedQrCodeException::class, OnboardingPayloadException::class)
  fun parseQrCode(qrCodeString: String): OnboardingPayload {
    return fetchPayloadFromQrCode(qrCodeString, false)
  }

  /**
   * Returns [OnboardingPayload] parsed from the QR code string.
   *
   * @param qrCodeString the QRCode for commissioning device.
   * @param skipPayloadValidation If this value is true, payload element validation is not checked.
   *   Consider saying that the payload must still parse correctly, but this skips validation of the
   *   content past parsing (i.e. it does not validate ranges for individual elements). Refer to
   *   [OnboardingPayload] for the description of the invalid element.
   */
  @Throws(UnrecognizedQrCodeException::class, OnboardingPayloadException::class)
  fun parseQrCode(qrCodeString: String, skipPayloadValidation: Boolean): OnboardingPayload {
    return fetchPayloadFromQrCode(qrCodeString, skipPayloadValidation)
  }

  /** Get QR code string from [OnboardingPayload]. */
  @Throws(OnboardingPayloadException::class)
  fun getQrCodeFromPayload(payload: OnboardingPayload): String {
    return QRCodeOnboardingPayloadGenerator(payload).payloadBase38Representation()
  }

  @Throws(UnrecognizedQrCodeException::class, OnboardingPayloadException::class)
  private fun fetchPayloadFromQrCode(
    qrCodeString: String,
    skipPayloadValidation: Boolean
  ): OnboardingPayload {
    val payload = QRCodeOnboardingPayloadParser(qrCodeString).populatePayload()

    if (skipPayloadValidation == false && !payload.isValidQRCodePayload()) {
      throw OnboardingPayloadException("Invalid payload")
    }

    return payload
  }

  /** Get Manual Pairing Code string from [OnboardingPayload]. */
  @Throws(OnboardingPayloadException::class)
  fun getManualPairingCodeFromPayload(payload: OnboardingPayload): String {
    return ManualOnboardingPayloadGenerator(payload).payloadDecimalStringRepresentation()
  }

  /**
   * Returns [OnboardingPayload] parsed from the Manual Pairing Code string. If an SetupPINCode has
   * invalid value, OnboardingPayloadException occurs. Refer to [OnboardingPayload] for the
   * description of the invalid element.
   */
  @Throws(InvalidManualPairingCodeFormatException::class, OnboardingPayloadException::class)
  fun parseManualPairingCode(manualPairingCodeString: String): OnboardingPayload {
    return parsePayloadFromManualPairingCode(manualPairingCodeString, false)
  }

  /**
   * Returns [OnboardingPayload] parsed from the Manual Pairing Code string.
   *
   * @param manualPairingCodeString the manual Pairing Code for commissioning device.
   * @param skipPayloadValidation If this value is true, payload element validation is not checked.
   *   Consider saying that the payload must still parse correctly, but this skips validation of the
   *   content past parsing (i.e. it does not validate ranges for individual elements). Refer to
   *   [OnboardingPayload] for the description of the invalid element.
   */
  @Throws(InvalidManualPairingCodeFormatException::class, OnboardingPayloadException::class)
  fun parseManualPairingCode(
    manualPairingCodeString: String,
    skipPayloadValidation: Boolean
  ): OnboardingPayload {
    return parsePayloadFromManualPairingCode(manualPairingCodeString, skipPayloadValidation)
  }

  @Throws(InvalidManualPairingCodeFormatException::class, OnboardingPayloadException::class)
  private fun parsePayloadFromManualPairingCode(
    manualPairingCodeString: String,
    skipPayloadValidation: Boolean
  ): OnboardingPayload {
    val payload = ManualOnboardingPayloadParser(manualPairingCodeString).populatePayload()

    if (skipPayloadValidation == false && !payload.isValidManualCode()) {
      throw OnboardingPayloadException("Invalid manual entry code")
    }

    return payload
  }
}
