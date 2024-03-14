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
import chip.devicecontroller.ControllerParams
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback
import chip.devicecontroller.ICDCheckInDelegate
import chip.devicecontroller.ICDClientInfo
import chip.platform.AndroidBleManager
import chip.platform.AndroidChipPlatform
import chip.platform.ChipMdnsCallbackImpl
import chip.platform.DiagnosticDataProviderImpl
import chip.platform.NsdManagerServiceBrowser
import chip.platform.NsdManagerServiceResolver
import chip.platform.PreferencesConfigurationManager
import chip.platform.PreferencesKeyValueStoreManager
import com.google.chip.chiptool.attestation.ExampleAttestationTrustStoreDelegate
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException
import kotlinx.coroutines.suspendCancellableCoroutine

/** Lazily instantiates [ChipDeviceController] and holds a reference to it. */
object ChipClient {
  private const val TAG = "ChipClient"
  private lateinit var chipDeviceController: ChipDeviceController
  private lateinit var androidPlatform: AndroidChipPlatform
  /* 0xFFF4 is a test vendor ID, replace with your assigned company ID */
  const val VENDOR_ID = 0xFFF4

  fun getDeviceController(context: Context): ChipDeviceController {
    getAndroidChipPlatform(context)

    if (!this::chipDeviceController.isInitialized) {
      chipDeviceController =
        ChipDeviceController(
          ControllerParams.newBuilder()
            .setControllerVendorId(VENDOR_ID)
            .setEnableServerInteractions(true)
            .build()
        )

      // Set delegate for attestation trust store for device attestation verifier.
      // It will replace the default attestation trust store.
      chipDeviceController.setAttestationTrustStoreDelegate(
        ExampleAttestationTrustStoreDelegate(chipDeviceController)
      )

      chipDeviceController.setICDCheckInDelegate(
        object : ICDCheckInDelegate {
          override fun onCheckInComplete(info: ICDClientInfo) {
            Log.d(TAG, "onCheckInComplete : $info")
          }

          override fun onKeyRefreshNeeded(info: ICDClientInfo): ByteArray? {
            Log.d(TAG, "onKeyRefreshNeeded : $info")
            return null
          }

          override fun onKeyRefreshDone(errorCode: Long) {
            Log.d(TAG, "onKeyRefreshDone : $errorCode")
          }
        }
      )
    }

    return chipDeviceController
  }

  fun getAndroidChipPlatform(context: Context?): AndroidChipPlatform {
    if (!this::androidPlatform.isInitialized && context != null) {
      // force ChipDeviceController load jni
      ChipDeviceController.loadJni()
      androidPlatform =
        AndroidChipPlatform(
          AndroidBleManager(context),
          PreferencesKeyValueStoreManager(context),
          PreferencesConfigurationManager(context),
          NsdManagerServiceResolver(
            context,
            NsdManagerServiceResolver.NsdManagerResolverAvailState()
          ),
          NsdManagerServiceBrowser(context),
          ChipMdnsCallbackImpl(),
          DiagnosticDataProviderImpl(context)
        )
    }

    return androidPlatform
  }

  /**
   * Wrapper around [ChipDeviceController.getConnectedDevicePointer] to return the value directly.
   */
  suspend fun getConnectedDevicePointer(context: Context, nodeId: Long): Long {
    // TODO (#21539) This is a memory leak because we currently never call
    // releaseConnectedDevicePointer
    // once we are done with the returned device pointer. Memory leak was introduced since the
    // refactor
    // that introduced it was very large in order to fix a use after free, which was considered
    // to be
    // worse than the memory leak that was introduced.
    return suspendCancellableCoroutine { continuation ->
      getDeviceController(context)
        .getConnectedDevicePointer(
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
          }
        )
    }
  }
}
