package com.matter.virtual.device.app.core.data.repository

interface NetworkRepository {
  suspend fun getSSID(): String
}
