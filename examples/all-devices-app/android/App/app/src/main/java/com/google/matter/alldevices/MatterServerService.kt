package com.google.matter.alldevices

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Context
import android.content.Intent
import android.net.wifi.WifiManager
import android.os.Build
import android.os.IBinder
import android.util.Log
import chip.platform.AndroidChipPlatform
import chip.platform.ChipMdnsCallbackImpl
import chip.platform.DiagnosticDataProviderImpl
import chip.platform.NsdManagerServiceBrowser
import chip.platform.NsdManagerServiceResolver
import chip.platform.PreferencesConfigurationManager
import chip.platform.PreferencesKeyValueStoreManager

/**
 * MatterServerService runs the native C++ Matter event loop and dynamic device simulator engine as
 * an Android Foreground Service.
 *
 * This is necessary because modern Android OS aggressively restricts background processes (via the
 * Cached Apps Freezer). Without a foreground service, the emulator/device OS halts the Matter
 * UDP/TCP event loop threads shortly after the application loses screen focus, causing
 * commissioning and cluster command handshakes to time out or fail.
 */
class MatterServerService : Service() {
  private var androidChipPlatform: AndroidChipPlatform? = null
  private var multicastLock: WifiManager.MulticastLock? = null

  override fun onBind(intent: Intent?): IBinder? {
    return null
  }

  override fun onCreate() {
    super.onCreate()
    val wifi = applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
    multicastLock =
      wifi.createMulticastLock("AllDevicesAppMulticastLock").apply {
        setReferenceCounted(true)
        acquire()
      }
    createNotificationChannel()
  }

  override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
    val discriminator = intent?.getIntExtra("discriminator", 3840) ?: 3840
    val configurationJson = intent?.getStringExtra("configurationJson") ?: "[]"

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      startForeground(NOTIFICATION_ID, createNotification())
    }

    startServerInstance(discriminator, configurationJson)

    return START_NOT_STICKY
  }

  private fun startServerInstance(discriminator: Int, configurationJson: String) {
    try {
      androidChipPlatform =
        AndroidChipPlatform(
          null,
          null,
          PreferencesKeyValueStoreManager(applicationContext),
          PreferencesConfigurationManager(applicationContext),
          NsdManagerServiceResolver(applicationContext),
          NsdManagerServiceBrowser(applicationContext),
          ChipMdnsCallbackImpl(),
          DiagnosticDataProviderImpl(applicationContext)
        )

      androidChipPlatform?.updateCommissionableDataProviderData(
        null,
        null,
        0,
        20202021L,
        discriminator
      )

      val success = App.getInstance().startApp(configurationJson)
      if (!success) {
        Log.e(TAG, "Failed to start Matter app JNI")
      } else {
        Log.i(TAG, "Matter app JNI started successfully in foreground service")
      }
    } catch (e: Exception) {
      Log.e(TAG, "Error starting Matter server: ${e.message}", e)
    }
  }

  override fun onDestroy() {
    super.onDestroy()
    App.getInstance().stopApp()
    multicastLock?.let {
      if (it.isHeld) {
        it.release()
      }
    }
    Log.i(TAG, "Matter app JNI stopped and service destroyed")
  }

  private fun createNotificationChannel() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      val serviceChannel =
        NotificationChannel(
          CHANNEL_ID,
          "Matter Simulator Service Channel",
          NotificationManager.IMPORTANCE_LOW
        )
      val manager = getSystemService(NotificationManager::class.java)
      manager.createNotificationChannel(serviceChannel)
    }
  }

  private fun createNotification(): Notification {
    return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      Notification.Builder(this, CHANNEL_ID)
        .setContentTitle("Matter Simulator App")
        .setContentText("Matter server is running in the background...")
        .setSmallIcon(android.R.drawable.sym_def_app_icon)
        .build()
    } else {
      Notification.Builder(this)
        .setContentTitle("Matter Simulator App")
        .setContentText("Matter server is running...")
        .setSmallIcon(android.R.drawable.sym_def_app_icon)
        .build()
    }
  }

  companion object {
    init {
      System.loadLibrary("AllDevicesApp")
    }

    private const val TAG = "MatterServerService"
    private const val CHANNEL_ID = "MatterServerServiceChannel"
    private const val NOTIFICATION_ID = 1001
  }
}
