package com.samsung.matter.chipstdeviceapp.core.domain.usecase.network

import com.samsung.matter.chipstdeviceapp.core.common.di.IoDispatcher
import com.samsung.matter.chipstdeviceapp.core.data.repository.NetworkRepository
import com.samsung.matter.chipstdeviceapp.core.domain.NonParamCoroutineUseCase
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
