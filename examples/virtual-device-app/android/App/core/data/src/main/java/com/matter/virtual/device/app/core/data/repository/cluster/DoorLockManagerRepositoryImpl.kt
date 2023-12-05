package com.matter.virtual.device.app.core.data.repository.cluster

import com.matter.virtual.device.app.core.matter.manager.DoorLockManagerStub
import com.matter.virtual.device.app.core.model.matter.LockState
import javax.inject.Inject
import kotlinx.coroutines.flow.StateFlow
import timber.log.Timber

internal class DoorLockManagerRepositoryImpl
@Inject
constructor(private val doorLockManagerStub: DoorLockManagerStub) : DoorLockManagerRepository {

  override fun getLockStateFlow(): StateFlow<LockState> {
    return doorLockManagerStub.lockState
  }

  override suspend fun setLockState(lockState: LockState) {
    Timber.d("setLockState():$lockState")
    doorLockManagerStub.setLockState(lockState)
  }

  override suspend fun sendLockAlarmEvent() {
    doorLockManagerStub.sendLockAlarmEvent()
  }
}
