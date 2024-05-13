package com.matter.virtual.device.app.core.domain.usecase.matter.cluster.doorlock

import com.matter.virtual.device.app.core.data.repository.cluster.DoorLockManagerRepository
import com.matter.virtual.device.app.core.model.matter.LockState
import javax.inject.Inject
import kotlinx.coroutines.flow.StateFlow

class GetLockStateFlowUseCase
@Inject
constructor(private val doorLockManagerRepository: DoorLockManagerRepository) {

  operator fun invoke(): StateFlow<LockState> = doorLockManagerRepository.getLockStateFlow()
}
