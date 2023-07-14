package com.samsung.matter.chipstdeviceapp.core.data.di

import com.samsung.matter.chipstdeviceapp.core.data.repository.*
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
