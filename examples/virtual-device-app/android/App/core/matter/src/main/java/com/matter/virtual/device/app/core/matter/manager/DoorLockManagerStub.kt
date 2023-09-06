package com.matter.virtual.device.app.core.matter.manager

import com.matter.virtual.device.app.DeviceApp
import com.matter.virtual.device.app.DoorLockManager
import com.matter.virtual.device.app.core.common.MatterConstants
import javax.inject.Inject
import javax.inject.Singleton
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import timber.log.Timber

@Singleton
class DoorLockManagerStub @Inject constructor(private val deviceApp: DeviceApp) : DoorLockManager {

  private val _lockState = MutableStateFlow(false)
  val lockState: StateFlow<Boolean>
    get() = _lockState

  override fun initAttributeValue() {
    Timber.d("initAttributeValue()")
    deviceApp.setLockType(MatterConstants.DEFAULT_ENDPOINT, DoorLockManager.DlLockType_kMagnetic)
    deviceApp.setLockState(MatterConstants.DEFAULT_ENDPOINT, lockState.value.asLockState())
    deviceApp.setActuatorEnabled(MatterConstants.DEFAULT_ENDPOINT, true)
    deviceApp.setOperatingMode(
      MatterConstants.DEFAULT_ENDPOINT,
      DoorLockManager.OperatingModeEnum_kNormal
    )
    deviceApp.setSupportedOperatingModes(
      MatterConstants.DEFAULT_ENDPOINT,
      DoorLockManager.DlSupportedOperatingModes_kNormal
    )
  }

  override fun handleLockStateChanged(value: Int) {
    Timber.d("handleLockStateChanged():$value")
    _lockState.value = value.asBooleanLockState()
  }

  fun setLockState(value: Boolean) {
    Timber.d("setLockState():$value")
    deviceApp.setLockState(MatterConstants.DEFAULT_ENDPOINT, value.asLockState())
  }

  fun sendLockAlarmEvent() {
    Timber.d("sendLockAlarmEvent()")
    deviceApp.sendLockAlarmEvent(MatterConstants.DEFAULT_ENDPOINT)
  }

  private fun Boolean.asLockState() =
    when (this) {
      true -> DoorLockManager.DlLockState_kUnlocked
      false -> DoorLockManager.DlLockState_kLocked
    }

  private fun Int.asBooleanLockState() =
    when (this) {
      DoorLockManager.DlLockState_kUnlocked -> true
      DoorLockManager.DlLockState_kLocked -> false
      else -> false
    }
}
