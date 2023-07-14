package com.samsung.matter.chipstdeviceapp.core.data.repository

import com.samsung.matter.chipstdeviceapp.core.matter.MatterApi
import com.samsung.matter.chipstdeviceapp.core.model.Payload
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
