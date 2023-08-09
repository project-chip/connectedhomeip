package com.matter.virtual.device.app.core.matter

import chip.appserver.ChipAppServer
import chip.appserver.ChipAppServerDelegate
import javax.inject.Inject
import javax.inject.Singleton
import timber.log.Timber

@Singleton
class MatterApp @Inject constructor() {

  private var chipAppServer: ChipAppServer? = null

  fun start() {
    chipAppServer = ChipAppServer()
    chipAppServer?.startAppWithDelegate(
      object : ChipAppServerDelegate {
        override fun onCommissioningSessionEstablishmentStarted() {
          Timber.d("onCommissioningSessionEstablishmentStarted()")
        }

        override fun onCommissioningSessionStarted() {
          Timber.d("onCommissioningSessionStarted()")
        }

        override fun onCommissioningSessionEstablishmentError(errorCode: Int) {
          Timber.d("onCommissioningSessionEstablishmentError():$errorCode")
        }

        override fun onCommissioningSessionStopped() {
          Timber.d("onCommissioningSessionStopped()")
        }

        override fun onCommissioningWindowOpened() {
          Timber.d("onCommissioningWindowOpened()")
        }

        override fun onCommissioningWindowClosed() {
          Timber.d("onCommissioningWindowClosed()")
        }
      }
    )
  }

  fun stop() {
    chipAppServer?.stopApp()
  }

  fun reset() {
    chipAppServer?.resetApp()
  }
}
