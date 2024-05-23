package com.matter.buildsrc

object Deps {
  object AndroidX {
    const val core = "androidx.core:core-ktx:1.8.0"
    const val appcompat = "androidx.appcompat:appcompat:1.5.1"
    const val fragment = "androidx.fragment:fragment-ktx:1.5.7"

    object Lifecycle {
      private const val lifecycleVersion = "2.3.1"
      const val viewmodel = "androidx.lifecycle:lifecycle-viewmodel-ktx:$lifecycleVersion"
      const val livedata = "androidx.lifecycle:lifecycle-livedata-ktx:$lifecycleVersion"
      const val compiler = "androidx.lifecycle:lifecycle-compiler:$lifecycleVersion"
    }
  }

  const val material = "com.google.android.material:material:1.7.0"

  object Kotlin {
    const val coroutinesCore = "org.jetbrains.kotlinx:kotlinx-coroutines-core:1.5.0"
    const val coroutinesAndroid = "org.jetbrains.kotlinx:kotlinx-coroutines-android:1.5.1"
    const val reflect = "org.jetbrains.kotlin:kotlin-reflect:1.5.31"
    const val serialization = "org.jetbrains.kotlinx:kotlinx-serialization-json:1.2.2"
  }

  object Dagger {
    private const val daggerVersion = "2.44.2"
    const val hiltAndroid = "com.google.dagger:hilt-android:$daggerVersion"
    const val hiltAndroidCompiler = "com.google.dagger:hilt-android-compiler:$daggerVersion"
  }

  object Test {
    const val junit = "junit:junit:4.13.2"
    const val junitExt = "androidx.test.ext:junit:1.1.3"
    const val espresso = "androidx.test.espresso:espresso-core:3.4.0"
  }

  object Navigation {
    private const val navigationVersion = "2.5.3"
    const val fragment = "androidx.navigation:navigation-fragment-ktx:$navigationVersion"
    const val ui = "androidx.navigation:navigation-ui-ktx:$navigationVersion"
  }

  const val timber = "com.jakewharton.timber:timber:5.0.1"
  const val inject = "javax.inject:javax.inject:1"
  const val zxing = "com.google.zxing:core:3.4.0"
  const val lottie = "com.airbnb.android:lottie:3.4.1"
  const val gson = "com.google.code.gson:gson:2.8.5"
}
