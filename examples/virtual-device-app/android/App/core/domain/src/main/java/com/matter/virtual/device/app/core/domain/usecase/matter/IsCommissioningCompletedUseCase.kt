package com.matter.virtual.device.app.core.domain.usecase.matter

import com.matter.virtual.device.app.core.common.di.IoDispatcher
import com.matter.virtual.device.app.core.data.repository.MatterRepository
import com.matter.virtual.device.app.core.domain.NonParamCoroutineUseCase
import javax.inject.Inject
import kotlinx.coroutines.CoroutineDispatcher
import timber.log.Timber

class IsCommissioningCompletedUseCase
@Inject
constructor(
  private val matterRepository: MatterRepository,
  @IoDispatcher dispatcher: CoroutineDispatcher
) : NonParamCoroutineUseCase<Boolean>(dispatcher) {

  override suspend fun execute(): Boolean {
    var result = false

    runCatching { result = matterRepository.isCommissioningCompleted() }
      .onFailure {
        Timber.e(it, "error")
        result = false
      }

    return result
  }
}
