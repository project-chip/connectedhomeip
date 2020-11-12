This directory contains an Android Studio project for CHIPTool.

CHIPTool features: Scan a CHIP QR code and display payload information to the
user Send echo requests to the CHIP echo server Send on/off cluster requests to
a CHIP device

-   Connection to the chip device's soft AP will be a manual step until pairing
    is implemented.

# Steps to build CHIP for Android apps

Pre-conditions: Have Android SDK 21 & NDK 21.0.6113669 downloaded to your machine. Set the
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

2. In commandline / Terminal, 'cd' into the top CHIP directory and run

    ```shell
    sudo apt-get install -y swig # "brew install swig" for macOS.
    TARGET_CPU=arm64 ./scripts/examples/android_app.sh
    ```

    See table above for other values of `TARGET_CPU`.

3. 'Gradle sync' the Android project and run.
