-   To build the Darwin Matter.framework, use
    `xcodebuild -project Matter.xcodeproj -scheme "Matter Framework"`, varying
    as needed.
-   To regenerate the zap-generated files for the Darwin Matter Framework, use
    `./scripts/tools/zap_regen_all.py --type specific` from the
    `connectedhomeip` repository root.
