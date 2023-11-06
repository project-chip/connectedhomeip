package com.matter.virtual.device.app.feature.main

import androidx.lifecycle.*
import com.matter.virtual.device.app.core.common.Device
import com.matter.virtual.device.app.core.common.Event
import com.matter.virtual.device.app.core.common.MatterSettings
import com.matter.virtual.device.app.core.common.successOr
import com.matter.virtual.device.app.core.domain.usecase.matter.IsCommissioningCompletedUseCase
import com.matter.virtual.device.app.core.domain.usecase.sharedpreferences.SetCommissionedDeviceUseCase
import com.matter.virtual.device.app.core.domain.usecase.sharedpreferences.SetCommissioningDeviceCompletedUseCase
import com.matter.virtual.device.app.core.domain.usecase.sharedpreferences.SetCommissioningSequenceFlagUseCase
import dagger.hilt.android.lifecycle.HiltViewModel
import javax.inject.Inject
import kotlinx.coroutines.launch
import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.json.Json
import timber.log.Timber

@OptIn(ExperimentalSerializationApi::class)
@HiltViewModel
class LoadingViewModel
@Inject
constructor(
  private val isCommissioningCompletedUseCase: IsCommissioningCompletedUseCase,
  private val setCommissioningDeviceCompletedUseCase: SetCommissioningDeviceCompletedUseCase,
  private val setCommissionedDeviceUseCase: SetCommissionedDeviceUseCase,
  private val setCommissioningSequenceFlagUseCase: SetCommissioningSequenceFlagUseCase,
  savedStateHandle: SavedStateHandle
) : ViewModel() {

  private val _uiState = MutableLiveData<Event<LoadingUiState>>(Event(LoadingUiState.Loading))
  val uiState: LiveData<Event<LoadingUiState>>
    get() = _uiState

  init {
    viewModelScope.launch {
      val jsonSettings = savedStateHandle.get<String>("setting")
      jsonSettings?.let {
        val matterSettings = Json.decodeFromString<MatterSettings>(it)
        Timber.d("device:${matterSettings.device.title}")

        val isCommissioningCompleted = isCommissioningCompletedUseCase().successOr(false)
        if (isCommissioningCompleted) {
          Timber.d("Commissioning Completed")
          setCommissioningDeviceCompletedUseCase(true)
          setCommissioningSequenceFlagUseCase(false)
          setCommissionedDeviceUseCase(matterSettings.device)
          _uiState.value = Event(LoadingUiState.Complete(matterSettings.device))
        } else {
          Timber.e("Commissioning Failure or Timeout")
          _uiState.value = Event(LoadingUiState.Failure)
        }
      }
    }
  }

  override fun onCleared() {
    Timber.d("onCleared()")
    super.onCleared()
  }
}

sealed class LoadingUiState {
  object Loading : LoadingUiState()

  data class Complete(val device: Device) : LoadingUiState()

  object Failure : LoadingUiState()
}
