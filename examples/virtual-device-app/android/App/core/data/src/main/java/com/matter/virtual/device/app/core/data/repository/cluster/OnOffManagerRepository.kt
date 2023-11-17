package com.matter.virtual.device.app.core.data.repository.cluster

import kotlinx.coroutines.flow.StateFlow

interface OnOffManagerRepository {
  fun getOnOffFlow(): StateFlow<Boolean>

  suspend fun setOnOff(value: Boolean)
}
