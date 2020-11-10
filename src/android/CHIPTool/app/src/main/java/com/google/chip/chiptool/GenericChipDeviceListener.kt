package com.google.chip.chiptool

import chip.devicecontroller.ChipDeviceController

open class GenericChipDeviceListener : ChipDeviceController.CompletionListener {
  override fun onConnectDeviceComplete() {
    // No op
  }

  override fun onSendMessageComplete(message: String?) {
    // No op
  }

  override fun onNetworkCredentialsRequested() {
    // No op
  }

  override fun onOperationalCredentialsRequested(opCreds: ByteArray?) {
    // No op
  }

  override fun onStatusUpdate(status: Int) {
    // No op
  }

  override fun onPairingComplete(code: Int) {
    // No op
  }

  override fun onPairingDeleted(code: Int) {
    // No op
  }

  override fun onNotifyChipConnectionClosed() {
    // No op
  }

  override fun onCloseBleComplete() {
    // No op
  }

  override fun onError(error: Throwable?) {
    // No op
  }
}
