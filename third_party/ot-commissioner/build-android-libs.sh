#!/bin/bash

readonly CUR_DIR="$(dirname "$(realpath -s "$0")")"

set -e

case "$TARGET_CPU" in
    arm)
        TARGET_ABI="armeabi-v7a"
        ;;
    arm64)
        TARGET_ABI="arm64-v8a"
        ;;
    x86)
        TARGET_ABI="x86"
        ;;
    x64)
        TARGET_ABI="x86-64"
        ;;
    *)
        echo "invalid TARGET_CPU value: $TARGET_CPU"
        exit 1
        ;;
esac

readonly BUILD_DIR=".build_$TARGET_CPU"

cd "$CUR_DIR"

mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR"
cmake -GNinja \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME"/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI="$TARGET_ABI" \
    -DANDROID_ARM_NEON=ON \
    -DANDROID_NATIVE_API_LEVEL=24 \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_CXX_STANDARD=11 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DOT_COMM_ANDROID=ON \
    -DOT_COMM_JAVA_BINDING=ON \
    -DOT_COMM_APP=OFF \
    -DOT_COMM_TEST=OFF \
    -DOT_COMM_CCM=OFF \
    ../repo

ninja
cd ../

rm -rf "$BUILD_DIR"/libs && mkdir -p "$BUILD_DIR"/libs

## Create JAR library
javac -source 8 -target 8 "$BUILD_DIR"/src/java/io/openthread/commissioner/*.java

cd "$BUILD_DIR"/src/java
find ./io/openthread/commissioner -name "*.class" | xargs jar cvf ../../libs/libotcommissioner.jar
cd ../../../

## Copy shared native libraries
rsync -a "$BUILD_DIR"/src/java/libcommissioner-java.so "$BUILD_DIR"/libs

rsync -a "$BUILD_DIR"/libs/libotcommissioner.jar ../../src/android/CHIPTool/app/libs
rsync -a "$BUILD_DIR"/libs/*.so ../../src/android/CHIPTool/app/src/main/jniLibs/"$TARGET_ABI"
