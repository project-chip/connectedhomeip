package com.matter.virtual.device.app.core.data.repository.cluster

import kotlinx.coroutines.flow.StateFlow

interface PowerSourceManagerRepository {
  fun getBatPercent(): StateFlow<Int>

  suspend fun setBatPercentRemaining(batteryPercentRemaining: Int)
}
