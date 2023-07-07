package com.samsung.matter.chipstdeviceapp.core.data.repository

import com.samsung.matter.chipstdeviceapp.core.model.Payload

interface MatterRepository {
    fun getQrcodeString(payload: Payload): String
    fun getManualPairingCodeString(payload: Payload): String
}