package com.matter.virtual.device.app.core.domain.usecase.matter.cluster.powersource

import com.matter.virtual.device.app.core.data.repository.cluster.PowerSourceManagerRepository
import javax.inject.Inject
import kotlinx.coroutines.flow.StateFlow

class GetBatPercentRemainingUseCase
@Inject
constructor(
  private val powerSourceManagerRepository: PowerSourceManagerRepository,
) {
  operator fun invoke(): StateFlow<Int> = powerSourceManagerRepository.getBatPercent()
}
