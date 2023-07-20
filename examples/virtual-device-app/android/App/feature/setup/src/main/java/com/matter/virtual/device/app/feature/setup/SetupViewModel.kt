package com.matter.virtual.device.app.feature.setup

import androidx.lifecycle.*
import com.matter.virtual.device.app.core.common.successOr
import com.matter.virtual.device.app.core.domain.usecase.network.GetSSIDUseCase
import dagger.hilt.android.lifecycle.HiltViewModel
import javax.inject.Inject
import kotlinx.coroutines.async
import timber.log.Timber

@HiltViewModel
class SetupViewModel @Inject constructor(private val getSSIDUseCase: GetSSIDUseCase) : ViewModel() {

  override fun onCleared() {
    Timber.d("Hit")
    super.onCleared()
  }

  suspend fun getSSID(): String {
    Timber.d("Hit")
    val deferred = viewModelScope.async { getSSIDUseCase().successOr("Unknown") }

    val ssid = deferred.await()
    Timber.d("ssid:${ssid}")
    return ssid
  }
}
