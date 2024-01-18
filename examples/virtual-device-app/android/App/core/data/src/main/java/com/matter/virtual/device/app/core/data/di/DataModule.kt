package com.matter.virtual.device.app.core.data.di

import com.matter.virtual.device.app.core.data.repository.*
import com.matter.virtual.device.app.core.data.repository.cluster.DoorLockManagerRepository
import com.matter.virtual.device.app.core.data.repository.cluster.DoorLockManagerRepositoryImpl
import com.matter.virtual.device.app.core.data.repository.cluster.OnOffManagerRepository
import com.matter.virtual.device.app.core.data.repository.cluster.OnOffManagerRepositoryImpl
import com.matter.virtual.device.app.core.data.repository.cluster.PowerSourceManagerRepository
import com.matter.virtual.device.app.core.data.repository.cluster.PowerSourceManagerRepositoryImpl
import dagger.Binds
import dagger.Module
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent

@InstallIn(SingletonComponent::class)
@Module
internal abstract class DataModule {
  @Binds
  abstract fun bindDoorLockManagerRepository(
    repository: DoorLockManagerRepositoryImpl
  ): DoorLockManagerRepository

  @Binds
  abstract fun bindOnOffManagerRepository(
    repository: OnOffManagerRepositoryImpl
  ): OnOffManagerRepository

  @Binds
  abstract fun bindPowerSourceManagerRepository(
    repository: PowerSourceManagerRepositoryImpl
  ): PowerSourceManagerRepository

  @Binds abstract fun bindMatterRepository(repository: MatterRepositoryImpl): MatterRepository

  @Binds abstract fun bindNetworkRepository(repository: NetworkRepositoryImpl): NetworkRepository

  @Binds
  abstract fun bindSharedPreferencesRepository(
    repository: SharedPreferencesRepositoryImpl
  ): SharedPreferencesRepository
}
