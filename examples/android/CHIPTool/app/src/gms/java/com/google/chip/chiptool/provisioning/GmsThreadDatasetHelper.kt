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

import android.app.Activity
import android.content.Intent
import android.util.Log
import androidx.fragment.app.Fragment
import com.google.android.gms.common.ConnectionResult
import com.google.android.gms.common.GoogleApiAvailability
import com.google.android.gms.threadnetwork.ThreadNetwork
import com.google.android.gms.threadnetwork.ThreadNetworkClient
import com.google.android.gms.threadnetwork.ThreadNetworkCredentials

/** GMSCore ThreadNetwork implementation enabled when `matterEnableGms=true`. */
object GmsThreadDatasetHelper {
  private const val TAG = "GmsThreadDatasetHelper"
  const val IS_AVAILABLE = true

  /**
   * Fetches the active dataset via GMS ThreadNetwork. Callbacks run on the main thread and may
   * arrive after [fragment] is detached; callers must tolerate that.
   */
  fun fetchFromGms(
    fragment: Fragment,
    requestCodeConsent: Int,
    onDatasetFetched: (ByteArray) -> Unit,
    onFallback: () -> Unit,
  ) {
    try {
      val context = fragment.requireContext()
      // Without Play services, GoogleApiManager keeps the call pending while it offers a
      // "Get Google Play services" resolution, so the Task would never complete or fail.
      if (
        GoogleApiAvailability.getInstance().isGooglePlayServicesAvailable(context) !=
          ConnectionResult.SUCCESS
      ) {
        Log.d(TAG, "Google Play services unavailable, falling back to system service")
        onFallback()
        return
      }
      val client = ThreadNetwork.getClient(context)
      Log.d(TAG, "Attempting GMS ThreadNetwork.getClient().allCredentials")
      client.allCredentials
        .addOnSuccessListener { credentialsList ->
          Log.d(TAG, "GMS getAllCredentials onSuccess: size=${credentialsList?.size}")
          val tlvs = credentialsList?.firstOrNull()?.activeOperationalDataset
          if (tlvs != null && tlvs.isNotEmpty()) {
            Log.d(TAG, "Successfully fetched Thread dataset TLVs from GMS getAllCredentials()")
            onDatasetFetched(tlvs)
            return@addOnSuccessListener
          }
          Log.w(
            TAG,
            "GMS ThreadNetwork returned empty credentials list, " +
              "attempting getPreferredCredentials consent fallback",
          )
          fetchPreferredGmsCredentials(client, fragment, requestCodeConsent, onFallback)
        }
        .addOnFailureListener { e ->
          Log.w(
            TAG,
            "GMS ThreadNetwork getAllCredentials failed, " +
              "attempting getPreferredCredentials consent fallback",
            e,
          )
          fetchPreferredGmsCredentials(client, fragment, requestCodeConsent, onFallback)
        }
    } catch (e: Exception) {
      Log.d(TAG, "GMS ThreadNetwork API failed, falling back to system service", e)
      onFallback()
    }
  }

  private fun fetchPreferredGmsCredentials(
    client: ThreadNetworkClient,
    fragment: Fragment,
    requestCodeConsent: Int,
    onFallback: () -> Unit,
  ) {
    try {
      client.preferredCredentials
        .addOnSuccessListener { result ->
          Log.d(TAG, "GMS getPreferredCredentials onSuccess: $result")
          if (!fragment.isAdded) {
            return@addOnSuccessListener
          }
          val sender = result?.intentSender
          if (sender != null) {
            try {
              @Suppress("DEPRECATION")
              fragment.startIntentSenderForResult(sender, requestCodeConsent, null, 0, 0, 0, null)
              return@addOnSuccessListener
            } catch (e: Exception) {
              Log.e(TAG, "Failed to launch GMS Thread consent intent", e)
            }
          }
          onFallback()
        }
        .addOnFailureListener { e ->
          Log.w(TAG, "GMS getPreferredCredentials failed", e)
          onFallback()
        }
    } catch (e: Exception) {
      Log.e(TAG, "Error calling getPreferredCredentials", e)
      onFallback()
    }
  }

  fun parseConsentResult(resultCode: Int, data: Intent?): ByteArray? {
    if (resultCode != Activity.RESULT_OK || data == null) {
      return null
    }
    return try {
      ThreadNetworkCredentials.fromIntentSenderResultData(data).activeOperationalDataset.takeIf {
        it.isNotEmpty()
      }
    } catch (e: Exception) {
      Log.e(TAG, "Failed to parse ThreadNetworkCredentials from intent result", e)
      null
    }
  }
}
