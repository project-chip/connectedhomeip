package com.matter.virtual.device.app.core.common.di

import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton
import kotlinx.coroutines.CoroutineDispatcher
import kotlinx.coroutines.Dispatchers

@InstallIn(SingletonComponent::class)
@Module
internal object CoroutinesModule {
  @DefaultDispatcher
  @Provides
  @Singleton
  fun provideDefaultDispatcher(): CoroutineDispatcher = Dispatchers.Default

  @IoDispatcher @Provides @Singleton fun provideIoDispatcher(): CoroutineDispatcher = Dispatchers.IO

  @MainDispatcher
  @Provides
  @Singleton
  fun provideMainDispatcher(): CoroutineDispatcher = Dispatchers.Main

  @MainImmediateDispatcher
  @Provides
  @Singleton
  fun provideMainImmediateDispatcher(): CoroutineDispatcher = Dispatchers.Main.immediate
}
