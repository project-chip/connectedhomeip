package com.matter.virtual.device.app.core.domain.usecase.matter

import com.matter.virtual.device.app.core.common.di.IoDispatcher
import com.matter.virtual.device.app.core.data.repository.MatterRepository
import com.matter.virtual.device.app.core.domain.CoroutineUseCase
import com.matter.virtual.device.app.core.model.Payload
import javax.inject.Inject
import kotlinx.coroutines.CoroutineDispatcher

class GetManualPairingCodeStringUseCase
@Inject
constructor(
  private val matterRepository: MatterRepository,
  @IoDispatcher dispatcher: CoroutineDispatcher
) : CoroutineUseCase<Payload, String>(dispatcher) {

  override suspend fun execute(param: Payload): String {
    return matterRepository.getManualPairingCodeString(param)
  }
}
