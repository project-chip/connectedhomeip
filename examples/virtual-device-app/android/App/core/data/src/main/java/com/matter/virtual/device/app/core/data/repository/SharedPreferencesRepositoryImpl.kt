package com.matter.virtual.device.app.core.data.repository

import com.matter.virtual.device.app.core.common.Device
import com.matter.virtual.device.app.core.common.sharedpreferences.SharedPreferencesKey
import com.matter.virtual.device.app.core.common.sharedpreferences.SharedPreferencesManager
import javax.inject.Inject

internal class SharedPreferencesRepositoryImpl
@Inject
constructor(private val sharedPreferencesManager: SharedPreferencesManager) :
  SharedPreferencesRepository {

  override suspend fun isCommissioningDeviceCompleted(): Boolean {
    return sharedPreferencesManager.getBoolean(SharedPreferencesKey.COMMISSIONING_DEVICE_COMPLETED)
  }

  override suspend fun setCommissioningDeviceCompleted(value: Boolean) {
    sharedPreferencesManager.setBoolean(SharedPreferencesKey.COMMISSIONING_DEVICE_COMPLETED, value)
  }

  override suspend fun getCommissionedDevice(): Device {
    return Device.map(sharedPreferencesManager.getString(SharedPreferencesKey.COMMISSIONED_DEVICE))
  }

  override suspend fun setCommissionedDevice(device: Device) {
    sharedPreferencesManager.setString(SharedPreferencesKey.COMMISSIONED_DEVICE, device.title)
  }

  override suspend fun deleteMatterSharedPreferences() {
    sharedPreferencesManager.deleteMatterSharedPreferences()
  }

  override suspend fun setCommissioningSequence(value: Boolean) {
    sharedPreferencesManager.setBoolean(SharedPreferencesKey.COMMISSIONING_SEQUENCE, value)
  }

  override suspend fun isCommissioningSequence(): Boolean {
    return sharedPreferencesManager.getBoolean(SharedPreferencesKey.COMMISSIONING_SEQUENCE)
  }
}
