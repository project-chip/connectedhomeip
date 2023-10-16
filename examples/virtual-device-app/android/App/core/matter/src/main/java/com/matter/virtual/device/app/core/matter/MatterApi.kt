package com.matter.virtual.device.app.core.matter

import com.matter.virtual.device.app.core.matter.model.asSetupPayload
import com.matter.virtual.device.app.core.model.Payload
import javax.inject.Inject
import javax.inject.Singleton
import matter.onboardingpayload.OnboardingPayloadException
import matter.onboardingpayload.OnboardingPayloadParser
import timber.log.Timber

@Singleton
class MatterApi @Inject constructor() {

  fun getQrcodeString(payload: Payload): String {
    val onboardingPayloadParser = OnboardingPayloadParser()
    var qrcode = ""
    try {
      qrcode = onboardingPayloadParser.getQrCodeFromPayload(payload.asSetupPayload())
    } catch (e: OnboardingPayloadException) {
      e.printStackTrace()
    }

    Timber.d("qrcode:$qrcode")
    return qrcode
  }

  fun getManualPairingCodeString(payload: Payload): String {
    val onboardingPayloadParser = OnboardingPayloadParser()
    var manualPairingCode = ""
    try {
      manualPairingCode =
        onboardingPayloadParser.getManualPairingCodeFromPayload(payload.asSetupPayload())
    } catch (e: OnboardingPayloadException) {
      e.printStackTrace()
    }

    Timber.d("manualPairingCode:$manualPairingCode")
    return manualPairingCode
  }
}
