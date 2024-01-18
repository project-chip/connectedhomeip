package com.matter.virtual.device.app.core.data.repository

import com.matter.virtual.device.app.core.common.Device

interface SharedPreferencesRepository {
  suspend fun isCommissioningDeviceCompleted(): Boolean

  suspend fun setCommissioningDeviceCompleted(value: Boolean)

  suspend fun getCommissionedDevice(): Device

  suspend fun setCommissionedDevice(device: Device)

  suspend fun deleteMatterSharedPreferences()

  suspend fun setCommissioningSequence(value: Boolean)

  suspend fun isCommissioningSequence(): Boolean
}
