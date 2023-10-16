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

import java.util.concurrent.atomic.AtomicInteger

/** A class that can be used to convert a decimal encoded payload to a OnboardingPayload object. */
class ManualOnboardingPayloadParser(decimalRepresentation: String) {
  private val decimalStringRepresentation: String

  init {
    // '-' might be being used in the decimal code as a digit group
    // separator, to aid in readability. It's not actually part of the
    // decimal code, so strip it out.
    decimalStringRepresentation = decimalRepresentation.replace("-", "")
  }

  fun populatePayload(): OnboardingPayload {
    var representationWithoutCheckDigit: String
    var outPayload: OnboardingPayload = OnboardingPayload()

    representationWithoutCheckDigit = checkDecimalStringValidity(decimalStringRepresentation)

    var stringOffset: AtomicInteger = AtomicInteger(0)

    var chunk1 =
      readDigitsFromDecimalString(
        representationWithoutCheckDigit,
        stringOffset,
        kManualSetupCodeChunk1CharLength
      )
    var chunk2 =
      readDigitsFromDecimalString(
        representationWithoutCheckDigit,
        stringOffset,
        kManualSetupCodeChunk2CharLength
      )
    var chunk3 =
      readDigitsFromDecimalString(
        representationWithoutCheckDigit,
        stringOffset,
        kManualSetupCodeChunk3CharLength
      )

    // First digit of '8' or '9' would be invalid for v1 and would indicate new format (e.g.
    // version 2)
    if (chunk1 == 8u || chunk1 == 9u) {
      throw InvalidManualPairingCodeFormatException("Invalid argument")
    }

    val isLongCode = ((chunk1 shr kManualSetupChunk1VidPidPresentBitPos) and 1u) == 1u
    checkCodeLengthValidity(representationWithoutCheckDigit, isLongCode)

    val kDiscriminatorMsbitsMask: UInt = (1u shl kManualSetupChunk1DiscriminatorMsbitsLength) - 1u
    val kDiscriminatorLsbitsMask: UInt = (1u shl kManualSetupChunk2DiscriminatorLsbitsLength) - 1u

    var discriminator =
      ((chunk2 shr kManualSetupChunk2DiscriminatorLsbitsPos) and kDiscriminatorLsbitsMask).toInt()

    discriminator =
      discriminator or
        (((chunk1 shr kManualSetupChunk1DiscriminatorMsbitsPos) and kDiscriminatorMsbitsMask)
          .toInt() shl kManualSetupChunk2DiscriminatorLsbitsLength)

    val kPincodeMsbitsMask: UInt = (1u shl kManualSetupChunk3PINCodeMsbitsLength) - 1u
    val kPincodeLsbitsMask: UInt = (1u shl kManualSetupChunk2PINCodeLsbitsLength) - 1u

    var setUpPINCode: Long =
      ((chunk2 shr kManualSetupChunk2PINCodeLsbitsPos) and kPincodeLsbitsMask).toLong()

    setUpPINCode =
      setUpPINCode or
        (((chunk3 shr kManualSetupChunk3PINCodeMsbitsPos) and kPincodeMsbitsMask).toLong() shl
          kManualSetupChunk2PINCodeLsbitsLength)

    if (setUpPINCode == 0L) {
      throw InvalidManualPairingCodeFormatException("Failed decoding base10. SetUpPINCode was 0")
    }

    if (isLongCode) {
      var vendorID =
        readDigitsFromDecimalString(
          representationWithoutCheckDigit,
          stringOffset,
          kManualSetupVendorIdCharLength
        )
      var productID =
        readDigitsFromDecimalString(
          representationWithoutCheckDigit,
          stringOffset,
          kManualSetupProductIdCharLength
        )

      // Need to do dynamic checks, because we are reading 5 chars, so could have 99,999 here
      // or something.
      if (vendorID !in UShort.MIN_VALUE..UShort.MAX_VALUE) {
        throw InvalidManualPairingCodeFormatException("Invalid integer value")
      }

      outPayload.vendorId = vendorID.toInt()
      if (productID !in UShort.MIN_VALUE..UShort.MAX_VALUE) {
        throw InvalidManualPairingCodeFormatException("Invalid interger value")
      }
      outPayload.productId = productID.toInt()
    }

    outPayload.commissioningFlow =
      if (isLongCode) CommissioningFlow.CUSTOM.value else CommissioningFlow.STANDARD.value
    require(kSetupPINCodeFieldLengthInBits <= 32) { "Won't fit in UInt32" }
    outPayload.setupPinCode = setUpPINCode
    require(kManualSetupDiscriminatorFieldLengthInBits <= 8) { "Won't fit in UInt8" }
    outPayload.discriminator = discriminator
    outPayload.hasShortDiscriminator = true

    return outPayload
  }

  companion object {
    fun toNumber(decimalString: String): UInt {
      var number: UInt = 0u
      for (c in decimalString) {
        if (!c.isDigit()) {
          throw InvalidManualPairingCodeFormatException(
            "Failed decoding base10. Character was invalid $c"
          )
        }
        number *= 10u
        number += (c - '0').toUInt()
      }
      return number
    }

    fun checkDecimalStringValidity(decimalString: String): String {
      if (decimalString.length < 2) {
        throw InvalidManualPairingCodeFormatException(
          "Failed decoding base10. Input was empty. ${decimalString.length}"
        )
      }
      val repWithoutCheckChar = decimalString.substring(0, decimalString.length - 1)

      val checkChar = decimalString.last()

      if (!Verhoeff10.validateCheckChar(checkChar, repWithoutCheckChar)) {
        throw InvalidManualPairingCodeFormatException("Integrity check failed")
      }

      return repWithoutCheckChar
    }

    fun checkCodeLengthValidity(decimalString: String, isLongCode: Boolean): Unit {
      val expectedCharLength =
        if (isLongCode) kManualSetupLongCodeCharLength else kManualSetupShortCodeCharLength
      if (decimalString.length != expectedCharLength) {
        throw InvalidManualPairingCodeFormatException(
          "Failed decoding base10. Input length ${decimalString.length} was not expected length $expectedCharLength"
        )
      }
    }

    // Populate numberOfChars into dest from decimalString starting at startIndex (least
    // significant digit = left-most digit)
    fun readDigitsFromDecimalString(
      decimalString: String,
      index: AtomicInteger,
      numberOfCharsToRead: Int
    ): UInt {
      val startIndex = index.get()
      if (
        decimalString.length < numberOfCharsToRead ||
          (numberOfCharsToRead + startIndex > decimalString.length)
      ) {
        throw InvalidManualPairingCodeFormatException(
          "Failed decoding base10. Input was too short. ${decimalString.length}"
        )
      }

      val decimalSubstring =
        decimalString.substring(index.toInt(), (startIndex + numberOfCharsToRead).toInt())
      index.set(startIndex + numberOfCharsToRead)
      return toNumber(decimalSubstring)
    }
  }
}
