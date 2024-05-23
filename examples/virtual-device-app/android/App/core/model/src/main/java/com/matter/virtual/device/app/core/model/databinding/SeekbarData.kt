package com.matter.virtual.device.app.core.model.databinding

import androidx.lifecycle.LiveData

data class SeekbarData(val progress: LiveData<Int>, val min: Int = 0, val max: Int = 100)
