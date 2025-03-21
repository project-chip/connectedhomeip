#!/bin/bash
set -e

CHIP_ROOT=$(_normpath "$(dirname "$0")/..")
echo $CHIP_ROOT
OUTPUT_ROOT="$CHIP_ROOT/out/python_lib"
echo $OUTPUT_ROOT

# webrtc directory
WEBRTC="$CHIP_ROOT/third_party/webrtc/"

cd $WEBRTC
gclient sync
cd -

echo "Running gn gen"
gn --root="$WEBRTC/src" gen "$OUTPUT_ROOT/webrtc" --args='target_os = "linux" target_cpu = "x64" rtc_include_tests = false rtc_use_x11=false rtc_use_h264 = true is_component_build = false use_rtti = true use_custom_libcxx=false use_custom_libcxx_for_host = false rtc_enable_protobuf = false is_clang = true treat_warnings_as_errors = false rtc_include_pulse_audio=false rtc_build_examples = false use_cxx17=true cpp_std="c++17" '

ninja -C "$OUTPUT_ROOT/webrtc"
echo "Webrtc build is complete and ready to use"
