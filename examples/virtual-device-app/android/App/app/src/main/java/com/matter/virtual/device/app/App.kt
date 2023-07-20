package com.matter.virtual.device.app

import android.app.Application
import dagger.hilt.android.HiltAndroidApp
import timber.log.Timber

@HiltAndroidApp
class App : Application() {

  override fun onCreate() {
    super.onCreate()
    Timber.plant(TimberDebugTree())
  }
}
