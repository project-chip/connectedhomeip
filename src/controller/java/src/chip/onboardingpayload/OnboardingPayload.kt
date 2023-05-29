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
}
