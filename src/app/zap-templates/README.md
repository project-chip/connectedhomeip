# ZAP generation templates

### What is this repo?

This directory contains generation templates for ZAP, ZCL Advanced Platform.

**IMPORTANT**: Changes to templates will affect all examples.

### How to configure an application

Run ZAP with UI to configure endpoints and clusters

```
./scripts/tools/zap/configure.sh
```

### How to generate files for all applications

When any of the `.zapt` templates has been changed, all examples applications
need to be updated. You can regenerate all files with:

```
./scripts/tools/zap_regen_all.py
```

### How to generate files for a single application

By default generated files are located in a `gen/` folder next to the
application `zap` file that has been created during the application
configuration.

The `gen/` folder content is the output of the templates listed into
`app-templates.json`.

#### To generate the application `gen/` folder the command is:

```
./scripts/tools/zap/generate.py <path to application *.zap file>
```

For example, to generate `examples/lighting-app/lighting-common/gen/` folder the
command is:

```
./scripts/tools/zap/generate.py examples/lighting-app/lighting-common/lighting-app.zap
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
./scripts/tools/zap/generate.py examples/chip-tool/chip-tool.zap -t examples/chip-tool/templates/templates.json
```

For more information please see the documentation under `docs/` in
[ZAP](https://github.com/project-chip/zap)
