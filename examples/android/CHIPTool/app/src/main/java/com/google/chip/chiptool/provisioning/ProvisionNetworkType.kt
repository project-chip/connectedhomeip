/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.google.chip.chiptool.provisioning

/** Network type - Wi-Fi vs Thread, to be provisioned. */
enum class ProvisionNetworkType {
  THREAD,
  WIFI,
  ;

  companion object {
    fun fromName(name: String?): ProvisionNetworkType? {
      return when (name) {
        THREAD.name -> THREAD
        WIFI.name -> WIFI
        else -> null
      }
    }
  }
}
