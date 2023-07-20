package com.matter.virtual.device.app.core.data.repository

import com.matter.virtual.device.app.core.matter.MatterApi
import com.matter.virtual.device.app.core.model.Payload
import javax.inject.Inject

internal class MatterRepositoryImpl @Inject constructor(private val matterApi: MatterApi) :
  MatterRepository {

  override fun getQrcodeString(payload: Payload): String {
    return matterApi.getQrcodeString(payload)
  }

  override fun getManualPairingCodeString(payload: Payload): String {
    return matterApi.getManualPairingCodeString(payload)
  }
}
