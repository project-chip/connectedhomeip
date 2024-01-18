package com.matter.virtual.device.app.core.matter.di

import com.matter.virtual.device.app.DeviceApp
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

@InstallIn(SingletonComponent::class)
@Module
internal object MatterModule {

  @Provides @Singleton fun provideDeviceApp(): DeviceApp = DeviceApp()
}
