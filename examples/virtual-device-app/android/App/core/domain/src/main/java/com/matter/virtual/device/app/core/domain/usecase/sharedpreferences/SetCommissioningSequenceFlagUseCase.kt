package com.matter.virtual.device.app.core.domain.usecase.sharedpreferences

import com.matter.virtual.device.app.core.common.di.MainImmediateDispatcher
import com.matter.virtual.device.app.core.data.repository.SharedPreferencesRepository
import com.matter.virtual.device.app.core.domain.CoroutineUseCase
import javax.inject.Inject
import kotlinx.coroutines.CoroutineDispatcher

class SetCommissioningSequenceFlagUseCase
@Inject
constructor(
  private val sharedPreferencesRepository: SharedPreferencesRepository,
  @MainImmediateDispatcher dispatcher: CoroutineDispatcher
) : CoroutineUseCase<Boolean, Unit>(dispatcher) {

  override suspend fun execute(param: Boolean) {
    sharedPreferencesRepository.setCommissioningSequence(param)
  }
}
