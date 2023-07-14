package com.samsung.matter.chipstdeviceapp.feature.qrcode

import androidx.lifecycle.*
import com.samsung.matter.chipstdeviceapp.core.common.MatterSettings
import com.samsung.matter.chipstdeviceapp.core.common.successOr
import com.samsung.matter.chipstdeviceapp.core.domain.usecase.matter.GetManualPairingCodeStringUseCase
import com.samsung.matter.chipstdeviceapp.core.domain.usecase.matter.GetQrcodeStringUseCase
import com.samsung.matter.chipstdeviceapp.core.model.Payload
import dagger.hilt.android.lifecycle.HiltViewModel
import javax.inject.Inject
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.json.Json
import timber.log.Timber

@OptIn(ExperimentalSerializationApi::class)
@HiltViewModel
class QrcodeViewModel
@Inject
constructor(
  private val getQrcodeStringUseCase: GetQrcodeStringUseCase,
  private val getManualPairingCodeStringUseCase: GetManualPairingCodeStringUseCase,
  savedStateHandle: SavedStateHandle
) : ViewModel() {

  private val _uiState = MutableLiveData<QrcodeUiState>(QrcodeUiState.Loading)
  val uiState: LiveData<QrcodeUiState>
    get() = _uiState

  init {
    viewModelScope.launch {
      val jsonSettings = savedStateHandle.get<String>("setting")
      jsonSettings?.let {
        val matterSettings = Json.decodeFromString<MatterSettings>(it)

        // TODO : start service
        delay(500)

        val payload =
          Payload(
            productId = matterSettings.device.productId.toInt(),
            onboardingType = matterSettings.onboardingType,
            discriminator = matterSettings.discriminator
          )

        val qrCode = getQrcodeStringUseCase(payload).successOr("")
        val manualPairingCode = getManualPairingCodeStringUseCase(payload).successOr("")

        _uiState.value = QrcodeUiState.Qrcode(qrCode, manualPairingCode)
      }
    }
  }

  override fun onCleared() {
    super.onCleared()
    Timber.d("onCleared()")
  }
}

sealed interface QrcodeUiState {
  object Loading : QrcodeUiState

  data class Qrcode(val qrCode: String, val manualPairingCode: String) : QrcodeUiState
}
