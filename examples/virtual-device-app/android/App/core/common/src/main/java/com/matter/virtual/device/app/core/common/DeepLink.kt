package com.matter.virtual.device.app.core.common

import androidx.core.net.toUri
import androidx.navigation.NavDeepLinkRequest
import timber.log.Timber

object DeepLink {
  fun getDeepLinkRequestForQrcodeFragment(setting: String): NavDeepLinkRequest {
    return NavDeepLinkRequest.Builder.fromUri(
        "android-app://com.matter.virtual.device.app.feature.qrcode/qrcodeFragment/${setting}"
          .toUri()
      )
      .build()
  }

  fun getDeepLinkRequestForSetupFragment(setting: String): NavDeepLinkRequest {
    return NavDeepLinkRequest.Builder.fromUri(
        "android-app://com.matter.virtual.device.app.feature.setup/setupFragment/${setting}".toUri()
      )
      .build()
  }

  fun getDeepLinkRequestForLoadingFragment(setting: String): NavDeepLinkRequest {
    return NavDeepLinkRequest.Builder.fromUri(
        "android-app://com.matter.virtual.device.app.feature.main/loadingFragment/${setting}"
          .toUri()
      )
      .build()
  }

  fun getDeepLinkRequestFromDevice(device: Device, setting: String): NavDeepLinkRequest {
    Timber.d("setting:$setting")
    val uri =
      when (device) {
        Device.OnOffSwitch ->
          "android-app://com.matter.virtual.device.app.feature.control/onOffSwitchFragment/${setting}"
            .toUri()
        Device.DoorLock ->
          "android-app://com.matter.virtual.device.app.feature.closure/doorLockFragment/${setting}"
            .toUri()
        Device.Unknown -> throw UnsupportedOperationException("Unsupported device")
      }

    return NavDeepLinkRequest.Builder.fromUri(uri).build()
  }
}
