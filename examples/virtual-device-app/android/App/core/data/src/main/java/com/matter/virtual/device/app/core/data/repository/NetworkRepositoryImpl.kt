package com.matter.virtual.device.app.core.data.repository

import android.content.Context
import android.net.wifi.SupplicantState
import android.net.wifi.WifiManager
import dagger.hilt.android.qualifiers.ApplicationContext
import javax.inject.Inject
import timber.log.Timber

internal class NetworkRepositoryImpl
@Inject
constructor(@ApplicationContext private val context: Context) : NetworkRepository {

  override suspend fun getSSID(): String {
    Timber.d("Hit")
    return try {
      val wifiManager = context.getSystemService(Context.WIFI_SERVICE) as WifiManager?
      wifiManager?.let {
        val wifiInfo = wifiManager.connectionInfo
        var ssid = ""
        if (wifiInfo.supplicantState == SupplicantState.COMPLETED) {
          Timber.d("ssid:${wifiInfo.ssid}")
          ssid = wifiInfo.ssid.replace("\"", "")
        }

        ssid
      }
        ?: "Unknown"
    } catch (e: Exception) {
      Timber.e(e, "Exception")
      "Unknown"
    }
  }
}
