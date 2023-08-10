package com.matter.virtual.device.app.core.domain.usecase.matter

import com.matter.virtual.device.app.core.common.di.IoDispatcher
import com.matter.virtual.device.app.core.data.repository.MatterRepository
import com.matter.virtual.device.app.core.domain.NonParamCoroutineUseCase
import javax.inject.Inject
import kotlinx.coroutines.CoroutineDispatcher

class IsFabricRemovedUseCase
@Inject
constructor(
  private val matterRepository: MatterRepository,
  @IoDispatcher dispatcher: CoroutineDispatcher
) : NonParamCoroutineUseCase<Boolean>(dispatcher) {

  override suspend fun execute(): Boolean {
    return matterRepository.isFabricRemoved()
  }
}
