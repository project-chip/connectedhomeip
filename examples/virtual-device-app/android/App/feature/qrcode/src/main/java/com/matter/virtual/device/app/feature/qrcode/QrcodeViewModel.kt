package com.matter.virtual.device.app.feature.qrcode

import androidx.lifecycle.*
import com.matter.virtual.device.app.core.common.Event
import com.matter.virtual.device.app.core.common.MatterSettings
import com.matter.virtual.device.app.core.common.successOr
import com.matter.virtual.device.app.core.domain.usecase.matter.GetManualPairingCodeStringUseCase
import com.matter.virtual.device.app.core.domain.usecase.matter.GetQrcodeStringUseCase
import com.matter.virtual.device.app.core.domain.usecase.matter.IsCommissioningSessionEstablishmentStarted
import com.matter.virtual.device.app.core.domain.usecase.matter.StartMatterAppServiceUseCase
import com.matter.virtual.device.app.core.model.Payload
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
  private val isCommissioningSessionEstablishmentStarted:
    IsCommissioningSessionEstablishmentStarted,
  private val startMatterAppServiceUseCase: StartMatterAppServiceUseCase,
  savedStateHandle: SavedStateHandle
) : ViewModel() {

  private val _uiState = MutableLiveData<Event<QrcodeUiState>>(Event(QrcodeUiState.Loading))
  val uiState: LiveData<Event<QrcodeUiState>>
    get() = _uiState

  init {
    viewModelScope.launch {
      val jsonSettings = savedStateHandle.get<String>("setting")
      jsonSettings?.let {
        val matterSettings = Json.decodeFromString<MatterSettings>(it)

        startMatterAppServiceUseCase(matterSettings)
        delay(500)

        val payload =
          Payload(
            onboardingType = matterSettings.onboardingType,
            discriminator = matterSettings.discriminator
          )

        val qrCode = getQrcodeStringUseCase(payload).successOr("")
        val manualPairingCode = getManualPairingCodeStringUseCase(payload).successOr("")

        _uiState.value = Event(QrcodeUiState.Qrcode(qrCode, manualPairingCode))

        val isCommissioningSessionEstablishmentStarted =
          isCommissioningSessionEstablishmentStarted().successOr(false)
        if (isCommissioningSessionEstablishmentStarted) {
          Timber.d("Session Establishment Started")
          _uiState.value = Event(QrcodeUiState.SessionEstablishmentStarted)
        }
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

  object SessionEstablishmentStarted : QrcodeUiState
}
