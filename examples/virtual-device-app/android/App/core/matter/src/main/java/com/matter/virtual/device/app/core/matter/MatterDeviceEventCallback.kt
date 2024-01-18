package com.matter.virtual.device.app.core.matter

import timber.log.Timber

interface MatterDeviceEventCallback {
  fun onCommissioningCompleted() = Timber.d("onCommissioningCompleted")

  fun onCommissioningSessionEstablishmentStarted() =
    Timber.d("onCommissioningSessionEstablishmentStarted")

  fun onCommissioningSessionEstablishmentError(errorCode: Int) =
    Timber.e("onCommissioningSessionEstablishmentError:$errorCode")

  fun onFabricRemoved() = Timber.d("onFabricRemoved")
}
