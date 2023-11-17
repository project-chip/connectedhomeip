package com.matter.virtual.device.app.core.domain.usecase.matter.cluster

import com.matter.virtual.device.app.core.common.di.IoDispatcher
import com.matter.virtual.device.app.core.data.repository.cluster.OnOffManagerRepository
import com.matter.virtual.device.app.core.domain.CoroutineUseCase
import javax.inject.Inject
import kotlinx.coroutines.CoroutineDispatcher

class SetOnOffUseCase
@Inject
constructor(
  private val onOffManagerRepository: OnOffManagerRepository,
  @IoDispatcher dispatcher: CoroutineDispatcher
) : CoroutineUseCase<Boolean, Unit>(dispatcher) {

  override suspend fun execute(param: Boolean) {
    onOffManagerRepository.setOnOff(param)
  }
}
