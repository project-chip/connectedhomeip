package com.matter.virtual.device.app.core.data.di

import com.matter.virtual.device.app.core.data.repository.MatterRepository
import com.matter.virtual.device.app.core.data.repository.MatterRepositoryImpl
import com.matter.virtual.device.app.core.data.repository.NetworkRepository
import com.matter.virtual.device.app.core.data.repository.NetworkRepositoryImpl
import dagger.Binds
import dagger.Module
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent

@InstallIn(SingletonComponent::class)
@Module
internal abstract class DataModule {

  @Binds abstract fun bindMatterRepository(repository: MatterRepositoryImpl): MatterRepository

  @Binds abstract fun bindNetworkRepository(repository: NetworkRepositoryImpl): NetworkRepository
}
