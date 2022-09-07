# Building Android

There are following Apps on Android

-   CHIPTool - Android CHIPTool is an application for Android for commissioning
    and controlling Matter accessory devices. It offers the following features:
    -   Scan a Matter QR code and display payload information to the user
    -   Read the NFC tag containing Matter onboarding information
    -   Commission a Matter device
    -   Send echo requests to the Matter echo server
    -   Send on/off cluster requests to a Matter device
-   CHIPTest
    -   Android CHIPTest is an application for Android for running Matter's unit
        tests

<hr>

-   [Building Android](#building-android)
    -   [Source files](#source-files)
    -   [Requirements for building](#requirements-for-building)
        -   [Linux](#linux)
        -   [MacOS](#macos)
        -   [ABIs and TARGET_CPU](#abis-and-target_cpu)
        -   [Gradle & JDK Version](#gradle--jdk-version)
    -   [Preparing for build](#preparing-for-build)
    -   [Building Android CHIPTool from scripts](#building-android-chiptool-from-scripts)
    -   [Building Android CHIPTool from Android Studio](#building-android-chiptool-from-android-studio)
    -   [Building Android CHIPTest from scripts](#building-android-chiptest-from-scripts)

<hr>

<a name="source"></a>

## Source files

You can find source files of the Android applications in the `src/android/`
directory.

<hr>

<a name="requirements"></a>

## Requirements for building

You need Android SDK 21 & NDK 21.4.7075529 downloaded to your machine. Set the
`$ANDROID_HOME` environment variable to where the SDK is downloaded and the
`$ANDROID_NDK_HOME` environment variable to point to where the NDK package is
downloaded.

1. Install [Android Studio](https://developer.android.com/studio)
2. Install NDK:
    1. Tools -> SDK Manager -> SDK Tools Tab
    2. Click [x] Show Package Details
    3. Select NDK (Side by Side) -> 21.4.7075529
    4. Apply
3. Install Command Line Tools:
    1. Tools -> SDK Manager -> SDK Tools Tab -> Android SDK Command Line Tools
       (latest)
    2. Apply
4. Install SDK 21:
    1. Tools -> SDK Manager -> SDK Platforms Tab -> Android 5.0 (Lollipop) SDK
       Level 21
    2. Apply
5. Install Emulator:
    1. Tools -> Device Manager -> Create device -> Pixel 5 -> Android S API 31
       -> Download

### Linux

```
export ANDROID_HOME=~/Android/Sdk
export ANDROID_NDK_HOME=~/Android/Sdk/ndk/21.4.7075529
```

### MacOS

```
export ANDROID_HOME=~/Library/Android/sdk
export ANDROID_NDK_HOME=~/Library/Android/sdk/ndk/21.4.7075529
```

<a name="abi"></a>

### ABIs and TARGET_CPU

`TARGET_CPU` can have the following values, depending on your smartphone CPU
architecture:

| ABI         | TARGET_CPU |
| ----------- | ---------- |
| armeabi-v7a | arm        |
| arm64-v8a   | arm64      |
| x86         | x86        |
| x86_64      | x64        |

<a name="jdk"></a>

### Gradle & JDK Version

We are using Gradle 7.1.1 for all android project which does not support Java 17
(https://docs.gradle.org/current/userguide/compatibility.html) while the default
JDK version on MacOS for Apple Silicon is 'openjdk 17.0.1' or above.

Using JDK bundled with Android Studio will help with that.

```shell
export JAVA_HOME=/Applications/Android\ Studio.app/Contents/jre/Contents/Home/
```

<hr>

<a name="preparing"></a>

## Preparing for build

Complete the following steps to prepare the Matter build:

1. Check out the Matter repository.

2. Run bootstrap (**only required first time**)

    ```shell
    source scripts/bootstrap.sh
    ```

3. Choose how you want to build the Android CHIPTool. There are **two** ways:
   from script, or from source within Android Studio.

<a name="building-scripts"></a>

## Building Android CHIPTool from scripts

This is the simplest option. In the command line, run the following command from
the top CHIP directory:

```shell
./scripts/build/build_examples.py --target android-arm64-chip-tool build
```

See the table above for other values of `TARGET_CPU`.

The debug Android package `app-debug.apk` will be generated at
`out/android-$TARGET_CPU-chip-tool/outputs/apk/debug/`, and can be installed
with

```shell
adb install out/android-$TARGET_CPU-chip-tool/outputs/apk/debug/app-debug.apk
```

You can use Android Studio to edit the Android CHIPTool app itself and run it
after build_examples.py, but you will not be able to edit Matter Android code
from `src/controller/java`, or other Matter C++ code within Android Studio.

<a name="building-studio"></a>

## Building Android CHIPTool from Android Studio

This option allows Android Studio to build the core Matter code from source,
which allows us to directly edit core Matter code in-IDE.

1. In the command line, run the following command from the top Matter directory:

    ```shell
    TARGET_CPU=arm64 ./scripts/examples/android_app_ide.sh
    ```

    See the table above for other values of `TARGET_CPU`.

2. Modify the `matterSdkSourceBuild` variable to true, `matterBuildSrcDir` point
   to the appropriate output directory (e.g. `../../../../out/android_arm64`),
   and `matterSourceBuildAbiFilters` to the desired ABIs in
   [src/android/CHIPTool/gradle.properties](https://github.com/project-chip/connectedhomeip/blob/master/src/android/CHIPTool/gradle.properties)

3) Open the project in Android Studio and run **File -> Sync Project with Gradle
   Files**.

4) Use one of the following options to build an Android package:

    - Click **Make Project** in Android Studio.
    - Run the following command in the command line:

        ```shell
        cd src/android/CHIPTool
        ./gradlew build
        ```

The debug Android package `app-debug.apk` will be generated at
`src/android/CHIPTool/app/build/outputs/apk/debug/`, and can be installed with

```shell
adb install src/android/CHIPTool/app/build/outputs/apk/debug/app-debug.apk
```

or

```shell
(cd src/android/CHIPTool && ./gradlew installDebug)
```

<a name="building-chiptest-scripts"></a>

## Building Android CHIPTest from scripts

Currently, the CHIPTest can only be built from scripts. The steps are similar to
[building CHIPTool from scripts](#building-scripts).

```shell
./scripts/build/build_examples.py --target android-arm64-chip-test build
```

You can modify the `matterUTestLib` variable to the test lib in
[src/android/CHIPTest/gradle.properties](https://github.com/project-chip/connectedhomeip/blob/master/src/android/CHIPTest/gradle.properties)
to change target to test.
