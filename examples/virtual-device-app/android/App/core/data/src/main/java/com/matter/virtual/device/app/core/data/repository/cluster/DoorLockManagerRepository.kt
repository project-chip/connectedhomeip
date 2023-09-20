package com.matter.virtual.device.app.core.data.repository.cluster

import kotlinx.coroutines.flow.StateFlow

interface DoorLockManagerRepository {
  fun getLockStateFlow(): StateFlow<Boolean>

  suspend fun setLockState(value: Boolean)

  suspend fun sendLockAlarmEvent()
}
