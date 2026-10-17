package com.google.chip.chiptool.clusterclient

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Context
import android.content.Intent
import android.content.pm.ServiceInfo
import android.os.Build
import android.os.IBinder
import android.util.Log
import com.google.chip.chiptool.R

/**
 * Foreground service that runs while the OTA provider has active BDX transfers.
 *
 * It holds no locks itself. Putting the app process in the foreground-service state is what keeps
 * the wake lock and Wi-Fi lock held by [OtaProviderClientFragment] (same UID) effective in Doze and
 * keeps network access when CHIPTool is not the foreground app.
 */
class OtaTransferService : Service() {
  private var isForeground = false

  override fun onCreate() {
    super.onCreate()
    getSystemService(NotificationManager::class.java)
      .createNotificationChannel(
        NotificationChannel(
          CHANNEL_ID,
          getString(R.string.ota_transfer_channel_name),
          NotificationManager.IMPORTANCE_LOW,
        )
      )
  }

  override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
    when (intent?.action) {
      ACTION_STOP -> {
        // Stopping through onStartCommand (rather than stopService) guarantees startForeground was
        // already called for every earlier startForegroundService request.
        isForeground = false
        stopForeground(STOP_FOREGROUND_REMOVE)
        stopSelfResult(startId)
      }
      ACTION_UPDATE_PROGRESS -> {
        if (isForeground) {
          val activeCount = intent.getIntExtra(EXTRA_ACTIVE_COUNT, 0)
          val progressPercent = intent.getIntExtra(EXTRA_PROGRESS_PERCENT, -1)
          val summaryText = intent.getStringExtra(EXTRA_SUMMARY_TEXT)
          getSystemService(NotificationManager::class.java)
            .notify(NOTIFICATION_ID, buildNotification(activeCount, progressPercent, summaryText))
        } else {
          // Ignore late progress updates queued after ACTION_STOP so the notification is not
          // recreated after the last transfer finishes.
          stopSelfResult(startId)
        }
      }
      else -> {
        isForeground = true
        val activeCount = intent?.getIntExtra(EXTRA_ACTIVE_COUNT, 0) ?: 0
        val progressPercent = intent?.getIntExtra(EXTRA_PROGRESS_PERCENT, -1) ?: -1
        val summaryText = intent?.getStringExtra(EXTRA_SUMMARY_TEXT)
        startInForeground(activeCount, progressPercent, summaryText)
      }
    }
    return START_NOT_STICKY
  }

  override fun onDestroy() {
    isForeground = false
    super.onDestroy()
  }

  override fun onBind(intent: Intent?): IBinder? = null

  private fun buildNotification(
    activeCount: Int,
    progressPercent: Int,
    summaryText: String?,
  ): Notification {
    val builder =
      Notification.Builder(this, CHANNEL_ID)
        .setSmallIcon(android.R.drawable.stat_sys_upload)
        .setContentTitle(getString(R.string.ota_transfer_notification_title))
        .setOngoing(true)
        .setOnlyAlertOnce(true)
    if (!summaryText.isNullOrEmpty()) {
      builder.setContentText(summaryText)
    }
    if (activeCount > 0 && progressPercent >= 0) {
      builder.setProgress(100, progressPercent.coerceIn(0, 100), false)
    }
    return builder.build()
  }

  private fun startInForeground(activeCount: Int, progressPercent: Int, summaryText: String?) {
    val notification = buildNotification(activeCount, progressPercent, summaryText)
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
      startForeground(
        NOTIFICATION_ID,
        notification,
        ServiceInfo.FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE,
      )
    } else {
      startForeground(NOTIFICATION_ID, notification)
    }
  }

  companion object {
    private const val TAG = "OtaTransferService"
    private const val ACTION_START = "com.google.chip.chiptool.action.START_OTA_TRANSFER_SERVICE"
    private const val ACTION_STOP = "com.google.chip.chiptool.action.STOP_OTA_TRANSFER_SERVICE"
    private const val ACTION_UPDATE_PROGRESS =
      "com.google.chip.chiptool.action.UPDATE_OTA_TRANSFER_PROGRESS"
    private const val EXTRA_ACTIVE_COUNT = "extra_active_count"
    private const val EXTRA_PROGRESS_PERCENT = "extra_progress_percent"
    private const val EXTRA_SUMMARY_TEXT = "extra_summary_text"
    private const val CHANNEL_ID = "ota_transfer"
    private const val NOTIFICATION_ID = 0x07A

    /** Starts the foreground service; returns false if disallowed by the platform. */
    fun start(context: Context, summaryText: String? = null): Boolean =
      try {
        val intent =
          Intent(context, OtaTransferService::class.java).apply {
            action = ACTION_START
            if (!summaryText.isNullOrEmpty()) {
              putExtra(EXTRA_SUMMARY_TEXT, summaryText)
            }
          }
        context.startForegroundService(intent)
        true
      } catch (e: Exception) {
        // For example ForegroundServiceStartNotAllowedException when the app is in the background
        // on API 31+. Transfers still run, but without the foreground-service Doze exemption.
        Log.w(TAG, "Could not start OTA transfer foreground service", e)
        false
      }

    /** Updates the ongoing foreground notification progress if the service is active. */
    fun updateProgress(
      context: Context,
      activeCount: Int,
      progressPercent: Int,
      summaryText: String,
    ) {
      val intent =
        Intent(context, OtaTransferService::class.java)
          .setAction(ACTION_UPDATE_PROGRESS)
          .putExtra(EXTRA_ACTIVE_COUNT, activeCount)
          .putExtra(EXTRA_PROGRESS_PERCENT, progressPercent)
          .putExtra(EXTRA_SUMMARY_TEXT, summaryText)
      try {
        context.startService(intent)
      } catch (e: Exception) {
        Log.w(TAG, "Could not update OTA transfer service progress", e)
      }
    }

    /** Requests the OTA transfer foreground service to remove its notification and stop. */
    fun stop(context: Context) {
      val intent = Intent(context, OtaTransferService::class.java).setAction(ACTION_STOP)
      try {
        context.startService(intent)
      } catch (e: Exception) {
        Log.w(TAG, "Could not deliver stop to OTA transfer service, stopping directly", e)
        context.stopService(Intent(context, OtaTransferService::class.java))
      }
    }
  }
}
