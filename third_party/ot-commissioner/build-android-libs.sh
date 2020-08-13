#!/bin/bash

readonly CUR_DIR=$(dirname "$(realpath -s $0)")

set -e

case "${TARGET}" in
    armv7a-linux-androideabi)
        ABI="armeabi-v7a"
        ;;
    aarch64-linux-android)
        ABI="arm64-v8a"
        ;;
    i686-linux-android)
        ABI="x86"
        ;;
    x86_64-linux-android)
        ABI="x86-64"
        ;;
    *)
        echo "invalid TARGET value: ${TARGET}"
        exit 1
esac

cd "${CUR_DIR}"

## Install dependencies.
[ ! -d build/ ] && ./repo/script/bootstrap.sh

mkdir -p build && cd build
cmake -GNinja                                                                               \
    -DCMAKE_TOOLCHAIN_FILE="${ANDROID_NDK_HOME}"/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI="${ABI}"                                                                  \
    -DANDROID_ARM_NEON=ON                                                                   \
    -DANDROID_NATIVE_API_LEVEL="${API}"                                                     \
    -DBUILD_SHARED_LIBS=ON                                                                  \
    -DCMAKE_CXX_STANDARD=11                                                                 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON                                                        \
    -DCMAKE_BUILD_TYPE=Release                                                              \
    -DOT_COMM_ANDROID=ON                                                                    \
    -DOT_COMM_JAVA_BINDING=ON                                                               \
    -DOT_COMM_APP=OFF                                                                       \
    -DOT_COMM_TEST=OFF                                                                      \
    -DOT_COMM_CCM=OFF                                                                \
    ../repo

ninja
cd ../

rm -rf libs && mkdir -p libs

## Create JAR library
javac -source 8 -target 8 build/src/java/io/openthread/commissioner/*.java

cd build/src/java
find ./io/openthread/commissioner -name "*.class" | xargs jar cvf ../../../libs/libotcommissioner.jar
cd ../../../

## Copy shared native libraries
for lib in $(find ./build -name "*.so"); do
    ## Avoid copying symblink files.
    [ ! -L "$lib" ] && cp "$lib" libs
done

cp libs/libotcommissioner.jar ../../src/android/CHIPTool/app/libs
cp libs/*.so ../../src/android/CHIPTool/app/src/main/jniLibs/"$ABI"
