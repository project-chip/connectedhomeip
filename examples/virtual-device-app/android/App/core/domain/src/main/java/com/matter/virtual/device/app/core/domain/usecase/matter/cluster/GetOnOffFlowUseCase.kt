package com.matter.virtual.device.app.core.domain.usecase.matter.cluster

import com.matter.virtual.device.app.core.data.repository.cluster.OnOffManagerRepository
import javax.inject.Inject
import kotlinx.coroutines.flow.StateFlow

class GetOnOffFlowUseCase
@Inject
constructor(private val onOffManagerRepository: OnOffManagerRepository) {
  operator fun invoke(): StateFlow<Boolean> = onOffManagerRepository.getOnOffFlow()
}
