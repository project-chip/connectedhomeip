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

// See section 5.1.2. QR Code in the Matter specification
const val kDiscriminatorLongBits = 12
const val kDiscriminatorShortBits = 4
const val kDiscriminatorLongMask = (1 shl kDiscriminatorLongBits) - 1
const val kDiscriminatorShortMask = (1 shl kDiscriminatorShortBits) - 1

const val kVersionFieldLengthInBits              = 3
const val kVendorIDFieldLengthInBits             = 16
const val kProductIDFieldLengthInBits            = 16
const val kCommissioningFlowFieldLengthInBits    = 2
const val kRendezvousInfoFieldLengthInBits       = 8
const val kPayloadDiscriminatorFieldLengthInBits = kDiscriminatorLongBits
const val kSetupPINCodeFieldLengthInBits         = 27
const val kPaddingFieldLengthInBits              = 4
const val kRawVendorTagLengthInBits              = 7

// See section 5.1.3. Manual Pairing Code in the Matter specification
const val kManualSetupDiscriminatorFieldLengthInBits  = kDiscriminatorShortBits
const val kManualSetupChunk1DiscriminatorMsbitsPos    = 0
const val kManualSetupChunk1DiscriminatorMsbitsLength = 2
const val kManualSetupChunk1VidPidPresentBitPos =
    (kManualSetupChunk1DiscriminatorMsbitsPos + kManualSetupChunk1DiscriminatorMsbitsLength)
const val kManualSetupChunk2PINCodeLsbitsPos       = 0
const val kManualSetupChunk2PINCodeLsbitsLength    = 14
const val kManualSetupChunk2DiscriminatorLsbitsPos = (kManualSetupChunk2PINCodeLsbitsPos + kManualSetupChunk2PINCodeLsbitsLength)
const val kManualSetupChunk2DiscriminatorLsbitsLength = 2
const val kManualSetupChunk3PINCodeMsbitsPos          = 0
const val kManualSetupChunk3PINCodeMsbitsLength       = 13

const val kManualSetupShortCodeCharLength  = 10
const val kManualSetupLongCodeCharLength   = 20
const val kManualSetupCodeChunk1CharLength = 1
const val kManualSetupCodeChunk2CharLength = 5
const val kManualSetupCodeChunk3CharLength = 4
const val kManualSetupVendorIdCharLength   = 5
const val kManualSetupProductIdCharLength  = 5

// Spec 5.1.4.2 CHIP-Common Reserved Tags
const val kSerialNumberTag         = 0x00
const val kPBKDFIterationsTag      = 0x01
const val kBPKFSaltTag             = 0x02
const val kNumberOFDevicesTag      = 0x03
const val kCommissioningTimeoutTag = 0x04

const val kSetupPINCodeMaximumValue   = 99999998
const val kSetupPINCodeUndefinedValue = 0

const val kTotalPayloadDataSizeInBits: Int =
    kVersionFieldLengthInBits +
    kVendorIDFieldLengthInBits +
    kProductIDFieldLengthInBits +
    kCommissioningFlowFieldLengthInBits +
    kRendezvousInfoFieldLengthInBits +
    kPayloadDiscriminatorFieldLengthInBits +
    kSetupPINCodeFieldLengthInBits +
    kPaddingFieldLengthInBits

const val kTotalPayloadDataSizeInBytes: Int = kTotalPayloadDataSizeInBits / 8

const val kQRCodePrefix = "MT:"

