package com.matter.virtual.device.app.core.ui

import androidx.lifecycle.*
import com.matter.virtual.device.app.core.common.Event
import com.matter.virtual.device.app.core.common.MatterSettings
import kotlinx.coroutines.launch
import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.json.Json
import timber.log.Timber

@OptIn(ExperimentalSerializationApi::class)
abstract class BaseViewModel constructor(savedStateHandle: SavedStateHandle) : ViewModel() {

  private val _onFabricRemoved = MutableLiveData<Event<Boolean>>()
  val onFabricRemoved: LiveData<Event<Boolean>>
    get() = _onFabricRemoved

  protected lateinit var matterSettings: MatterSettings

  init {
    viewModelScope.launch {
      val jsonSettings = savedStateHandle.get<String>("setting")
      jsonSettings?.let { matterSettings = Json.decodeFromString(it) }
    }
  }

  override fun onCleared() {
    super.onCleared()
    Timber.d("onCleared()")
  }

  protected fun onFabricRemoved() {
    Timber.d("onFabricRemoved()")
    _onFabricRemoved.value = Event(true)
  }
}
