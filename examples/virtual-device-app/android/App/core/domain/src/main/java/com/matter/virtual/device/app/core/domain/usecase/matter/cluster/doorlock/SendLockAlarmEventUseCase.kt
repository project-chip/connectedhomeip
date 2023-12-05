package com.matter.virtual.device.app.core.domain.usecase.matter.cluster.doorlock

import com.matter.virtual.device.app.core.common.di.IoDispatcher
import com.matter.virtual.device.app.core.data.repository.cluster.DoorLockManagerRepository
import com.matter.virtual.device.app.core.domain.NonParamCoroutineUseCase
import javax.inject.Inject
import kotlinx.coroutines.CoroutineDispatcher

class SendLockAlarmEventUseCase
@Inject
constructor(
  private val doorLockManagerRepository: DoorLockManagerRepository,
  @IoDispatcher dispatcher: CoroutineDispatcher
) : NonParamCoroutineUseCase<Unit>(dispatcher) {

  override suspend fun execute() {
    return doorLockManagerRepository.sendLockAlarmEvent()
  }
}
