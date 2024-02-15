/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

package com.google.chip.chiptool.setuppayloadscanner

import android.os.Parcelable
import kotlinx.parcelize.Parcelize
import matter.onboardingpayload.DiscoveryCapability
import matter.onboardingpayload.OnboardingPayload

/** Class to hold the CHIP device information. */
@Parcelize
data class CHIPDeviceInfo(
  val version: Int = 0,
  val vendorId: Int = 0,
  val productId: Int = 0,
  val discriminator: Int = 0,
  val setupPinCode: Long = 0L,
  var commissioningFlow: Int = 0,
  val optionalQrCodeInfoMap: Map<Int, QrCodeInfo> = mapOf(),
  val discoveryCapabilities: MutableSet<DiscoveryCapability> = mutableSetOf(),
  val isShortDiscriminator: Boolean = false,
  val ipAddress: String? = null,
  val port: Int = 5540
) : Parcelable {

  companion object {
    fun fromSetupPayload(setupPayload: OnboardingPayload): CHIPDeviceInfo {
      return CHIPDeviceInfo(
        setupPayload.version,
        setupPayload.vendorId,
        setupPayload.productId,
        setupPayload.getLongDiscriminatorValue(),
        setupPayload.setupPinCode,
        setupPayload.commissioningFlow,
        setupPayload.optionalQRCodeInfo.mapValues { (_, info) ->
          QrCodeInfo(info.tag, info.type, info.data, info.int32)
        },
        setupPayload.discoveryCapabilities,
        setupPayload.hasShortDiscriminator
      )
    }
  }
}
