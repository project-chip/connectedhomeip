package com.matter.virtual.device.app.core.data.repository

import com.matter.virtual.device.app.core.model.Payload

interface MatterRepository {
  fun getQrcodeString(payload: Payload): String

  fun getManualPairingCodeString(payload: Payload): String
}
