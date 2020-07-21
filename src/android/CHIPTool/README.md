This directory contains an Android Studio project for CHIPTool.

CHIPTool features: Scan a CHIP QR code and display payload information to the
user Send echo requests to the CHIP echo server Send on/off cluster requests to
a CHIP device

-   Connection to the chip device's soft AP will be a manual step until pairing
    is implemented.

===========================================

Steps to build CHIP for Android apps

Pre-conditions: Have Android NDK downloaded to your machine. Set $ANDROID_NDK_HOME environment variable to point to the NDK package is downloaded.

1. Checkout the CHIP repo

2. In commandline / Terminal, 'cd' into the top CHIP directory

3. Run './bootstrap -w make'

4. Exit out of the checked out CHIP repo directory and using 'mkdir [some-dir]' create
a new empty one where you want the generated output files to go.

5. 'cd' into this newly created directory

6. Export the following variables

ABIs that can be used as TARGET
    armeabi-v7a:  armv7a-linux-androideabi
    arm64-v8a:	  aarch64-linux-android
    x86:	        i686-linux-android
    x86-64:	      x86_64-linux-android

NDK OS Variants:
    macOS:	        darwin-x86_64
    Linux:	        linux-x86_64
    32-bit Windows	windows
    64-bit Windows	windows-x86_64

export TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/darwin-x86_64 // Use the corresponding OS variant here.
export TARGET=aarch64-linux-android // Use desired ABI
export API=21
export AR=$TOOLCHAIN/bin/$TARGET-ar
export AS=$TOOLCHAIN/bin/$TARGET-as
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
export LD=$TOOLCHAIN/bin/$TARGET-ld
export RANLIB=$TOOLCHAIN/bin/$TARGET-ranlib
export STRIP=$TOOLCHAIN/bin/$TARGET-strip

7. Then run '../connectedhomeip/configure --host=$TARGET --with-crypto=mbedtls --enable-tests=no --enable-shared'

8. Finally 'make'

You should see the generated SetupPayloadParser.jar under src/setup_payload/java and libSetupPayloadParser.so under
src/setup_payload/java/.libs folders in the output directory.

Drop the .jar in the libs folder in the Android project.
Drop the .so in the ABI-specific jniLibs folder in the Android project, eg. jniLibs/arm64-v8a
'Gradle sync' the Android project and run.

You will also need the "libc++_shared.so" file in the jniLibs folder. This file comes packaged with Android NDK
and can be found under $ANDROID_NDK_HOME/sources/cxx-stl/llvm-libc++/libs/$TARGET
(Eventually hoping to not have to include this .so, but that needs some more tweaking of the Android automake build rules.
Include it in the interim to be able to build the Android app).
