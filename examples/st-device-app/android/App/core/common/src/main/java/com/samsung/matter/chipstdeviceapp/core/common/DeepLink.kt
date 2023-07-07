package com.samsung.matter.chipstdeviceapp.core.common

import androidx.core.net.toUri
import androidx.navigation.NavDeepLinkRequest

object DeepLink {
    fun getDeepLinkRequestForQrcodeFragment(setting: String): NavDeepLinkRequest {
        return NavDeepLinkRequest.Builder
            .fromUri("android-app://com.samsung.matter.chipstdeviceapp.feature.qrcode/qrcodeFragment/${setting}".toUri())
            .build()
    }

    fun getDeepLinkRequestForSetupFragment(setting: String): NavDeepLinkRequest {
        return NavDeepLinkRequest.Builder
            .fromUri("android-app://com.samsung.matter.chipstdeviceapp.feature.setup/setupFragment/${setting}".toUri())
            .build()
    }
}