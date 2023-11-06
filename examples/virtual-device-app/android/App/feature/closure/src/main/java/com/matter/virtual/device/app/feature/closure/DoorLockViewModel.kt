package com.matter.virtual.device.app.feature.closure

import androidx.lifecycle.LiveData
import androidx.lifecycle.SavedStateHandle
import androidx.lifecycle.asLiveData
import androidx.lifecycle.viewModelScope
import com.matter.virtual.device.app.core.common.successOr
import com.matter.virtual.device.app.core.domain.usecase.matter.IsFabricRemovedUseCase
import com.matter.virtual.device.app.core.domain.usecase.matter.StartMatterAppServiceUseCase
import com.matter.virtual.device.app.core.domain.usecase.matter.cluster.doorlock.GetLockStateFlowUseCase
import com.matter.virtual.device.app.core.domain.usecase.matter.cluster.doorlock.SendLockAlarmEventUseCase
import com.matter.virtual.device.app.core.domain.usecase.matter.cluster.doorlock.SetLockStateUseCase
import com.matter.virtual.device.app.core.domain.usecase.matter.cluster.powersource.GetBatPercentRemainingUseCase
import com.matter.virtual.device.app.core.domain.usecase.matter.cluster.powersource.SetBatPercentRemainingUseCase
import com.matter.virtual.device.app.core.ui.BaseViewModel
import dagger.hilt.android.lifecycle.HiltViewModel
import javax.inject.Inject
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch
import timber.log.Timber

@HiltViewModel
class DoorLockViewModel
@Inject
constructor(
  getLockStateFlowUseCase: GetLockStateFlowUseCase,
  getBatPercentRemainingUseCase: GetBatPercentRemainingUseCase,
  private val setLockStateUseCase: SetLockStateUseCase,
  private val sendLockAlarmEventUseCase: SendLockAlarmEventUseCase,
  private val setBatPercentRemainingUseCase: SetBatPercentRemainingUseCase,
  private val startMatterAppServiceUseCase: StartMatterAppServiceUseCase,
  private val isFabricRemovedUseCase: IsFabricRemovedUseCase,
  savedStateHandle: SavedStateHandle
) : BaseViewModel(savedStateHandle) {

  private val _lockState: StateFlow<Boolean> = getLockStateFlowUseCase()
  val lockState: LiveData<Boolean>
    get() = _lockState.asLiveData()

  private val _batteryStatus: MutableStateFlow<Int> =
    getBatPercentRemainingUseCase() as MutableStateFlow<Int>
  val batteryStatus: LiveData<Int>
    get() = _batteryStatus.asLiveData()

  init {
    viewModelScope.launch { startMatterAppServiceUseCase(matterSettings) }

    viewModelScope.launch {
      val isFabricRemoved = isFabricRemovedUseCase().successOr(false)
      if (isFabricRemoved) {
        Timber.d("Fabric Removed")
        onFabricRemoved()
      }
    }
  }

  override fun onCleared() {
    Timber.d("onCleared()")
    super.onCleared()
  }

  fun onClickButton() {
    Timber.d("onClickButton()")
    viewModelScope.launch {
      Timber.d("current lockState value = ${_lockState.value}")
      if (_lockState.value == LOCK_STATE_LOCKED) {
        Timber.d("set value = unlocked")
        setLockStateUseCase(LOCK_STATE_UNLOCKED)
      } else {
        Timber.d("set value = locked")
        setLockStateUseCase(LOCK_STATE_LOCKED)
      }
    }
  }

  fun onClickSendLockAlarmEventButton() {
    Timber.d("onClickSendLockAlarmEventButton()")
    viewModelScope.launch {
      if (!_lockState.value) {
        // if lockState == locked, send alarm event and change the lockState to unlocked
        sendLockAlarmEventUseCase()
        setLockStateUseCase(LOCK_STATE_UNLOCKED)
      }
    }
  }

  fun updateBatterySeekbarProgress(progress: Int) {
    _batteryStatus.value = progress
  }

  fun updateBatteryStatusToCluster(progress: Int) {
    Timber.d("progress:$progress")
    viewModelScope.launch {
      updateBatterySeekbarProgress(progress)
      setBatPercentRemainingUseCase(progress)
    }
  }

  companion object {
    const val LOCK_STATE_LOCKED = false
    const val LOCK_STATE_UNLOCKED = true
  }
}
