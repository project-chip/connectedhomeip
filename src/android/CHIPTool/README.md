# Android CHIPTool

This directory contains an Android Studio project for CHIPTool, an Android application for commissioning and controlling CHIP accessories.

CHIPTool features: 
- Scan a CHIP QR code and display payload information to the user
- Read the NFC tag containing CHIP onboarding information
- Commission a CHIP device
- Send echo requests to the CHIP echo server
- Send on/off cluster requests to a CHIP device

> :warning: Connection to the CHIP device's soft AP will be a manual step until
> pairing is implemented.

## Steps to build Android CHIPTool

Pre-conditions: Have Android SDK 21 & NDK downloaded to your machine. Set the
\$ANDROID_HOME environment variable to where the SDK is downloaded and the
\$ANDROID_NDK_HOME environment variable to point to the NDK package is
downloaded.

ABIs and corresponding values for `TARGET_CPU`

| ABI         | TARGET_CPU |
| ----------- | ---------- |
| armeabi-v7a | arm        |
| arm64-v8a   | arm64      |
| x86         | x86        |
| x86_64      | x64        |

1. Checkout the CHIP repo

2. On the command line, `cd` into the top CHIP directory and run:

    ```shell
    sudo apt-get install -y swig # "brew install swig" for macOS.
    TARGET_CPU=arm64 ./scripts/examples/android_app.sh
    ```

    See table above for other values of `TARGET_CPU`.

3. Open the project in Android Studio and run **Sync Project with Gradle Files**

4. Click **Make Project** to build an Android package. Alternatively, on the command line, run:

    ```shell
    cd src/android/CHIPTool
    ./gradlew build
    ```

    The debug Android package will generated at _./app/build/outputs/apk/debug/app-debug.apk_
