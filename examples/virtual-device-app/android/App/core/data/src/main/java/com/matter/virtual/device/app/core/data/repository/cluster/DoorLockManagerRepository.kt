package com.matter.virtual.device.app.core.data.repository.cluster

import com.matter.virtual.device.app.core.model.matter.LockState
import kotlinx.coroutines.flow.StateFlow

interface DoorLockManagerRepository {
  fun getLockStateFlow(): StateFlow<LockState>

  suspend fun setLockState(lockState: LockState)

  suspend fun sendLockAlarmEvent()
}
