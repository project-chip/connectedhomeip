package com.matter.virtual.device.app.core.matter.manager

import com.matter.virtual.device.app.DeviceApp
import com.matter.virtual.device.app.DoorLockManager
import com.matter.virtual.device.app.core.common.MatterConstants
import com.matter.virtual.device.app.core.model.matter.LockState
import javax.inject.Inject
import javax.inject.Singleton
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import timber.log.Timber

@Singleton
class DoorLockManagerStub @Inject constructor(private val deviceApp: DeviceApp) : DoorLockManager {

  private val _lockState = MutableStateFlow(LockState.LOCKED)
  val lockState: StateFlow<LockState>
    get() = _lockState

  override fun initAttributeValue() {
    Timber.d("initAttributeValue()")
    deviceApp.setLockType(MatterConstants.DEFAULT_ENDPOINT, DoorLockManager.DlLockType_kMagnetic)
    deviceApp.setLockState(MatterConstants.DEFAULT_ENDPOINT, DoorLockManager.DlLockState_kLocked)
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
    _lockState.value = value.asLockStateEnum()
  }

  fun setLockState(lockState: LockState) {
    Timber.d("setLockState():$lockState")
    deviceApp.setLockState(MatterConstants.DEFAULT_ENDPOINT, lockState.asLockState())
  }

  fun sendLockAlarmEvent() {
    Timber.d("sendLockAlarmEvent()")
    deviceApp.sendLockAlarmEvent(MatterConstants.DEFAULT_ENDPOINT)
  }

  private fun LockState.asLockState() =
    when (this) {
      LockState.NOT_FULLY_LOCKED -> DoorLockManager.DlLockState_kNotFullyLocked
      LockState.LOCKED -> DoorLockManager.DlLockState_kLocked
      LockState.UNLOCKED -> DoorLockManager.DlLockState_kUnlocked
      LockState.UNLATCHED -> DoorLockManager.DlLockState_kUnlatched
      else -> DoorLockManager.DlLockState_kUnknownEnumValue
    }

  private fun Int.asLockStateEnum() =
    when (this) {
      DoorLockManager.DlLockState_kNotFullyLocked -> LockState.NOT_FULLY_LOCKED
      DoorLockManager.DlLockState_kLocked -> LockState.LOCKED
      DoorLockManager.DlLockState_kUnlocked -> LockState.UNLOCKED
      DoorLockManager.DlLockState_kUnlatched -> LockState.UNLATCHED
      else -> LockState.UNKNOWN_ENUM_VALUE
    }
}
