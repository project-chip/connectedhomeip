package com.matter.virtual.device.app.core.common

import androidx.core.net.toUri
import androidx.navigation.NavDeepLinkRequest

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
}
