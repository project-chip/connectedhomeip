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
import com.matter.virtual.device.app.core.model.matter.LockState
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

  private val _lockState: StateFlow<LockState> = getLockStateFlowUseCase()
  val lockState: LiveData<LockState>
    get() = _lockState.asLiveData()

  private val _batteryRemainingPercentage: MutableStateFlow<Int> =
    getBatPercentRemainingUseCase() as MutableStateFlow<Int>
  val batteryRemainingPercentage: LiveData<Int>
    get() = _batteryRemainingPercentage.asLiveData()

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

  fun setLockState(lockState: LockState) {
    Timber.d("setLockState():new:$lockState")
    viewModelScope.launch { setLockStateUseCase(lockState) }
  }

  fun onClickSendLockAlarmEventButton() {
    Timber.d("onClickSendLockAlarmEventButton()")
    viewModelScope.launch {
      if (_lockState.value == LockState.LOCKED) {
        // if lockState == locked, send alarm event and change the lockState to unlocked
        sendLockAlarmEventUseCase()
        setLockStateUseCase(LockState.UNLOCKED)
      }
    }
  }

  fun updateBatterySeekbarProgress(progress: Int) {
    _batteryRemainingPercentage.value = progress
  }

  fun updateBatteryStatusToCluster(progress: Int) {
    Timber.d("progress:$progress")
    viewModelScope.launch {
      updateBatterySeekbarProgress(progress)
      setBatPercentRemainingUseCase(progress)
    }
  }
}
