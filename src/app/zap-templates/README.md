# ZAP generation templates

### What is this repo?

This directory contains generation templates for ZAP, ZCL Advanced Platform.

**IMPORTANT**: Changes to templates will affect all examples.

### How to configure an application

Before running ZAP, you need to have [npm](https://www.npmjs.com/) installed.
The best way is to simply download latest install of
[node](https://nodejs.org/en/download/) and you will get npm.

Run ZAP with UI to configure endpoints and clusters

```
./scripts/tools/zap/run_zaptool.sh
```

or

```
./scripts/tools/zap/run_zaptool.sh <path to *.zap file>
```

## Generating Files

### How to generate files for all applications

When any of the `.zapt` templates has been changed, all examples applications
need to be updated. You can regenerate all files with:

```
./scripts/tools/zap_regen_all.py
```

### How to generate files for a single application

By default generated files are located in a `zap-generated/` folder under
`zzz_generated/app-name/`.

The `zap-generated/` folder content is the output of the templates listed into
`app-templates.json`.

#### To generate the application `zap-generated/` folder the command is:

```
./scripts/tools/zap/generate.py <path to application *.zap file> -o zzz_generated/app-name/zap-generated

```

For example, to generate the `zzz_generated/lighting-app/zap-generated/` folder
the command is:

```
./scripts/tools/zap/generate.py examples/lighting-app/lighting-common/lighting-app.zap -o zzz_generated/lighting-app/zap-generated
```

### Note

If you are encountering issues while generating zap files, try running the
following commands

```
source ./scripts/activate.sh
./scripts/tools/zap/run_zaptool.sh
```

### How to generate files for a single application using custom templates

An application may use some custom templates to generate additional files.

In this case the file listing the templates needs to be passed as an argument to
`generate.py`.

To generate the application additional files, the command is:

```
./scripts/tools/zap/generate.py <path to *.zap file> -t <path to templates.json file>
```

For example, to generate `chip-tool` additional files the command is:

```
./scripts/tools/zap/generate.py src/controller/data_model/controller-clusters.zap -t examples/chip-tool/templates/templates.json
```

For more information please see the documentation under `docs/` in
[ZAP](https://github.com/project-chip/zap)
