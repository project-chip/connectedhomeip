# Building Android CHIPTool

Android CHIPTool is an application for Android for commissioning and controlling
CHIP accessory devices.

CHIPTool offers the following features:

-   Scan a CHIP QR code and display payload information to the user
-   Read the NFC tag containing CHIP onboarding information
-   Commission a CHIP device
-   Send echo requests to the CHIP echo server
-   Send on/off cluster requests to a CHIP device

<hr>

-   [Source files](#source)
-   [Requirements for building](#requirements)
    -   [ABIs and TARGET_CPU](#abi)
-   [Preparing for build](#preparing)
-   [Building Android CHIPTool from scripts](#building-scripts)
-   [Building Android CHIPTool from Android Studio](#building-studio)

<hr>

<a name="source"></a>

## Source files

You can find source files of the Android CHIPTool application in the
`src/android/CHIPTool` directory.

<hr>

<a name="requirements"></a>

## Requirements for building

You need Android SDK 21 & NDK downloaded to your machine. Set the
`$ANDROID_HOME` environment variable to where the SDK is downloaded and the
`$ANDROID_NDK_HOME` environment variable to point to where the NDK package is
downloaded.

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

<hr>

<a name="preparing"></a>

## Preparing for build

Complete the following steps to prepare the CHIP build:

1. Check out the CHIP repository.

2. Run bootstrap (**only required first time**)

    ```shell
    source scripts/bootstrap.sh
    ```

3. Choose how you want to build the Android CHIPTool. There are two ways: from
   script, or from source within Android Studio.

<a name="building-scripts"></a>

## Building Android CHIPTool from scripts

This is the simplest option. In the command line, run the following command from
the top CHIP directory:

```shell
./scripts/build/build_examples.py --platform android --board arm64 build
```

See the table above for other values of `TARGET_CPU`.

The debug Android package `app-debug.apk` will be generated at
`out/android-$TARGET_CPU-chip_tool/outputs/apk/debug/`, and can be installed
with

```shell
adb install out/android-$TARGET_CPU-chip_tool/outputs/apk/debug/app-debug.apk
```

You can use Android Studio to edit the Android CHIPTool app itself, but you will
not be able to edit CHIP Android code from `src/controller/java`, or other CHIP
C++ code within Android Studio.

<a name="building-studio"></a>

## Building Android CHIPTool from Android Studio

This option allows Android Studio to build the core CHIP code from source, which
allows us to directly edit core CHIP code in-IDE.

1. In the command line, run the following command from the top CHIP directory:

    ```shell
    TARGET_CPU=arm64 ./scripts/examples/android_app_ide.sh
    ```

    See the table above for other values of `TARGET_CPU`.

2. Modify the `matterBuildSrcDir` variable in
   [src/android/CHIPTool/build.gradle](https://github.com/project-chip/connectedhomeip/blob/master/src/android/CHIPTool/build.gradle)
   to point to the appropriate output directory (e.g. `out/android_arm64`).

3. Open the project in Android Studio and run **Sync Project with Gradle
   Files**.

4. Use one of the following options to build an Android package:

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
