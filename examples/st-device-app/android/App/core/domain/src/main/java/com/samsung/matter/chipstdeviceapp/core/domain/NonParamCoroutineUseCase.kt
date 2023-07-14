package com.samsung.matter.chipstdeviceapp.core.domain

import com.samsung.matter.chipstdeviceapp.core.common.Result
import kotlinx.coroutines.CoroutineDispatcher
import kotlinx.coroutines.withContext

abstract class NonParamCoroutineUseCase<R>(private val coroutineDispatcher: CoroutineDispatcher) {

  suspend operator fun invoke(): Result<R> {
    return try {
      withContext(coroutineDispatcher) { execute().let { Result.Success(it) } }
    } catch (e: Exception) {
      Result.Error(e)
    }
  }

  @Throws(RuntimeException::class) protected abstract suspend fun execute(): R
}
