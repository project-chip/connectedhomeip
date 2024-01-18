package com.matter.virtual.device.app.core.data.repository.cluster

import com.matter.virtual.device.app.core.matter.manager.OnOffManagerStub
import javax.inject.Inject
import kotlinx.coroutines.flow.StateFlow
import timber.log.Timber

internal class OnOffManagerRepositoryImpl
@Inject
constructor(private val onOffManagerStub: OnOffManagerStub) : OnOffManagerRepository {

  override fun getOnOffFlow(): StateFlow<Boolean> {
    Timber.d("Hit")
    return onOffManagerStub.onOff
  }

  override suspend fun setOnOff(value: Boolean) {
    Timber.d("value:$value")
    onOffManagerStub.setOnOff(value)
  }
}
