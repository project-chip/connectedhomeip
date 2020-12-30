# Android CHIPTool

This directory contains the Android Studio project for CHIPTool, an Android
application for commissioning and controlling CHIP accessories.

CHIPTool offers the following features:

-   Scan a CHIP QR code and display payload information to the user
-   Read the NFC tag containing CHIP onboarding information
-   Commission a CHIP device
-   Send echo requests to the CHIP echo server
-   Send on/off cluster requests to a CHIP device

> :warning: Connection to the CHIP device's soft AP will be a manual step until
> pairing is implemented.

## Requirements for building

You need Android SDK 21 & NDK downloaded to your machine. Set the
`$ANDROID_HOME` environment variable to where the SDK is downloaded and the
`$ANDROID_NDK_HOME` environment variable to point to where the NDK package is
downloaded.

## ABIs and TARGET_CPU

`TARGET_CPU` can have the following values, depending on your smartphone CPU
architecture:

| ABI         | TARGET_CPU |
| ----------- | ---------- |
| armeabi-v7a | arm        |
| arm64-v8a   | arm64      |
| x86         | x86        |
| x86_64      | x64        |

## Building CHIPTool

Complete the following steps to build CHIPTool:

1. Check out the CHIP repository.

2. In the command line, run the following command from the top CHIP directory:

    ```shell
    sudo apt-get install -y swig # "brew install swig" for macOS.
    TARGET_CPU=arm64 ./scripts/examples/android_app.sh
    ```

    See the table above for other values of `TARGET_CPU`.

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
`./app/build/outputs/apk/debug/`.
