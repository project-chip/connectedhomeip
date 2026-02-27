#!/usr/bin/env bash
set -euo pipefail

# Clean and recreate output directories for a reproducible build
rm -rf archives xcframeworks
mkdir -p archives xcframeworks

xcodebuild archive \
    -project MatterTvCastingBridge.xcodeproj \
    -scheme MatterTvCastingBridge \
    -configuration Release \
    -destination "generic/platform=iOS" \
    -archivePath "archives/MatterTvCastingBridge-iOS" \
    SKIP_INSTALL=0

xcodebuild archive \
    -project MatterTvCastingBridge.xcodeproj \
    -scheme MatterTvCastingBridge \
    -configuration Release \
    -destination "generic/platform=iOS Simulator" \
    -archivePath "archives/MatterTvCastingBridge-iOS_Simulator" \
    SKIP_INSTALL=0

xcodebuild -create-xcframework \
    -archive archives/MatterTvCastingBridge-iOS.xcarchive -framework MatterTvCastingBridge.framework \
    -archive archives/MatterTvCastingBridge-iOS_Simulator.xcarchive -framework MatterTvCastingBridge.framework \
    -output xcframeworks/MatterTvCastingBridge.xcframework
