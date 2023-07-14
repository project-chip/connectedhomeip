package com.samsung.matter.chipstdeviceapp.core.domain.usecase.matter

import com.samsung.matter.chipstdeviceapp.core.common.di.IoDispatcher
import com.samsung.matter.chipstdeviceapp.core.data.repository.MatterRepository
import com.samsung.matter.chipstdeviceapp.core.domain.CoroutineUseCase
import com.samsung.matter.chipstdeviceapp.core.model.Payload
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
