package com.matter.virtual.device.app.core.domain.usecase.matter

import com.matter.virtual.device.app.core.common.MatterSettings
import com.matter.virtual.device.app.core.common.di.IoDispatcher
import com.matter.virtual.device.app.core.data.repository.MatterRepository
import com.matter.virtual.device.app.core.domain.CoroutineUseCase
import javax.inject.Inject
import kotlinx.coroutines.CoroutineDispatcher

class StartMatterAppServiceUseCase
@Inject
constructor(
  private val matterRepository: MatterRepository,
  @IoDispatcher dispatcher: CoroutineDispatcher
) : CoroutineUseCase<MatterSettings, Unit>(dispatcher) {

  override suspend fun execute(param: MatterSettings) {
    matterRepository.startMatterAppService(param)
  }
}
