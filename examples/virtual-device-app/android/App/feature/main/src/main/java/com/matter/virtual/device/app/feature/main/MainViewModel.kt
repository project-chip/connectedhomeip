package com.matter.virtual.device.app.feature.main

import androidx.annotation.StringRes
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.matter.virtual.device.app.core.common.Device
import com.matter.virtual.device.app.core.common.successOr
import com.matter.virtual.device.app.core.domain.usecase.sharedpreferences.GetCommissionedDeviceUseCase
import com.matter.virtual.device.app.core.domain.usecase.sharedpreferences.IsCommissioningDeviceCompletedUseCase
import com.matter.virtual.device.app.core.domain.usecase.sharedpreferences.IsCommissioningSequenceUseCase
import dagger.hilt.android.lifecycle.HiltViewModel
import javax.inject.Inject
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch
import timber.log.Timber

@HiltViewModel
class MainViewModel
@Inject
constructor(
  private val isCommissioningDeviceCompletedUseCase: IsCommissioningDeviceCompletedUseCase,
  private val getCommissionedDeviceUseCase: GetCommissionedDeviceUseCase,
  private val isCommissioningSequenceUseCase: IsCommissioningSequenceUseCase,
) : ViewModel() {

  private val _uiState = MutableLiveData<MainUiState>(MainUiState.Loading)
  val uiState: LiveData<MainUiState>
    get() = _uiState

  init {
    viewModelScope.launch {
      val isCommissioningSequence = isCommissioningSequenceUseCase().successOr(false)
      if (isCommissioningSequence) {
        Timber.e("Need factory reset")
        _uiState.value =
          MainUiState.Reset(
            messageResId = R.string.dialog_force_factory_reset_message,
            isCancelable = false
          )
        this.cancel()
        return@launch
      }

      val isCommissioningDeviceCompleted = isCommissioningDeviceCompletedUseCase().successOr(false)
      val commissionedDevice = getCommissionedDeviceUseCase().successOr(Device.Unknown)

      if (isCommissioningDeviceCompleted) {
        Timber.d("Go to commissioned device")
        _uiState.value = MainUiState.CommissioningCompleted(commissionedDevice)
      } else {
        Timber.d("Start commissioning")
        _uiState.value = MainUiState.Start
      }
    }
  }

  override fun onCleared() {
    Timber.d("onCleared()")
    super.onCleared()
  }

  fun consumeUiState() {
    Timber.d("consumeUiState()")
    viewModelScope.launch { _uiState.value = MainUiState.Start }
  }
}

sealed class MainUiState {
  object Loading : MainUiState()

  object Start : MainUiState()

  data class CommissioningCompleted(val device: Device) : MainUiState()

  data class Reset(@StringRes val messageResId: Int, val isCancelable: Boolean) : MainUiState()
}
