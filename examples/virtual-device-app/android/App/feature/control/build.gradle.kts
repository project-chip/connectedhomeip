import com.matter.buildsrc.Deps
import com.matter.buildsrc.Versions

plugins {
    id("com.android.library")
    id("org.jetbrains.kotlin.android")
    id("com.google.dagger.hilt.android")
    id("androidx.navigation.safeargs.kotlin")
    kotlin("kapt")
}

android {
    namespace = "com.matter.virtual.device.app.feature.control"
    compileSdk = Versions.compileSdkVersion

    defaultConfig {
        minSdk = Versions.minSdkVersion
        targetSdk = Versions.targetSdkVersion

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        consumerProguardFiles("consumer-rules.pro")
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    kotlinOptions {
        jvmTarget = "1.8"
    }
    buildFeatures {
        viewBinding = true
        dataBinding = true
    }
}

dependencies {

    implementation(project(":core:common"))
    implementation(project(":core:domain"))
    implementation(project(":core:model"))
    implementation(project(":core:ui"))

    implementation(Deps.AndroidX.core)
    implementation(Deps.AndroidX.appcompat)
    implementation(Deps.AndroidX.fragment)
    implementation(Deps.AndroidX.Lifecycle.viewmodel)
    implementation(Deps.AndroidX.Lifecycle.livedata)

    implementation(Deps.Kotlin.serialization)

    implementation(Deps.Navigation.fragment)
    implementation(Deps.Navigation.ui)

    implementation(Deps.Dagger.hiltAndroid)
    kapt(Deps.Dagger.hiltAndroidCompiler)

    implementation(Deps.timber)

    testImplementation(Deps.Test.junit)
    androidTestImplementation(Deps.Test.junitExt)
    androidTestImplementation(Deps.Test.espresso)
}