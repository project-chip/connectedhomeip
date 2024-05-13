package com.matter.virtual.device.app.core.domain.usecase.matter.cluster.powersource

import com.matter.virtual.device.app.core.common.di.IoDispatcher
import com.matter.virtual.device.app.core.data.repository.cluster.PowerSourceManagerRepository
import com.matter.virtual.device.app.core.domain.CoroutineUseCase
import javax.inject.Inject
import kotlinx.coroutines.CoroutineDispatcher

class SetBatPercentRemainingUseCase
@Inject
constructor(
  private val powerSourceManagerRepository: PowerSourceManagerRepository,
  @IoDispatcher dispatcher: CoroutineDispatcher
) : CoroutineUseCase<Int, Unit>(dispatcher) {

  override suspend fun execute(param: Int) {
    powerSourceManagerRepository.setBatPercentRemaining(param)
  }
}
