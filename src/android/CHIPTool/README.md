This directory contains an Android Studio project for CHIPTool.

CHIPTool features: Scan a CHIP QR code and display payload information to the
user Send echo requests to the CHIP echo server Send on/off cluster requests to
a CHIP device

-   Connection to the chip device's soft AP will be a manual step until pairing
    is implemented.

# Steps to build CHIP for Android apps

Pre-conditions: Have Android SDK & NDK downloaded to your machine. Set the
\$ANDROID_HOME environment variable to where the SDK is downloaded and the
\$ANDROID_NDK_HOME environment variable to point to the NDK package is
downloaded.

Make sure that JAVA_HOME is set to the correct path.

ABIs and corresponding values for `target_cpu`

| ABI         | target_cpu |
| ----------- | ---------- |
| armeabi-v7a | arm        |
| arm64-v8a   | arm64      |
| x86         | x86        |
| x86-64      | x64        |

1. Checkout the CHIP repo

2. In commandline / Terminal, 'cd' into the top CHIP directory and run

    ```shell
    source scripts/activate.sh
    gn gen out/android_arm64 --args="target_os=\"android\" target_cpu=\"arm64\" android_ndk_root=\"${ANDROID_NDK_HOME}\" android_sdk_root=\"${ANDROID_HOME}\""
    ninja -C out/android_arm64 src/setup_payload/java src/controller/java
    ```

    See table above for other values of `target_cpu`.

3. You should see the generated SetupPayloadParser.jar under
   `out/android_arm64/lib` and libSetupPayloadParser.so under
   `out/android_arm64/lib/jni/arm64-v8a` in the output directory.

4. Copy the .jar and .so files into the Android project:

```shell
rsync -a out/android_arm64/lib/*.jar src/android/CHIPTool/app/libs
rsync -a out/android_arm64/lib/jni/* src/android/CHIPTool/app/src/main/jniLibs
```

5. Build OT Commissioner

    ```shell
    sudo apt-get install -y swig # "brew install swig" for macOS.

    git submodule update --init --recursive third_party/ot-commissioner/repo
    ABI=arm64-v8a API=21 ./third_party/ot-commissioner/build-android-libs.sh
    ## JAR and .so libraries will be copy to target directories.
    ```

6. 'Gradle sync' the Android project and run.
