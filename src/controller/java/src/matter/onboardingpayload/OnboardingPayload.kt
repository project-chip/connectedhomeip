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

// See section 5.1.2. QR Code in the Matter specification
const val kDiscriminatorLongBits = 12
const val kDiscriminatorShortBits = 4
const val kDiscriminatorLongMask = (1 shl kDiscriminatorLongBits) - 1
const val kDiscriminatorShortMask = (1 shl kDiscriminatorShortBits) - 1

const val kVersionFieldLengthInBits = 3
const val kVendorIDFieldLengthInBits = 16
const val kProductIDFieldLengthInBits = 16
const val kCommissioningFlowFieldLengthInBits = 2
const val kRendezvousInfoFieldLengthInBits = 8
const val kPayloadDiscriminatorFieldLengthInBits = kDiscriminatorLongBits
const val kSetupPINCodeFieldLengthInBits = 27
const val kPaddingFieldLengthInBits = 4
const val kRawVendorTagLengthInBits = 7

// See section 5.1.3. Manual Pairing Code in the Matter specification
const val kManualSetupDiscriminatorFieldLengthInBits = kDiscriminatorShortBits
const val kManualSetupChunk1DiscriminatorMsbitsPos = 0
const val kManualSetupChunk1DiscriminatorMsbitsLength = 2
const val kManualSetupChunk1VidPidPresentBitPos =
  (kManualSetupChunk1DiscriminatorMsbitsPos + kManualSetupChunk1DiscriminatorMsbitsLength)
const val kManualSetupChunk2PINCodeLsbitsPos = 0
const val kManualSetupChunk2PINCodeLsbitsLength = 14
const val kManualSetupChunk2DiscriminatorLsbitsPos =
  (kManualSetupChunk2PINCodeLsbitsPos + kManualSetupChunk2PINCodeLsbitsLength)
const val kManualSetupChunk2DiscriminatorLsbitsLength = 2
const val kManualSetupChunk3PINCodeMsbitsPos = 0
const val kManualSetupChunk3PINCodeMsbitsLength = 13

const val kManualSetupShortCodeCharLength = 10
const val kManualSetupLongCodeCharLength = 20
const val kManualSetupCodeChunk1CharLength = 1
const val kManualSetupCodeChunk2CharLength = 5
const val kManualSetupCodeChunk3CharLength = 4
const val kManualSetupVendorIdCharLength = 5
const val kManualSetupProductIdCharLength = 5

// Spec 5.1.4.2 CHIP-Common Reserved Tags
const val kSerialNumberTag = 0x00
const val kPBKDFIterationsTag = 0x01
const val kBPKFSaltTag = 0x02
const val kNumberOFDevicesTag = 0x03
const val kCommissioningTimeoutTag = 0x04

const val kSetupPINCodeMaximumValue = 99999998L
const val kSetupPINCodeUndefinedValue = 0L

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

