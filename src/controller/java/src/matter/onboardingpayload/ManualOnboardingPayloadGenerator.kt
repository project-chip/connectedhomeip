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

class ManualOnboardingPayloadGenerator(private val payloadContents: OnboardingPayload) {
  private var skipPayloadValidation = false
  private var forceShortCode = false

  fun setSkipPayloadValidation(allow: Boolean) {
    skipPayloadValidation = allow
  }

  fun setForceShortCode(useShort: Boolean) {
    forceShortCode = useShort
  }

  fun payloadDecimalStringRepresentation(): String {
    // One extra char for the check digit.
    val decimalString = CharArray(kManualSetupLongCodeCharLength + 1)

    if (
      kManualSetupCodeChunk1CharLength +
        kManualSetupCodeChunk2CharLength +
        kManualSetupCodeChunk3CharLength != kManualSetupShortCodeCharLength
    ) {
      throw OnboardingPayloadException("Manual code length mismatch (short)")
    }

    if (
      kManualSetupShortCodeCharLength +
        kManualSetupVendorIdCharLength +
        kManualSetupProductIdCharLength != kManualSetupLongCodeCharLength
    ) {
      throw OnboardingPayloadException("Manual code length mismatch (long)")
    }

    if (
      kManualSetupChunk1DiscriminatorMsbitsLength + kManualSetupChunk2DiscriminatorLsbitsLength !=
        kManualSetupDiscriminatorFieldLengthInBits
    ) {
      throw OnboardingPayloadException("Discriminator length is not valid")
    }

    if (
      kManualSetupChunk2PINCodeLsbitsLength + kManualSetupChunk3PINCodeMsbitsLength !=
        kSetupPINCodeFieldLengthInBits
    ) {
      throw OnboardingPayloadException("PIN code length is not valid")
    }

    val useLongCode =
      (payloadContents.commissioningFlow != CommissioningFlow.STANDARD.value) && !forceShortCode

    if (!skipPayloadValidation && !payloadContents.isValidManualCode()) {
      throw OnboardingPayloadException("The Manual Pairing code is not valid")
    }

    // Add two for the check digit and null terminator.
    if (
      (useLongCode && decimalString.size < kManualSetupLongCodeCharLength + 1) ||
        (!useLongCode && decimalString.size < kManualSetupShortCodeCharLength + 1)
    ) {
      throw OnboardingPayloadException("The decimalString has insufficient size")
    }

    val chunk1 = chunk1PayloadRepresentation(payloadContents)
    val chunk2 = chunk2PayloadRepresentation(payloadContents)
    val chunk3 = chunk3PayloadRepresentation(payloadContents)

    var offset = 0

    decimalStringWithPadding(decimalString, offset, kManualSetupCodeChunk1CharLength, chunk1)
    offset += kManualSetupCodeChunk1CharLength
    decimalStringWithPadding(decimalString, offset, kManualSetupCodeChunk2CharLength, chunk2)
    offset += kManualSetupCodeChunk2CharLength
    decimalStringWithPadding(decimalString, offset, kManualSetupCodeChunk3CharLength, chunk3)
    offset += kManualSetupCodeChunk3CharLength

    if (useLongCode) {
      decimalStringWithPadding(
        decimalString,
        offset,
        kManualSetupVendorIdCharLength,
        payloadContents.vendorId
      )
      offset += kManualSetupVendorIdCharLength
      decimalStringWithPadding(
        decimalString,
        offset,
        kManualSetupProductIdCharLength,
        payloadContents.productId
      )
      offset += kManualSetupProductIdCharLength
    }

    val str = decimalString.concatToString().substring(0, offset)
    val checkDigit = Verhoeff10.charToVal(Verhoeff10.computeCheckChar(str))
    decimalStringWithPadding(decimalString, offset, 1, checkDigit)
    offset += 1

    // Reduce decimalString size to be the size of written data and to not include
    // null-terminator. In Kotlin, there is no direct
    // method to resize an array.We use copyOfRange(0, offset) to create a new CharArray that
    // includes only the elements from index
    // 0 to offset-1, effectively reducing the size of the buffer.
    val newDecimalString = decimalString.copyOfRange(0, offset)

    return String(newDecimalString)
  }

  private fun chunk1PayloadRepresentation(payload: OnboardingPayload): Int {
    /* <1 digit> Represents:
     *     - <bits 1..0> Discriminator <bits 11.10>
     *     - <bit 2> VID/PID present flag
     */
    val discriminatorShift =
      (kManualSetupDiscriminatorFieldLengthInBits - kManualSetupChunk1DiscriminatorMsbitsLength)
    val discriminatorMask: Int = (1 shl kManualSetupChunk1DiscriminatorMsbitsLength) - 1

    if (
      kManualSetupChunk1VidPidPresentBitPos <
        kManualSetupChunk1DiscriminatorMsbitsPos + kManualSetupChunk1DiscriminatorMsbitsLength
    ) {
      throw OnboardingPayloadException("Discriminator won't fit")
    }

    val discriminatorChunk: Int =
      (payload.getShortDiscriminatorValue() shr discriminatorShift) and discriminatorMask
    val vidPidPresentFlag: Int =
      if (payload.commissioningFlow != CommissioningFlow.STANDARD.value) 1 else 0

    return (discriminatorChunk shl kManualSetupChunk1DiscriminatorMsbitsPos) or
      (vidPidPresentFlag shl kManualSetupChunk1VidPidPresentBitPos)
  }

  private fun chunk2PayloadRepresentation(payload: OnboardingPayload): Int {
    /* <5 digits> Represents:
     *     - <bits 13..0> PIN Code <bits 13..0>
     *     - <bits 15..14> Discriminator <bits 9..8>
     */
    val discriminatorMask: Int = (1 shl kManualSetupChunk2DiscriminatorLsbitsLength) - 1
    val pincodeMask: Int = (1 shl kManualSetupChunk2PINCodeLsbitsLength) - 1

    val discriminatorChunk: Int = payload.getShortDiscriminatorValue() and discriminatorMask

    return ((payload.setupPinCode.toInt() and pincodeMask) shl
      kManualSetupChunk2PINCodeLsbitsPos) or
      (discriminatorChunk shl kManualSetupChunk2DiscriminatorLsbitsPos)
  }

  private fun chunk3PayloadRepresentation(payload: OnboardingPayload): Int {
    /* <4 digits> Represents:
     *     - <bits 12..0> PIN Code <bits 26..14>
     */
    val pincodeShift: Int = (kSetupPINCodeFieldLengthInBits - kManualSetupChunk3PINCodeMsbitsLength)
    val pincodeMask: Int = (1 shl kManualSetupChunk3PINCodeMsbitsLength) - 1

    return ((payload.setupPinCode.toInt() shr pincodeShift) and pincodeMask) shl
      kManualSetupChunk3PINCodeMsbitsPos
  }

  private fun decimalStringWithPadding(
    buffer: CharArray,
    offset: Int,
    len: Int,
    number: Int
  ): Unit {
    if (offset + len > buffer.size) {
      throw OnboardingPayloadException("The outBuffer has insufficient size")
    }

    String.format("%0${len}d", number).toCharArray(buffer, offset, 0, len)
  }
}
