package com.samsung.matter.chipstdeviceapp.feature.setup

import androidx.lifecycle.*
import com.samsung.matter.chipstdeviceapp.core.common.successOr
import com.samsung.matter.chipstdeviceapp.core.domain.usecase.network.GetSSIDUseCase
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.async
import timber.log.Timber
import javax.inject.Inject

@HiltViewModel
class SetupViewModel @Inject constructor(
    private val getSSIDUseCase: GetSSIDUseCase
) : ViewModel() {

    override fun onCleared() {
        Timber.d("Hit")
        super.onCleared()
    }

    suspend fun getSSID(): String {
        Timber.d("Hit")
        val deferred = viewModelScope.async {
            getSSIDUseCase().successOr("Unknown")
        }

        val ssid = deferred.await()
        Timber.d("ssid:${ssid}")
        return ssid
    }
}