/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package com.google.chip.chiptool.setuppayloadscanner

import android.os.Parcelable
import chip.setuppayload.DiscoveryCapability
import chip.setuppayload.SetupPayload
import kotlinx.android.parcel.Parcelize

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
  val discoveryCapabilities: Set<DiscoveryCapability> = setOf(),
  val ipAddress: String? = null,

  ) : Parcelable {

  companion object {
    fun fromSetupPayload(setupPayload: SetupPayload): CHIPDeviceInfo {
      return CHIPDeviceInfo(
        setupPayload.version,
        setupPayload.vendorId,
        setupPayload.productId,
        setupPayload.discriminator,
        setupPayload.setupPinCode,
        setupPayload.commissioningFlow,
        setupPayload.optionalQRCodeInfo.mapValues { (_, info) ->
          QrCodeInfo(
            info.tag,
            info.type,
            info.data,
            info.int32
          )
        },
        setupPayload.discoveryCapabilities,
      )
    }
  }
}
