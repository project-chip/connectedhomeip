package com.matter.virtual.device.app.core.data.repository

import android.app.ActivityManager
import android.content.Context
import android.content.Intent
import android.os.Build
import com.matter.virtual.device.app.core.common.MatterSettings
import com.matter.virtual.device.app.core.matter.*
import com.matter.virtual.device.app.core.model.Payload
import dagger.hilt.android.qualifiers.ApplicationContext
import javax.inject.Inject
import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import timber.log.Timber

internal class MatterRepositoryImpl
@Inject
constructor(
  @ApplicationContext private val context: Context,
  private val matterApi: MatterApi,
  private val matterApp: MatterApp
) : MatterRepository {

  override fun getQrcodeString(payload: Payload): String {
    return matterApi.getQrcodeString(payload)
  }

  override fun getManualPairingCodeString(payload: Payload): String {
    return matterApi.getManualPairingCodeString(payload)
  }

  @OptIn(ExperimentalSerializationApi::class)
  override suspend fun startMatterAppService(matterSettings: MatterSettings) {
    Timber.d("startMatterAppService():$matterSettings")

    if (!isMatterAppServiceRunning()) {
      val intent =
        Intent(context, MatterAppService::class.java).apply {
          this.action = MatterAppServiceConstants.ACTION_START_MATTER_APP_SERVICE
          val jsonSettings = Json.encodeToString(matterSettings)
          this.putExtra("setting", jsonSettings)
        }

      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
        context.startForegroundService(intent)
      } else {
        context.startService(intent)
      }
    }
  }

  override suspend fun stopMatterAppService() {
    Timber.d("stopMatterAppService()")

    if (isMatterAppServiceRunning()) {
      val intent =
        Intent(context, MatterAppService::class.java).apply {
          this.action = MatterAppServiceConstants.ACTION_STOP_MATTER_APP_SERVICE
        }

      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
        context.startForegroundService(intent)
      } else {
        context.startService(intent)
      }
    }
  }

  private fun isMatterAppServiceRunning(): Boolean {
    val activityManager = context.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager?
    activityManager?.let {
      it.getRunningServices(Integer.MAX_VALUE).forEach { serviceInfo ->
        Timber.d("${serviceInfo.service.className}:${MatterAppService::class.java.name}")
        if (serviceInfo.service.className == MatterAppService::class.java.name) {
          Timber.d("true")
          return true
        }
      }
    }

    Timber.d("false")
    return false
  }  
}
