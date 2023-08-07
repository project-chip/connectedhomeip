package com.matter.virtual.device.app.core.domain.usecase.sharedpreferences

import com.matter.virtual.device.app.core.common.Device
import com.matter.virtual.device.app.core.common.di.IoDispatcher
import com.matter.virtual.device.app.core.data.repository.SharedPreferencesRepository
import com.matter.virtual.device.app.core.domain.NonParamCoroutineUseCase
import javax.inject.Inject
import kotlinx.coroutines.CoroutineDispatcher

class GetCommissionedDeviceUseCase
@Inject
constructor(
  private val sharedPreferencesRepository: SharedPreferencesRepository,
  @IoDispatcher dispatcher: CoroutineDispatcher
) : NonParamCoroutineUseCase<Device>(dispatcher) {

  override suspend fun execute(): Device {
    return sharedPreferencesRepository.getCommissionedDevice()
  }
}
