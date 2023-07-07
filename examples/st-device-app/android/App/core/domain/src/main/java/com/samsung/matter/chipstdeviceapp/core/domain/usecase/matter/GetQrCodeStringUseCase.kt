package com.samsung.matter.chipstdeviceapp.core.domain.usecase.matter

import com.samsung.matter.chipstdeviceapp.core.common.di.IoDispatcher
import com.samsung.matter.chipstdeviceapp.core.data.repository.MatterRepository
import com.samsung.matter.chipstdeviceapp.core.domain.CoroutineUseCase
import com.samsung.matter.chipstdeviceapp.core.model.Payload
import kotlinx.coroutines.CoroutineDispatcher
import javax.inject.Inject

class GetQrcodeStringUseCase @Inject constructor(
    private val matterRepository: MatterRepository,
    @IoDispatcher dispatcher: CoroutineDispatcher
) : CoroutineUseCase<Payload, String>(dispatcher) {

    override suspend fun execute(param: Payload): String {
        return matterRepository.getQrcodeString(param)
    }
}