This directory contains an Android Studio project for CHIPTool.

CHIPTool features: Scan a CHIP QR code and display payload information to the
user Send echo requests to the CHIP echo server Send on/off cluster requests to
a CHIP device

-   Connection to the chip device's soft AP will be a manual step until pairing
    is implemented.

===========================================

Steps to build CHIP for Android apps

Pre-conditions: Have Android NDK downloaded to your machine. Set
\$ANDROID_NDK_HOME environment variable to point to the NDK package is
downloaded.

1. Checkout the CHIP repo

2. In commandline / Terminal, 'cd' into the top CHIP directory

3. Run './bootstrap'

4. Exit out of the checked out CHIP repo directory and using 'mkdir [some-dir]'
   create a new empty one where you want the generated output files to go.

5. 'cd' into this newly created directory

6. Export the following variables

ABIs that can be used as TARGET:

| ABI         | Triple                   |
| ----------- | ------------------------ |
| armeabi-v7a | armv7a-linux-androideabi |
| arm64-v8a   | aarch64-linux-android    |
| x86         | i686-linux-android       |
| x86-64      | x86_64-linux-android     |

NDK OS Variants:

| NDK OS Variant | Host Tag       |
| -------------- | -------------- |
| macOS          | darwin-x86_64  |
| Linux          | linux-x86_64   |
| 32-bit Windows | windows        |
| 64-bit Windows | windows-x86_64 |

```
OS_VARIANT=darwin-x86_64 // Specify OS variant corresponding to system
export TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/$OS_VARIANT
export TARGET=aarch64-linux-android // Use desired ABI
export API=21
export AR=$TOOLCHAIN/bin/$TARGET-ar
export AS=$TOOLCHAIN/bin/$TARGET-as
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
export LD=$TOOLCHAIN/bin/$TARGET-ld
export RANLIB=$TOOLCHAIN/bin/$TARGET-ranlib
export STRIP=$TOOLCHAIN/bin/$TARGET-strip
export JNI_INCLUDE_DIRS=$ANDROID_NDK_HOME/sysroot/usr/include
```

Make sure that JAVA_HOME is set to the correct path.

Note: Regardless of where JAVA_HOME points, always use the JNI headers from the
Android NDK, and only include the top-most directory (include), not the system
directory (include/linux). Because the NDK mixes the JNI headers in with the
linux headers, listing the system directory in the -I flags will result in
strange compilation errors. And unlike the standard Java jni.h, the jni.h that
comes with the Android NDK does not depend on any system-specific JNI header
files (e.g. jni_md.h). Thus only the top-level include directory is needed.

7. Then run '../connectedhomeip/configure --host=\$TARGET --with-crypto=mbedtls
   --enable-tests=no --enable-shared --with-device-layer=none
   --with-chip-project-includes=../connectedhomeip/config/android'

8. Finally 'make'

9. You should see the generated SetupPayloadParser.jar under
   src/setup_payload/java and libSetupPayloadParser.so under
   src/setup_payload/java/.libs folders in the output directory.

Drop the .jar in the libs folder in the Android project.

Drop the .so in the ABI-specific jniLibs folder in the Android project, eg.
jniLibs/arm64-v8a

'Gradle sync' the Android project and run.

10. You will also need the "libc++\_shared.so" file in the jniLibs folder. This
    file comes packaged with Android NDK and can be found under
    `$ANDROID_NDK_HOME/sources/cxx-stl/llvm-libc++/libs/$TARGET`.

    (Eventually hoping to not have to include this .so, but that needs some more
    tweaking of the Android automake build rules. Include it in the interim to
    be able to build the Android app).

11. Before running `gradle sync`, setup ot-commissioner

    ```shell
    cd third_party/ot-commissioner/repo/
    ./script/bootstrap.sh

    ## Install SWIG 4.0
    brew install swig@4
    ```
