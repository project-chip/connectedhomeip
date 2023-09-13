package com.matter.virtual.device.app.core.data.repository.cluster

import com.matter.virtual.device.app.core.matter.manager.PowerSourceManagerStub
import javax.inject.Inject
import kotlinx.coroutines.flow.StateFlow

internal class PowerSourceManagerRepositoryImpl
@Inject
constructor(private val powerSourceManagerStub: PowerSourceManagerStub) :
  PowerSourceManagerRepository {

  override fun getBatPercent(): StateFlow<Int> {
    return powerSourceManagerStub.batPercent
  }

  override suspend fun setBatPercentRemaining(batteryPercentRemaining: Int) {
    powerSourceManagerStub.setBatPercentRemaining(batteryPercentRemaining)
  }
}
