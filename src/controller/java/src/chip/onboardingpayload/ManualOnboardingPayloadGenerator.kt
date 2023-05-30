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

package chip.onboardingpayload

import java.util.logging.Level
import java.util.logging.Logger

class ManualOnboardingPayloadGenerator(private val payloadContents: OnboardingPayload) {
  private var allowInvalidPayload = false
  private var forceShortCode = false

  fun setAllowInvalidPayload(allow: Boolean) {
    allowInvalidPayload = allow
  }

  fun setForceShortCode(useShort: Boolean) {
    forceShortCode = useShort
  }

  fun payloadDecimalStringRepresentation(): String {
    val decimalString = CharArray(kManualSetupLongCodeCharLength + 1 + 1)
    payloadDecimalStringRepresentation(decimalString)
    return decimalString.joinToString()
  }

  private fun chunk1PayloadRepresentation(payload: OnboardingPayload): Int {
    /* <1 digit> Represents:
     *     - <bits 1..0> Discriminator <bits 11.10>
     *     - <bit 2> VID/PID present flag
     */
    val discriminatorShift = (kManualSetupDiscriminatorFieldLengthInBits - kManualSetupChunk1DiscriminatorMsbitsLength)
    val discriminatorMask: Int = (1 shl kManualSetupChunk1DiscriminatorMsbitsLength) - 1

    if (kManualSetupChunk1VidPidPresentBitPos <
      kManualSetupChunk1DiscriminatorMsbitsPos + kManualSetupChunk1DiscriminatorMsbitsLength) {
      throw OnboardingPayloadException("Discriminator won't fit")
    }

    val discriminatorChunk: Int = (payload.getShortDiscriminatorValue() shr discriminatorShift) and discriminatorMask
    val vidPidPresentFlag: Int = if (payload.commissioningFlow != CommissioningFlow.STANDARD.value) 1 else 0

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

    return ((payload.setupPinCode.toInt() and pincodeMask) shl kManualSetupChunk2PINCodeLsbitsPos) or
           (discriminatorChunk shl kManualSetupChunk2DiscriminatorLsbitsPos)
  }

  private fun chunk3PayloadRepresentation(payload: OnboardingPayload): Int {
    /* <4 digits> Represents:
     *     - <bits 12..0> PIN Code <bits 26..14>
     */
    val pincodeShift: Int = (kSetupPINCodeFieldLengthInBits - kManualSetupChunk3PINCodeMsbitsLength)
    val pincodeMask: Int = (1 shl kManualSetupChunk3PINCodeMsbitsLength) - 1

    return ((payload.setupPinCode.toInt() shr pincodeShift) and pincodeMask) shl kManualSetupChunk3PINCodeMsbitsPos
  }

  private fun decimalStringWithPadding(buffer: CharArray, number: Int): Unit {
    val len = buffer.size - 1
    val retval = String.format("%0${len}d", number).toCharArray(buffer, 0, buffer.size)

    if (retval.size >= buffer.size) {
        throw OnboardingPayloadException("The outBuffer has insufficient size")
    } 
  }  

  private fun payloadDecimalStringRepresentation(outBuffer: CharArray): Unit {
    if (kManualSetupCodeChunk1CharLength + kManualSetupCodeChunk2CharLength + kManualSetupCodeChunk3CharLength !=
        kManualSetupShortCodeCharLength) {
      throw OnboardingPayloadException("Manual code length mismatch (short)")
    }

    if (kManualSetupShortCodeCharLength + kManualSetupVendorIdCharLength + kManualSetupProductIdCharLength !=
        kManualSetupLongCodeCharLength) {
      throw OnboardingPayloadException("Manual code length mismatch (long)")
    }

    if (kManualSetupChunk1DiscriminatorMsbitsLength + kManualSetupChunk2DiscriminatorLsbitsLength !=
        kManualSetupDiscriminatorFieldLengthInBits) {
      throw OnboardingPayloadException("Discriminator won't fit")
    }

    if (kManualSetupChunk2PINCodeLsbitsLength + kManualSetupChunk3PINCodeMsbitsLength !=
        kSetupPINCodeFieldLengthInBits) {
      throw OnboardingPayloadException("PIN code won't fit")
    }

    val useLongCode = (payloadContents.commissioningFlow != CommissioningFlow.STANDARD.value) && !forceShortCode

    if (!allowInvalidPayload && !payloadContents.isValidManualCode()) {
      throw OnboardingPayloadException("The payload is invalid")
    }

    // Add two for the check digit and null terminator.
    if ((useLongCode && outBuffer.size < kManualSetupLongCodeCharLength + 2) ||
        (!useLongCode && outBuffer.size < kManualSetupShortCodeCharLength + 2)) {
      throw OnboardingPayloadException("The outBuffer has insufficient size")
    }

    val chunk1 = chunk1PayloadRepresentation(payloadContents)
    val chunk2 = chunk2PayloadRepresentation(payloadContents)
    val chunk3 = chunk3PayloadRepresentation(payloadContents)

    var offset = 0

    // Add one to the length of each chunk, since snprintf writes a null terminator.
    decimalStringWithPadding(outBuffer.sliceArray(offset until offset + kManualSetupCodeChunk1CharLength + 1), chunk1)
    offset += kManualSetupCodeChunk1CharLength
    decimalStringWithPadding(outBuffer.sliceArray(offset until offset + kManualSetupCodeChunk2CharLength + 1), chunk2)
    offset += kManualSetupCodeChunk2CharLength
    decimalStringWithPadding(outBuffer.sliceArray(offset until offset + kManualSetupCodeChunk3CharLength + 1), chunk3)
    offset += kManualSetupCodeChunk3CharLength
    
    if (useLongCode) {
      decimalStringWithPadding(outBuffer.sliceArray(offset until offset + kManualSetupVendorIdCharLength + 1), payloadContents.vendorId)
      offset += kManualSetupVendorIdCharLength
      decimalStringWithPadding(outBuffer.sliceArray(offset until offset + kManualSetupProductIdCharLength + 1), payloadContents.productId)
      offset += kManualSetupProductIdCharLength
    }

    val checkDigit = Verhoeff10.charToVal(Verhoeff10.computeCheckChar(outBuffer.concatToString()))
    decimalStringWithPadding(outBuffer.sliceArray(offset until offset + 2), checkDigit)
    offset += 1

    // Reduce outBuffer span size to be the size of written data and to not include null-terminator. In Kotlin, there is no direct
    // method to resize an array.We use copyOfRange(0, offset) to create a new CharArray that includes only the elements from index
    // 0 to offset-1, effectively reducing the size of the buffer.
    outBuffer.copyOfRange(0, offset)     
  }
}
