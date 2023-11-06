package com.matter.virtual.device.app.core.common

sealed class Result<out R> {

  data class Success<out T>(val data: T) : Result<T>()

  data class Error(val exception: Throwable) : Result<Nothing>()

  object Loading : Result<Nothing>()
}

val Result<*>.succeeded
  get() = this is Result.Success && data != null

fun <T> Result<T>.successOr(fallback: T): T {
  return (this as? Result.Success<T>)?.data ?: fallback
}

val <T> Result<T>.data: T?
  get() = (this as? Result.Success)?.data
