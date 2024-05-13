package com.matter.virtual.device.app.core.common.di

import javax.inject.Qualifier

@Retention(AnnotationRetention.BINARY) @Qualifier annotation class DefaultDispatcher

@Retention(AnnotationRetention.BINARY) @Qualifier annotation class IoDispatcher

@Retention(AnnotationRetention.BINARY) @Qualifier annotation class MainDispatcher

@Retention(AnnotationRetention.BINARY) @Qualifier annotation class MainImmediateDispatcher