/** Class to hold the data from the scanned QR code or Manual Pairing Code. */
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
  var discoveryCapabilities: MutableSet<DiscoveryCapability> = mutableSetOf(),

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
  var optionalQRCodeInfo: HashMap<Int, OptionalQRCodeInfo> = HashMap()
  private val optionalVendorData: HashMap<Int, OptionalQRCodeInfo> = HashMap()
  private val optionalExtensionData: HashMap<Int, OptionalQRCodeInfoExtension> = HashMap()

  constructor(
    version: Int,
    vendorId: Int,
    productId: Int,
    commissioningFlow: Int,
    discoveryCapabilities: MutableSet<DiscoveryCapability>,
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

  override fun equals(other: Any?): Boolean {
    if (this === other) return true
    if (other !is OnboardingPayload) return false

    return version == other.version &&
      vendorId == other.vendorId &&
      productId == other.productId &&
      commissioningFlow == other.commissioningFlow &&
      discoveryCapabilities == other.discoveryCapabilities &&
      discriminator == other.discriminator &&
      hasShortDiscriminator == other.hasShortDiscriminator &&
      setupPinCode == other.setupPinCode
  }

  override fun toString(): String {
    return "OnboardingPayload(" +
      "version=$version, " +
      "vendorId=$vendorId, " +
      "productId=$productId, " +
      "commissioningFlow=$commissioningFlow, " +
      "discoveryCapabilities=$discoveryCapabilities, " +
      "discriminator=$discriminator, " +
      "hasShortDiscriminator=$hasShortDiscriminator, " +
      "setupPinCode=$setupPinCode, " +
      "optionalQRCodeInfo=$optionalQRCodeInfo, " +
      "optionalVendorData=$optionalVendorData, " +
      "optionalExtensionData=$optionalExtensionData" +
      ")"
  }

  fun addOptionalQRCodeInfo(info: OptionalQRCodeInfo) {
    optionalQRCodeInfo[info.tag] = info
  }

  fun isValidManualCode(): Boolean {
    if (setupPinCode >= (1 shl kSetupPINCodeFieldLengthInBits)) {
      return false
    }

    return checkPayloadCommonConstraints()
  }

  fun isValidQRCodePayload(): Boolean {
    // 3-bit value specifying the QR code payload version.
    if (version >= 1 shl kVersionFieldLengthInBits) {
      return false
    }

    if (commissioningFlow.toUInt() > ((1 shl kCommissioningFlowFieldLengthInBits) - 1).toUInt()) {
      return false
    }

    // Device Commissioning Flow
    // 0: Standard commissioning flow: such a device, when uncommissioned, always enters
    // commissioning mode upon power-up, subject
    // to the rules in [ref_Announcement_Commencement]. 1: User-intent commissioning flow: user
    // action required to enter
    // commissioning mode. 2: Custom commissioning flow: interaction with a vendor-specified
    // means is needed before commissioning.
    // 3: Reserved
    if (
      commissioningFlow != CommissioningFlow.STANDARD.value &&
        commissioningFlow != CommissioningFlow.USER_ACTION_REQUIRED.value &&
        commissioningFlow != CommissioningFlow.CUSTOM.value
    ) {
      return false
    }

    val allValid =
      setOf(DiscoveryCapability.BLE, DiscoveryCapability.ON_NETWORK, DiscoveryCapability.SOFT_AP)

    // If discoveryCapabilities is not empty and discoveryCapabilities contains values outside
    // of allValid
    if (!discoveryCapabilities.isEmpty() && discoveryCapabilities.any { it !in allValid }) {
      return false
    }

    // Discriminator validity is enforced by the SetupDiscriminator class.
    if (setupPinCode >= 1 shl kSetupPINCodeFieldLengthInBits) {
      return false
    }

    return checkPayloadCommonConstraints()
  }

  fun setShortDiscriminatorValue(discriminator: Int) {
    if (discriminator != (discriminator and kDiscriminatorShortMask)) {
      throw OnboardingPayloadException("Invalid argument")
    }

    this.discriminator = (discriminator and kDiscriminatorShortMask)
    this.hasShortDiscriminator = true
  }

  fun setLongDiscriminatorValue(discriminator: Int) {
    if (discriminator != (discriminator and kDiscriminatorLongMask)) {
      throw OnboardingPayloadException("Invalid argument")
    }

    this.discriminator = (discriminator and kDiscriminatorLongMask)
    this.hasShortDiscriminator = false
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

  fun getRendezvousInformation(): Long {
    var rendezvousInfo: Long = 0

    if (discoveryCapabilities.contains(DiscoveryCapability.SOFT_AP)) {
      // set bit 0
      rendezvousInfo = rendezvousInfo or (1L shl 0)
    }

    if (discoveryCapabilities.contains(DiscoveryCapability.BLE)) {
      // set bit 1
      rendezvousInfo = rendezvousInfo or (1L shl 1)
    }

    if (discoveryCapabilities.contains(DiscoveryCapability.ON_NETWORK)) {
      // set bit 2
      rendezvousInfo = rendezvousInfo or (1L shl 2)
    }

    return rendezvousInfo
  }

  fun setRendezvousInformation(rendezvousInfo: Long) {
    // Removes all elements from discoveryCapabilities.
    discoveryCapabilities.clear()

    // bit 0 is set
    if (rendezvousInfo and (1L shl 0) != 0L) {
      discoveryCapabilities.add(DiscoveryCapability.SOFT_AP)
    }

    // bit 1 is set
    if (rendezvousInfo and (1L shl 1) != 0L) {
      discoveryCapabilities.add(DiscoveryCapability.BLE)
    }

    // bit 2 is set
    if (rendezvousInfo and (1L shl 2) != 0L) {
      discoveryCapabilities.add(DiscoveryCapability.ON_NETWORK)
    }
  }

  /**
   * A function to add a String serial number
   *
   * @param serialNumber String serial number
   */
  fun addSerialNumber(serialNumber: String) {
    val info = OptionalQRCodeInfoExtension()
    info.tag = kSerialNumberTag
    info.type = OptionalQRCodeInfoType.TYPE_STRING
    info.data = serialNumber

    addOptionalExtensionData(info)
  }

  /**
   * A function to add a Int serial number
   *
   * @param serialNumber Int serial number
   */
  fun addSerialNumber(serialNumber: Int) {
    val info = OptionalQRCodeInfoExtension()
    info.tag = kSerialNumberTag
    info.type = OptionalQRCodeInfoType.TYPE_UINT32
    info.uint32 = serialNumber.toUInt()

    addOptionalExtensionData(info)
  }

  /**
   * A function to retrieve serial number as a string
   *
   * @return retrieved string serial number
   */
  fun getSerialNumber(): String {
    val outSerialNumber = StringBuilder()
    val info = getOptionalExtensionData(kSerialNumberTag)

    when (info.type) {
      OptionalQRCodeInfoType.TYPE_STRING -> outSerialNumber.append(info.data)
      OptionalQRCodeInfoType.TYPE_UINT32 -> outSerialNumber.append(info.uint32)
      else -> throw OnboardingPayloadException("Invalid argument")
    }

    return outSerialNumber.toString()
  }

  /** A function to remove the serial number from the payload */
  fun removeSerialNumber() {
    if (optionalExtensionData.containsKey(kSerialNumberTag)) {
      optionalExtensionData.remove(kSerialNumberTag)
      return
    } else {
      throw OnboardingPayloadException("Key not found")
    }
  }

  /**
   * Checks if the tag is CHIP Common type Spec 5.1.4.2 CHIPCommon tag numbers are in the range
   * [0x00, 0x7F]
   *
   * @param tag Tag to be checked
   * @return True if the tag is of Common type, False otherwise
   */
  private fun isCommonTag(tag: Int): Boolean {
    return tag < 0x80
  }

  /**
   * Checks if the tag is vendor-specific Spec 5.1.4.1 Manufacture-specific tag numbers are in the
   * range [0x80, 0xFF]
   *
   * @param tag Tag to be checked
   * @return True if the tag is Vendor-specific, False otherwise
   */
  private fun isVendorTag(tag: Int): Boolean {
    return !isCommonTag(tag)
  }

  /**
   * A function to add an optional vendor data
   *
   * @param tag tag number in the [0x80-0xFF] range
   * @param data String representation of data to add
   */
  fun addOptionalVendorData(tag: Int, data: String) {
    val info = OptionalQRCodeInfo()
    info.tag = tag
    info.type = OptionalQRCodeInfoType.TYPE_STRING
    info.data = data

    addOptionalVendorData(info)
  }

  /**
   * A function to add an optional vendor data
   *
   * @param tag 7 bit [0-127] tag number
   * @param data Integer representation of data to add
   */
  fun addOptionalVendorData(tag: Int, data: Int) {
    val info = OptionalQRCodeInfo()
    info.tag = tag
    info.type = OptionalQRCodeInfoType.TYPE_INT32
    info.int32 = data

    addOptionalVendorData(info)
  }

  /**
   * A function to add an optional QR Code info vendor object
   *
   * @param info Optional QR code info object to add
   */
  private fun addOptionalVendorData(info: OptionalQRCodeInfo) {
    if (isVendorTag(info.tag)) {
      optionalVendorData[info.tag] = info
      return
    } else {
      throw OnboardingPayloadException("Invalid argument")
    }
  }

  /**
   * A function to remove an optional vendor data
   *
   * @param tag 7 bit [0-127] tag number
   */
  fun removeOptionalVendorData(tag: Int) {
    if (optionalVendorData.containsKey(tag)) {
      optionalVendorData.remove(tag)
      return
    } else {
      throw OnboardingPayloadException("Key not found")
    }
  }

  /**
   * A function to retrieve the vector of OptionalQRCodeInfo infos
   *
   * @return a vector of optionalQRCodeInfos
   */
  fun getAllOptionalVendorData(): List<OptionalQRCodeInfo> {
    val returnedOptionalInfo = mutableListOf<OptionalQRCodeInfo>()

    for (entry in optionalVendorData) {
      returnedOptionalInfo.add(entry.value)
    }

    return returnedOptionalInfo
  }

  /**
   * A function to add an optional QR Code info CHIP object
   *
   * @param info Optional QR code info object to add
   */
  private fun addOptionalExtensionData(info: OptionalQRCodeInfoExtension) {
    if (isCommonTag(info.tag)) {
      optionalExtensionData[info.tag] = info
      return
    } else {
      throw OnboardingPayloadException("Invalid argument")
    }
  }

  /**
   * A function to retrieve the vector of CHIPQRCodeInfo infos
   *
   * @return a vector of OptionalQRCodeInfoExtension
   */
  fun getAllOptionalExtensionData(): List<OptionalQRCodeInfoExtension> {
    val returnedOptionalInfo = mutableListOf<OptionalQRCodeInfoExtension>()
    for (entry in optionalExtensionData) {
      returnedOptionalInfo.add(entry.value)
    }

    return returnedOptionalInfo
  }

  /**
   * A function to retrieve an optional QR Code info vendor object
   *
   * @param tag 7 bit [0-127] tag number
   * @return retrieved OptionalQRCodeInfo object
   */
  private fun getOptionalVendorData(tag: Int): OptionalQRCodeInfo {
    return optionalVendorData[tag] ?: throw OnboardingPayloadException("Key not found")
  }

  /**
   * A function to retrieve an optional QR Code info extended object
   *
   * @param tag 8 bit [128-255] tag number
   * @return retrieved OptionalQRCodeInfoExtension object
   */
  private fun getOptionalExtensionData(tag: Int): OptionalQRCodeInfoExtension {
    return optionalExtensionData[tag] ?: throw OnboardingPayloadException("Key not found")
  }

  /**
   * A function to retrieve the associated expected numeric value for a tag
   *
   * @param tag 8 bit [0-255] tag number
   * @return an OptionalQRCodeInfoType value
   */
  private fun getNumericTypeFor(tag: Int): OptionalQRCodeInfoType {
    var elemType = OptionalQRCodeInfoType.TYPE_UNKNOWN

    if (isVendorTag(tag)) {
      elemType = OptionalQRCodeInfoType.TYPE_INT32
    } else if (tag == kSerialNumberTag) {
      elemType = OptionalQRCodeInfoType.TYPE_UINT32
    }

    return elemType
  }

  private fun checkPayloadCommonConstraints(): Boolean {
    if (version != 0) {
      return false
    }

    if (!isValidSetupPIN(setupPinCode)) {
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
    private fun isValidSetupPIN(setupPIN: Long): Boolean {
      // SHALL be restricted to the values 0x0000001 to 0x5F5E0FE (00000001 to 99999998 in
      // decimal),
      // excluding the invalid Passcode values.
      return (setupPIN != kSetupPINCodeUndefinedValue &&
        setupPIN <= kSetupPINCodeMaximumValue &&
        setupPIN != 11111111L &&
        setupPIN != 22222222L &&
        setupPIN != 33333333L &&
        setupPIN != 44444444L &&
        setupPIN != 55555555L &&
        setupPIN != 66666666L &&
        setupPIN != 77777777L &&
        setupPIN != 88888888L &&
        setupPIN != 12345678L &&
        setupPIN != 87654321L)
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
