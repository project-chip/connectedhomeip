# ZAP generation templates

## What is this repo?

This directory contains generation templates for ZAP, ZCL Advanced Platform.

**IMPORTANT**: Changes to templates will affect all examples.

# Useful command for CHIP

Run ZAP with UI to configure endpoints and clusters

```
cd ./third_party/zap/repo/
node src-script/zap-start.js --logToStdout  --gen ../../../src/app/zap-templates/app/templates.json
```

Generate files in headless mode

```
cd ./third_party/zap/repo/
node src-script/zap-generate.js -z ./zcl-builtin/silabs/zcl.json -g ../../../src/app/zap-templates/app-templates.json -i <path to *.zap file> -o <Path to /gen/ folder>
node src-script/zap-generate.js -z ./zcl-builtin/silabs/zcl.json -g ../../../src/app/zap-templates/chip-templates.json -i <path to *.zap file> -o ../../../
```

For more information please see the documentation under `docs/` in
[ZAP](https://github.com/project-chip/zap)
