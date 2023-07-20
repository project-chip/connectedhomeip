package com.matter.virtual.device.app.core.domain.usecase.network

import com.matter.virtual.device.app.core.common.di.IoDispatcher
import com.matter.virtual.device.app.core.data.repository.NetworkRepository
import com.matter.virtual.device.app.core.domain.NonParamCoroutineUseCase
import javax.inject.Inject
import kotlinx.coroutines.CoroutineDispatcher

class GetSSIDUseCase
@Inject
constructor(
  private val networkRepository: NetworkRepository,
  @IoDispatcher dispatcher: CoroutineDispatcher
) : NonParamCoroutineUseCase<String>(dispatcher) {

  override suspend fun execute(): String {
    return networkRepository.getSSID()
  }
}
