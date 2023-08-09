package com.matter.virtual.device.app.core.matter

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.os.Build
import android.os.IBinder
import androidx.core.app.NotificationCompat
import com.matter.virtual.device.app.core.common.MatterSettings
import dagger.hilt.android.AndroidEntryPoint
import javax.inject.Inject
import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.json.Json
import timber.log.Timber

@AndroidEntryPoint
class MatterAppService : Service() {

  @Inject lateinit var matterApp: MatterApp

  override fun onCreate() {
    super.onCreate()
    Timber.d("onCreate()")
  }

  override fun onDestroy() {
    super.onDestroy()
    Timber.d("onDestroy()")
  }

  override fun onBind(intent: Intent?): IBinder? {
    Timber.d("onBind()")
    return null
  }

  @OptIn(ExperimentalSerializationApi::class)
  override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
    Timber.d("onStartCommand()")

    intent?.let {
      when (it.action) {
        MatterAppServiceConstants.ACTION_START_MATTER_APP_SERVICE -> {
          Timber.i("Start matter app service")
          startService()
          val jsonSetting = it.getStringExtra("setting")
          jsonSetting?.let {
            val matterSettings = Json.decodeFromString<MatterSettings>(jsonSetting)
            startApp(matterSettings)
          }
        }
        MatterAppServiceConstants.ACTION_STOP_MATTER_APP_SERVICE -> {
          Timber.i("Stop matter app service")
          stopApp()
          stopService()
        }
        else -> {}
      }
    }

    return START_REDELIVER_INTENT
  }

  private fun createNotificationChannel() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      val notificationChannel =
        NotificationChannel(
          MatterAppServiceConstants.NOTIFICATION_CHANNEL_ID,
          MatterAppServiceConstants.NOTIFICATION_CHANNEL_NAME,
          NotificationManager.IMPORTANCE_DEFAULT
        )

      val manager = getSystemService(NotificationManager::class.java)
      manager?.createNotificationChannel(notificationChannel)
    }
  }

  private fun cancelNotificationChannel() {
    val manager = getSystemService(NotificationManager::class.java)
    manager?.cancel(MatterAppServiceConstants.NOTIFICATION_FOREGROUND_ID)
  }

  private fun startService() {
    Timber.d("startService()")

    createNotificationChannel()

    val notification: Notification =
      NotificationCompat.Builder(this, MatterAppServiceConstants.NOTIFICATION_CHANNEL_ID)
        .setContentTitle("MatterApp Service")
        .setContentText("MatterApp is running")
        .setSmallIcon(R.mipmap.ic_launcher)
        .build()

    startForeground(MatterAppServiceConstants.NOTIFICATION_FOREGROUND_ID, notification)
  }

  private fun stopService() {
    Timber.d("stopService()")
    cancelNotificationChannel()
    stopForeground(true)
    stopSelf()
  }

  private fun startApp(matterSettings: MatterSettings) {
    Timber.d("startApp()")
    matterApp.start(matterSettings)
  }

  private fun stopApp() {
    Timber.d("stopApp()")
    matterApp.stop()
  }
}
