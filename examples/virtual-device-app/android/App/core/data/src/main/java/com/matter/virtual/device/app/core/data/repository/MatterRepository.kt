package com.matter.virtual.device.app.core.data.repository

import com.matter.virtual.device.app.core.common.MatterSettings
import com.matter.virtual.device.app.core.model.Payload

interface MatterRepository {
  fun getQrcodeString(payload: Payload): String

  fun getManualPairingCodeString(payload: Payload): String

  suspend fun startMatterAppService(matterSettings: MatterSettings)

  suspend fun stopMatterAppService()

  fun reset()

  suspend fun isCommissioningCompleted(): Boolean

  suspend fun isCommissioningSessionEstablishmentStarted(): Boolean

  suspend fun isFabricRemoved(): Boolean
}