/** Class to hold the data from the scanned QR code or Manual Pairing Code.  */
class OnboardingPayload(
  /** Version info of the OnboardingPayload: version SHALL be 0 */
  var version: Int = 0,
  
  /** The CHIP device vendor ID: Vendor ID SHALL be between 1 and 0xFFF4. */
  var vendorId: Int = 0,
  
  /** The CHIP device product ID: Product ID SHALL BE greater than 0. */
  var productId: Int = 0,
  
  /** Commissioning flow: 0 = standard, 1 = requires user action, 2 = custom */
  var commissioningFlow: Int = 0,
  
  /**
   * The CHIP device supported rendezvous flags: At least one DiscoveryCapability must be included.
   */
  var discoveryCapabilities: Set<DiscoveryCapability> = emptySet(),
  
  /** The CHIP device discriminator: */
  var discriminator: Int = 0,
  
  /**
   * If hasShortDiscriminator is true, the discriminator value contains just the high 4 bits of the
   * full discriminator. For example, if hasShortDiscriminator is true and discriminator is 0xA,
   * then the full discriminator can be anything in the range 0xA00 to 0xAFF.
   */
  var hasShortDiscriminator: Boolean = false,
  
  /**
   * The CHIP device setup PIN code: setupPINCode SHALL be greater than 0. Also invalid setupPINCode
   * is {000000000, 11111111, 22222222, 33333333, 44444444, 55555555, 66666666, 77777777, 88888888,
   * 99999999, 12345678, 87654321}.
   */
  var setupPinCode: Long = 0
) {
  var optionalQRCodeInfo: HashMap<Int, OptionalQRCodeInfo>

  init {
    optionalQRCodeInfo = HashMap()
  }

  constructor(
    version: Int,
    vendorId: Int,
    productId: Int,
    commissioningFlow: Int,
    discoveryCapabilities: Set<DiscoveryCapability>,
    discriminator: Int,
    setupPinCode: Long
  ) : this(
    version,
    vendorId,
    productId,
    commissioningFlow,
    discoveryCapabilities,
    discriminator,
    false,
    setupPinCode
  )

  fun addOptionalQRCodeInfo(info: OptionalQRCodeInfo) {
    optionalQRCodeInfo[info.tag] = info
  }

  fun isValidManualCode(): Boolean {
    if (setupPinCode >= (1 shl kSetupPINCodeFieldLengthInBits)) {
      return false
    }

    return checkPayloadCommonConstraints()
  }

  fun getShortDiscriminatorValue(): Int {
    if (hasShortDiscriminator) {
      return discriminator
    }
    return longToShortValue(discriminator)
  }

  fun getLongDiscriminatorValue(): Int {
    if (hasShortDiscriminator) {
      return shortToLongValue(discriminator)
    }
    return discriminator
  }

  private fun checkPayloadCommonConstraints(): Boolean {
    if (version != 0) {
      return false
    }

    if (!isValidSetupPIN(setupPinCode.toInt())) {
      return false
    }

    if (!isVendorIdValidOperationally(vendorId) && vendorId != VendorId.UNSPECIFIED.value) {
      return false
    }

    if (productId == 0 && vendorId != VendorId.UNSPECIFIED.value) {
      return false
    }

    return true
  }

  private fun isVendorIdValidOperationally(vendorId: Int): Boolean {
    return vendorId != VendorId.UNSPECIFIED.value && vendorId <= VendorId.TESTVENDOR4.value
  }     

  companion object {
    private fun isValidSetupPIN(setupPIN: Int): Boolean {
      return (setupPIN != kSetupPINCodeUndefinedValue && setupPIN <= kSetupPINCodeMaximumValue &&
              setupPIN != 11111111 && setupPIN != 22222222 && setupPIN != 33333333 &&
              setupPIN != 44444444 && setupPIN != 55555555 && setupPIN != 66666666 &&
              setupPIN != 77777777 && setupPIN != 88888888 && setupPIN != 12345678 &&
              setupPIN != 87654321)
    }

    private fun longToShortValue(longValue: Int): Int {
      return (longValue shr (kDiscriminatorLongBits - kDiscriminatorShortBits))
    }

    private fun shortToLongValue(shortValue: Int): Int {
      return (shortValue shl (kDiscriminatorLongBits - kDiscriminatorShortBits))
    }
  }  
}

class UnrecognizedQrCodeException(qrCode: String) :
  Exception(String.format("Invalid QR code string: %s", qrCode), null) {
  companion object {
    private const val serialVersionUID = 1L
  }
}

class InvalidManualPairingCodeFormatException(entryCode: String) :
  Exception(String.format("Invalid format for entry code string: %s", entryCode), null) {
  companion object {
    private const val serialVersionUID = 1L
  }
}

class OnboardingPayloadException(message: String) :
  Exception(String.format("Failed to encode Onboarding payload to buffer: %s", message)) {
  companion object {
    private const val serialVersionUID = 1L
  }
}
