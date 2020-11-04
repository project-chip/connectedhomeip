# ZAP generation templates

## What is this repo?

This directory contains generation templates for ZAP, ZCL Advanced Platform.

**IMPORTANT**: Changes to templates will affect all examples.

# Usefull command for CHIP

Run ZAP with UI to configure endpoints and clusters

```
<Inside /third_party/zap/repo/>
node src-script/zap-start.js --logToStdout  --gen ../templates/chip-templates.json
```

Generate files in headless mode

```
<Inside /third_party/zap/repo/>
node src-script/zap-generate.js -z ./zcl-builtin/silabs/zcl.json -g ../templates/chip_templates.json -i <path to *.zap file> -o <Path to /gen/ folder>
```

For more information please see the documentation under
third_party/zap/repo/docs/
