# Matter TV Casting Android App Example

This is a Matter TV Casting Android app that can be used to cast content to a
TV. This app discovers TVs on the local network that act as commissioners, lets
the user select one, sends the TV a User Directed Commissioning request, enters
commissioning mode, advertises itself as a Commissionable Node and gets
commissioned. Then it allows the user to send Matter ContentLauncher commands to
the TV.

Refer to the
[Matter Casting APIs documentation](https://project-chip.github.io/connectedhomeip-doc/examples/tv-casting-app/APIs.html)
to build the Matter “Casting Client” into your consumer-facing mobile app.

<hr>

-   [Matter TV Casting Android App Example](#matter-tv-casting-android-app-example)
    -   [Requirements for building](#requirements-for-building)
        -   [ABIs and TARGET_CPU](#abis-and-target_cpu)
        -   [Gradle \& JDK Version](#gradle--jdk-version)
    -   [Preparing for build](#preparing-for-build)
    -   [Building \& Installing the app](#building--installing-the-app)
    -   [Common build environment issues](#common-build-environment-issues)
    -   [Running the app](#running-the-app)

<hr>

## Requirements for building

Refer to
[this file](../../../docs/platforms/android/android_building.md#requirements-for-building)
to download the recommended version for the Android SDK and NDK for your
machine. Set the `$ANDROID_HOME` environment variable to where the SDK is
downloaded and the `$ANDROID_NDK_HOME` environment variable to point to where
the NDK package is downloaded.

```
export ANDROID_HOME=~/Library/Android/sdk
export ANDROID_NDK_HOME=~/Library/Android/sdk/ndk/{NDK_VERSION_NUMBER}
```

### ABIs and TARGET_CPU

`TARGET_CPU` can have the following values, depending on your smartphone CPU
architecture:

| ABI         | TARGET_CPU |
| ----------- | ---------- |
| armeabi-v7a | arm        |
| arm64-v8a   | arm64      |
| x86         | x86        |
| x86_64      | x64        |

### Gradle & JDK Version

We are using Gradle 7.1.1 for all android project which does not support Java 17
(https://docs.gradle.org/current/userguide/compatibility.html) while the default
JDK version on MacOS for Apple Silicon is 'openjdk 17.0.1' or above.

If you attempt to build with an incompatible Java version, you may encounter the
following error:

```text
Unsupported class file major version XX
```

This error occurs when the Java version being used is not compatible with the
Gradle version in your project.

See the
[Building Android](../../../docs/platforms/android/android_building.md#gradle--jdk-version)
guide for more info about the supported Gradle & JDK Version.

You can verify your current Java version by running:

```shell
java -version
```

<hr>

## Preparing for build

Complete the following steps to prepare the Matter build:

1. Check out the Matter repository.

2. Run bootstrap (**only required first time**)

    ```shell
    source scripts/bootstrap.sh
    ```

3. The build will produce libraries: AndroidPlatform.jar, CHIPAppServer.jar,
   CHIPInteractionModel.jar and TVCastingApp.jar in [App/app/libs](App/app/libs)
   and libTvCastingApp.so and libc++\_shared.so in
   [App/app/libs/jniLibs/](App/app/libs/jniLibs/) consumed by any casting app to
   leverage the [casting APIs](../APIs.md), like the sample android
   tv-casting-app. If building for your own casting app, make sure your client's
   specific values are set for `CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID` and
   `CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID` in the
   [CHIPProjectAppConfig.h](tv-casting-common/include/CHIPProjectAppConfig.h)
   file, before the build. Other values like the
   `CHIP_DEVICE_CONFIG_DEVICE_NAME` may be optionally updated as well.

## Building & Installing the app

This is the simplest option. In the command line, run the following command from
the top Matter directory:

```shell
./scripts/build/build_examples.py --target android-arm64-tv-casting-app build
```

(To build this app with no debugging hooks, use the
`android-arm64-tv-casting-app-no-debug` target)

See the table above for other values of `TARGET_CPU`.

The debug Android package `app-debug.apk` will be generated at
`out/android-$TARGET_CPU-tv-casting-app/outputs/apk/debug/`, and can be
installed with

```shell
adb install out/android-$TARGET_CPU-tv-casting-app/outputs/apk/debug/app-debug.apk
```

You can use Android Studio to edit the Android app itself and run it after
build_examples.py, but you will not be able to edit Matter Android code from
`src/controller/java`, or other Matter C++ code within Android Studio.

## Common build environment issues

1. If you see an error like `kotlinc: command not found`, install the Kotlin in
   your build environment. Eg. on MacOS, this can be done with the command:

```shell
brew install kotlin
```

## Running the app

This example Matter TV Casting Android app can be tested with the following
video players:

1. With the
   [example Matter tv-app](https://github.com/project-chip/connectedhomeip/tree/master/examples/tv-app)
   running on a Raspberry Pi - works out of the box.
2. With a FireTV device - requires your Amazon Customer ID to be allow-listed
   first.
