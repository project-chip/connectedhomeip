package com.matter.virtual.device.app.core.data.repository.cluster

import com.matter.virtual.device.app.core.matter.manager.DoorLockManagerStub
import javax.inject.Inject
import kotlinx.coroutines.flow.StateFlow
import timber.log.Timber

internal class DoorLockManagerRepositoryImpl
@Inject
constructor(private val doorLockManagerStub: DoorLockManagerStub) : DoorLockManagerRepository {

  override fun getLockStateFlow(): StateFlow<Boolean> {
    return doorLockManagerStub.lockState
  }

  override suspend fun setLockState(value: Boolean) {
    Timber.d("setLockState():$value")
    doorLockManagerStub.setLockState(value)
  }

  override suspend fun sendLockAlarmEvent() {
    doorLockManagerStub.sendLockAlarmEvent()
  }
}
