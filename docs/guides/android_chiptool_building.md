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
-   [Building Android CHIPTool](#building)

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

<a name="building"></a>

## Building Android CHIPTool

Complete the following steps to build CHIPTool:

1. Check out the CHIP repository.

2. Run bootstrap (**only required first time**)

    ```shell
    source scripts/bootstrap.sh
    ```

3. In the command line, run the following command from the top CHIP directory:

    ```shell
    TARGET_CPU=arm64 ./scripts/examples/android_app.sh
    ```

    See the table above for other values of `TARGET_CPU`.

4. Open the project in Android Studio and run **Sync Project with Gradle
   Files**.

5. Use one of the following options to build an Android package:

    - Click **Make Project** in Android Studio.
    - Run the following command in the command line:

        ```shell
        cd src/android/CHIPTool
        ./gradlew build
        ```

The debug Android package `app-debug.apk` will be generated at
`src/android/CHIPTool/app/build/outputs/apk/debug/`.
