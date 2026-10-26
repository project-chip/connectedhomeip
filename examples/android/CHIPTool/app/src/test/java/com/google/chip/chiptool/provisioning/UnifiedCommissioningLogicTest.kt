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

import com.google.chip.chiptool.R
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.awaitCancellation
import kotlinx.coroutines.cancelAndJoin
import kotlinx.coroutines.launch
import kotlinx.coroutines.test.runTest
import matter.onboardingpayload.DiscoveryCapability
import matter.onboardingpayload.OnboardingPayload
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test

class UnifiedCommissioningLogicTest {

  @Test
  fun testGuessNetworkTypeFromPayload_OnNetwork() {
    val payload = OnboardingPayload()
    payload.discoveryCapabilities = mutableSetOf(DiscoveryCapability.ON_NETWORK)
    assertEquals(
      NetworkType.ON_NETWORK,
      UnifiedCommissioningLogic.guessNetworkTypeFromPayload(payload),
    )
  }

  @Test
  fun testGuessNetworkTypeFromPayload_BLE() {
    val payload = OnboardingPayload()
    payload.discoveryCapabilities = mutableSetOf(DiscoveryCapability.BLE)
    assertEquals(NetworkType.WIFI, UnifiedCommissioningLogic.guessNetworkTypeFromPayload(payload))
  }

  @Test
  fun testGuessNetworkTypeFromPayload_Null() {
    assertEquals(NetworkType.NONE, UnifiedCommissioningLogic.guessNetworkTypeFromPayload(null))
  }

  @Test
  fun testIsValidWifiCredentials() {
    assertTrue(UnifiedCommissioningLogic.isValidWifiCredentials("MySSID", "password"))
    assertTrue(UnifiedCommissioningLogic.isValidWifiCredentials("MySSID", "")) // open network
    assertFalse(UnifiedCommissioningLogic.isValidWifiCredentials("", "password")) // empty ssid
    assertFalse(UnifiedCommissioningLogic.isValidWifiCredentials("MySSID", "short")) // < 8 chars
    val longSsid = "a".repeat(33)
    assertFalse(
      UnifiedCommissioningLogic.isValidWifiCredentials(longSsid, "password")
    ) // > 32 chars
  }

  @Test
  fun testIsValidThreadDataset() {
    assertTrue(UnifiedCommissioningLogic.isValidThreadDataset("00112233aabbccdd"))
    assertFalse(UnifiedCommissioningLogic.isValidThreadDataset(""))
    assertFalse(UnifiedCommissioningLogic.isValidThreadDataset("123")) // odd length
    assertFalse(UnifiedCommissioningLogic.isValidThreadDataset("123G")) // invalid hex
    val longHex = "00".repeat(255)
    assertFalse(UnifiedCommissioningLogic.isValidThreadDataset(longHex)) // > 254 bytes
  }

  @Test
  fun testGetValidationTypeForState() {
    assertEquals(
      NetworkType.WIFI,
      UnifiedCommissioningLogic.getValidationTypeForState(R.id.wifiButton, null),
    )
    assertEquals(
      NetworkType.THREAD,
      UnifiedCommissioningLogic.getValidationTypeForState(R.id.threadButton, null),
    )
    assertEquals(
      NetworkType.WIFI,
      UnifiedCommissioningLogic.getValidationTypeForState(R.id.threadButton, NetworkType.WIFI),
    )
  }

  @Test
  fun testCredentialsRequestEndsWithTheSession() {
    val state = CredentialsRequestState()
    assertFalse(state.shouldSubmitCredentials)

    state.onCredentialsNeeded(NetworkType.WIFI)
    assertTrue(state.shouldSubmitCredentials)
    assertEquals(
      NetworkType.WIFI,
      UnifiedCommissioningLogic.getValidationTypeForState(R.id.threadButton, state.awaiting),
    )

    // Commissioning failed: Retry must start a new session, not submit credentials.
    state.onSessionEnded()
    assertFalse(state.shouldSubmitCredentials)
    assertNull(state.awaiting)
    assertEquals(
      NetworkType.THREAD,
      UnifiedCommissioningLogic.getValidationTypeForState(R.id.threadButton, state.awaiting),
    )
  }

  @Test
  fun testRunReportingFailuresRethrowsCancellation() = runTest {
    val failures = mutableListOf<Exception>()
    val started = CompletableDeferred<Unit>()
    val job = launch {
      UnifiedCommissioningLogic.runReportingFailures(onFailure = { failures += it }) {
        started.complete(Unit)
        awaitCancellation()
      }
    }
    started.await()
    job.cancelAndJoin()

    assertTrue("Cancel is not a pairing failure: $failures", failures.isEmpty())
    assertTrue(job.isCancelled)
  }

  @Test
  fun testRunReportingFailuresReportsOtherErrors() = runTest {
    val failures = mutableListOf<Exception>()
    UnifiedCommissioningLogic.runReportingFailures(onFailure = { failures += it }) {
      throw IllegalStateException("gatt connect failed")
    }
    assertEquals(listOf("gatt connect failed"), failures.map { it.message })
  }
}
