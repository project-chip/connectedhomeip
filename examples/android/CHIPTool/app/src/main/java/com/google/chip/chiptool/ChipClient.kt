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
import chip.platform.AndroidNfcCommissioningManager
import chip.platform.ChipMdnsCallbackImpl
import chip.platform.DiagnosticDataProviderImpl
import chip.platform.NsdManagerServiceBrowser
import chip.platform.NsdManagerServiceResolver
import chip.platform.PreferencesConfigurationManager
import chip.platform.PreferencesKeyValueStoreManager
import com.google.chip.chiptool.attestation.ExampleAttestationTrustStoreDelegate
import com.google.chip.chiptool.clusterclient.ICDCheckInCallback
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
  private var androidNfcCommissioningManager: AndroidNfcCommissioningManager =
    AndroidNfcCommissioningManager()

  private var icdCheckInCallback: ICDCheckInCallback? = null

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
            icdCheckInCallback?.notifyCheckInMessage(info)
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
          androidNfcCommissioningManager,
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

  fun setICDCheckInCallback(callback: ICDCheckInCallback) {
    icdCheckInCallback = callback
  }

  fun startDnssd(context: Context) {
    if (!this::chipDeviceController.isInitialized) {
      getDeviceController(context)
    } else {
      chipDeviceController.startDnssd()
    }
  }

  fun stopDnssd(context: Context) {
    if (!this::chipDeviceController.isInitialized) {
      getDeviceController(context)
    }
    chipDeviceController.stopDnssd()
  }

  /**
   * Wrapper around [ChipDeviceController.getConnectedDevicePointer] to return the value directly.
   *
   * Prefer [withConnectedDevice] over this function. Callers of this function are responsible for
   * calling [ChipDeviceController.releaseConnectedDevicePointer] when done with the pointer to
   * avoid a native memory leak.
   */
  suspend fun getConnectedDevicePointer(context: Context, nodeId: Long): Long {
    return suspendCancellableCoroutine { continuation ->
      getDeviceController(context)
        .getConnectedDevicePointer(
          nodeId,
          object : GetConnectedDeviceCallback {
            override fun onDeviceConnected(devicePointer: Long) {
              Log.d(TAG, "Got connected device pointer")
              if (continuation.isActive) {
                continuation.resume(devicePointer)
              }
            }

            override fun onConnectionFailure(nodeId: Long, error: Exception) {
              val errorMessage = "Unable to get connected device with nodeId $nodeId"
              Log.e(TAG, errorMessage, error)
              if (continuation.isActive) {
                continuation.resumeWithException(IllegalStateException(errorMessage))
              }
            }
          }
        )
    }
  }

  /**
   * Acquires a native device pointer for [nodeId], invokes [block] with it, and then releases the
   * pointer via [ChipDeviceController.releaseConnectedDevicePointer] in a `finally` block.
   *
   * This is the preferred way to interact with a connected device. It ensures the native pointer is
   * always released, preventing the memory leak described in issue #21539.
   */
  suspend fun <T> withConnectedDevice(
    context: Context,
    nodeId: Long,
    block: suspend (devicePointer: Long) -> T
  ): T {
    val devicePointer = getConnectedDevicePointer(context, nodeId)
    return try {
      block(devicePointer)
    } finally {
      getDeviceController(context).releaseConnectedDevicePointer(devicePointer)
    }
  }
}
