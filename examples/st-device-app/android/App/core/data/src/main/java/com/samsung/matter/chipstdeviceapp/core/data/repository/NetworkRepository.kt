package com.samsung.matter.chipstdeviceapp.core.data.repository

interface NetworkRepository {
    suspend fun getSSID(): String
}