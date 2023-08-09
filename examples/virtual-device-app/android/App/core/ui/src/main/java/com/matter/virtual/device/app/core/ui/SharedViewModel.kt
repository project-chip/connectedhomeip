package com.matter.virtual.device.app.core.ui

import androidx.annotation.StringRes
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.matter.virtual.device.app.core.common.Event
import com.matter.virtual.device.app.core.domain.usecase.matter.ResetMatterAppUseCase
import dagger.hilt.android.lifecycle.HiltViewModel
import javax.inject.Inject
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import timber.log.Timber

@HiltViewModel
class SharedViewModel
@Inject
constructor(private val resetMatterAppUseCase: ResetMatterAppUseCase) : ViewModel() {

  private val _uiState = MutableLiveData<Event<UiState>>()
  val uiState: LiveData<Event<UiState>>
    get() = _uiState

  override fun onCleared() {
    Timber.d("onCleared()")
    super.onCleared()
  }

  fun requestFactoryReset(@StringRes messageResId: Int, isCancelable: Boolean) {
    Timber.d("Hit")
    viewModelScope.launch { _uiState.value = Event(UiState.Reset(messageResId, isCancelable)) }
  }

  fun resetMatterAppServer() {
    Timber.d("Hit")
    viewModelScope.launch {
      _uiState.value = Event(UiState.Waiting)
      resetMatterAppUseCase()
      delay(1500)
      _uiState.value = Event(UiState.Exit)
    }
  }
}

sealed class UiState {
  data class Reset(@StringRes val messageResId: Int, val isCancelable: Boolean) : UiState()

  object Waiting : UiState()

  object Exit : UiState()
}
