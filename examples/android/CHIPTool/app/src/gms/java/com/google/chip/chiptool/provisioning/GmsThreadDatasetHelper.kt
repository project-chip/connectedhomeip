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
import android.content.IntentSender
import android.util.Log
import androidx.fragment.app.Fragment
import com.google.android.gms.common.ConnectionResult
import com.google.android.gms.common.GoogleApiAvailability
import com.google.android.gms.threadnetwork.GetAllActiveCredentialsRequest
import com.google.android.gms.threadnetwork.ThreadNetwork
import com.google.android.gms.threadnetwork.ThreadNetworkClient
import com.google.android.gms.threadnetwork.ThreadNetworkCredentials

/** GMSCore ThreadNetwork implementation enabled when `matterEnableGms=true`. */
object GmsThreadDatasetHelper {
  private const val TAG = "GmsThreadDatasetHelper"
  private const val REQUEST_CODE_ACTIVE_CREDENTIALS = 1002
  private const val REQUEST_CODE_PREFERRED_CREDENTIALS = 1003
  const val IS_AVAILABLE = true

  /**
   * Requests Thread network credentials via GMS ThreadNetwork: all active networks reachable
   * through nearby border routers, or the preferred network if that is unavailable. The credentials
   * are delivered through the consent intent result (see [parseConsentResult]); [onFallback] runs
   * when Play services or the credentials are unavailable. Callbacks run on the main thread and may
   * arrive after the fragment view is destroyed.
   */
  fun fetchFromGms(fragment: Fragment, onFallback: () -> Unit) {
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
      client
        .getAllActiveCredentials(GetAllActiveCredentialsRequest.newBuilder().build())
        .addOnSuccessListener { result ->
          if (fragment.view == null) {
            return@addOnSuccessListener
          }
          if (!launchConsent(fragment, result?.intentSender, REQUEST_CODE_ACTIVE_CREDENTIALS)) {
            fetchPreferredCredentials(client, fragment, onFallback)
          }
        }
        .addOnFailureListener { e ->
          // For example LOCAL_NETWORK_NOT_CONNECTED when the phone is not on Wi-Fi.
          Log.w(TAG, "GMS getAllActiveCredentials failed, trying getPreferredCredentials", e)
          fetchPreferredCredentials(client, fragment, onFallback)
        }
    } catch (e: Exception) {
      Log.d(TAG, "GMS ThreadNetwork API failed, falling back to system service", e)
      onFallback()
    }
  }

  private fun fetchPreferredCredentials(
    client: ThreadNetworkClient,
    fragment: Fragment,
    onFallback: () -> Unit,
  ) {
    try {
      client.preferredCredentials
        .addOnSuccessListener { result ->
          if (fragment.view == null) {
            return@addOnSuccessListener
          }
          if (!launchConsent(fragment, result?.intentSender, REQUEST_CODE_PREFERRED_CREDENTIALS)) {
            onFallback()
          }
        }
        .addOnFailureListener { e ->
          Log.w(TAG, "GMS getPreferredCredentials failed, falling back to system service", e)
          onFallback()
        }
    } catch (e: Exception) {
      Log.e(TAG, "Error calling getPreferredCredentials", e)
      onFallback()
    }
  }

  /** Launches the GMS consent prompt; returns false if there is nothing to launch or it failed. */
  private fun launchConsent(fragment: Fragment, sender: IntentSender?, requestCode: Int): Boolean {
    if (sender == null) {
      return false
    }
    return try {
      @Suppress("DEPRECATION")
      fragment.startIntentSenderForResult(sender, requestCode, null, 0, 0, 0, null)
      true
    } catch (e: Exception) {
      Log.e(TAG, "Failed to launch GMS Thread consent intent", e)
      false
    }
  }

  /**
   * Returns the networks shared through a consent prompt launched by [fetchFromGms], an empty list
   * if the user declined or nothing usable was shared, or null if [requestCode] is not ours.
   */
  fun parseConsentResult(
    requestCode: Int,
    resultCode: Int,
    data: Intent?,
  ): List<ThreadDatasetCandidate>? {
    if (
      requestCode != REQUEST_CODE_ACTIVE_CREDENTIALS &&
        requestCode != REQUEST_CODE_PREFERRED_CREDENTIALS
    ) {
      return null
    }
    if (resultCode != Activity.RESULT_OK || data == null) {
      return emptyList()
    }
    return try {
      val credentials =
        if (requestCode == REQUEST_CODE_ACTIVE_CREDENTIALS) {
          ThreadNetworkCredentials.parseListFromIntentSenderResultData(data)
        } else {
          listOf(ThreadNetworkCredentials.fromIntentSenderResultData(data))
        }
      credentials
        .filter { it.activeOperationalDataset.isNotEmpty() }
        .map {
          ThreadDatasetCandidate(
            it.networkName,
            it.extendedPanId,
            it.channel,
            it.activeOperationalDataset,
          )
        }
    } catch (e: Exception) {
      Log.e(TAG, "Failed to parse ThreadNetworkCredentials from intent result", e)
      emptyList()
    }
  }
}
