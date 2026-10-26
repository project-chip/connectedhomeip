/*
 *   Copyright (c) 2026 Project CHIP Authors
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
package com.google.chip.chiptool.provisioning

import kotlinx.coroutines.CancellationException
import matter.onboardingpayload.DiscoveryCapability
import matter.onboardingpayload.OnboardingPayload

enum class NetworkType {
  WIFI,
  THREAD,
  ON_NETWORK,
  NONE,
}

class UnifiedCommissioningLogic {
  companion object {
    fun guessNetworkTypeFromPayload(payload: OnboardingPayload?): NetworkType {
      if (payload == null) return NetworkType.NONE

      val caps = payload.discoveryCapabilities
      if (caps.contains(DiscoveryCapability.ON_NETWORK)) {
        return NetworkType.ON_NETWORK
      }
      if (caps.contains(DiscoveryCapability.BLE)) {
        return NetworkType.WIFI
      }
      if (caps.contains(DiscoveryCapability.SOFT_AP)) {
        return NetworkType.WIFI
      }
      return NetworkType.WIFI
    }

    fun isValidWifiCredentials(ssid: String, pass: String): Boolean {
      if (ssid.isEmpty() || ssid.toByteArray().size > 32) return false
      if (pass.isNotEmpty() && pass.length < 8) return false
      return true
    }

    fun isValidThreadDataset(hex: String): Boolean {
      if (hex.isEmpty()) return false
      if (hex.length % 2 != 0) return false
      if (hex.length > 508) return false // 254 bytes
      val hexRegex = Regex("^[0-9a-fA-F]+$")
      return hexRegex.matches(hex)
    }

    fun getValidationTypeForState(
      selectedId: Int,
      awaitingCredentialsFor: NetworkType?,
    ): NetworkType {
      if (awaitingCredentialsFor != null) {
        return awaitingCredentialsFor
      }
      return when (selectedId) {
        com.google.chip.chiptool.R.id.wifiButton -> NetworkType.WIFI
        com.google.chip.chiptool.R.id.threadButton -> NetworkType.THREAD
        else -> NetworkType.ON_NETWORK
      }
    }

    /**
     * Runs [block] and reports any failure to [onFailure], except coroutine cancellation, which is
     * rethrown: a user Cancel must never show up as a pairing error.
     */
    suspend fun runReportingFailures(onFailure: (Exception) -> Unit, block: suspend () -> Unit) {
      try {
        block()
      } catch (e: CancellationException) {
        throw e
      } catch (e: Exception) {
        onFailure(e)
      }
    }
  }
}

/**
 * Which network credentials the commissioner is waiting for, if any. The request belongs to one
 * commissioning session: every terminal event (failure, completion, cancel, teardown) must call
 * [onSessionEnded], or Start/Retry would send credentials to a session that no longer exists.
 */
class CredentialsRequestState {
  var awaiting: NetworkType? = null
    private set

  /** True if Start/Retry should hand credentials to the running session. */
  val shouldSubmitCredentials: Boolean
    get() = awaiting != null

  fun onCredentialsNeeded(type: NetworkType) {
    awaiting = type
  }

  fun onSessionEnded() {
    awaiting = null
  }
}
