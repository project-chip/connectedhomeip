package com.matter.virtual.device.app.core.domain.usecase.matter

import com.matter.virtual.device.app.core.common.di.IoDispatcher
import com.matter.virtual.device.app.core.data.repository.MatterRepository
import com.matter.virtual.device.app.core.data.repository.SharedPreferencesRepository
import com.matter.virtual.device.app.core.domain.NonParamCoroutineUseCase
import javax.inject.Inject
import kotlinx.coroutines.CoroutineDispatcher

class ResetMatterAppUseCase
@Inject
constructor(
  private val matterRepository: MatterRepository,
  private val sharedPreferencesRepository: SharedPreferencesRepository,
  @IoDispatcher dispatcher: CoroutineDispatcher
) : NonParamCoroutineUseCase<Unit>(dispatcher) {

  override suspend fun execute() {
    matterRepository.reset()
    matterRepository.stopMatterAppService()
    sharedPreferencesRepository.deleteMatterSharedPreferences()
    sharedPreferencesRepository.setCommissioningDeviceCompleted(false)
    sharedPreferencesRepository.setCommissioningSequence(false)
  }
}
