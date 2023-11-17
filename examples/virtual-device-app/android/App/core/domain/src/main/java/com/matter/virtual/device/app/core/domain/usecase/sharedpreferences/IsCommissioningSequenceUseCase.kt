package com.matter.virtual.device.app.core.domain.usecase.sharedpreferences

import com.matter.virtual.device.app.core.common.di.IoDispatcher
import com.matter.virtual.device.app.core.data.repository.SharedPreferencesRepository
import com.matter.virtual.device.app.core.domain.NonParamCoroutineUseCase
import javax.inject.Inject
import kotlinx.coroutines.CoroutineDispatcher

class IsCommissioningSequenceUseCase
@Inject
constructor(
  private val sharedPreferencesRepository: SharedPreferencesRepository,
  @IoDispatcher dispatcher: CoroutineDispatcher
) : NonParamCoroutineUseCase<Boolean>(dispatcher) {

  override suspend fun execute(): Boolean {
    return sharedPreferencesRepository.isCommissioningSequence()
  }
}
