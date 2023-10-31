package com.matter.virtual.device.app.feature.closure

import androidx.lifecycle.SavedStateHandle
import com.matter.virtual.device.app.core.ui.BaseViewModel
import dagger.hilt.android.lifecycle.HiltViewModel
import javax.inject.Inject
import timber.log.Timber

@HiltViewModel
class DoorLockViewModel @Inject constructor(savedStateHandle: SavedStateHandle) :
  BaseViewModel(savedStateHandle) {

  override fun onCleared() {
    Timber.d("onCleared()")
    super.onCleared()
  }
}
