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
package com.google.chip.chiptool

import android.content.Context
import android.util.Log
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback
import chip.devicecontroller.PreferencesKeyValueStoreManager
import chip.devicecontroller.mdns.ChipMdnsCallbackImpl
import chip.devicecontroller.mdns.NsdManagerServiceResolver
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException
import kotlin.coroutines.suspendCoroutine

/** Lazily instantiates [ChipDeviceController] and holds a reference to it. */
object ChipClient {
  private const val TAG = "ChipClient"
  private lateinit var chipDeviceController: ChipDeviceController

  fun getDeviceController(context: Context): ChipDeviceController {
    if (!this::chipDeviceController.isInitialized) {
      chipDeviceController = ChipDeviceController(
        PreferencesKeyValueStoreManager(context),
        NsdManagerServiceResolver(context),
        ChipMdnsCallbackImpl()
      )
    }
    return chipDeviceController
  }

  /**
   * Wrapper around [ChipDeviceController.getConnectedDevicePointer] to return the value directly.
   */
  suspend fun getConnectedDevicePointer(context: Context, nodeId: Long): Long {
    return suspendCoroutine { continuation ->
      getDeviceController(context).getConnectedDevicePointer(
        nodeId,
        object : GetConnectedDeviceCallback {
          override fun onDeviceConnected(devicePointer: Long) {
            Log.d(TAG, "Got connected device pointer")
            continuation.resume(devicePointer)
          }

          override fun onConnectionFailure(nodeId: Long, error: Exception) {
            val errorMessage = "Unable to get connected device with nodeId $nodeId"
            Log.e(TAG, errorMessage, error)
            continuation.resumeWithException(IllegalStateException(errorMessage))
          }
        })
    }
  }
}
