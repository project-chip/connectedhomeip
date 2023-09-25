# Ecosystem

This directory contains the capture process and log analysis implementations for
each ecosystem.

## Extending ecosystems

For each package in `capture/ecosystem`, the ecosystem loader expects a module
name matching the package name.  
This module must contain a single class which subclasses
`capture.base.EcosystemCapture`.

`/capture/ecosystem/play_services_user` contains a minimal example
implementation.

As another example, symlink `/res/plugin_demo/ecosystem/demo_ext_ecosystem`.

```
$ idt_go && ln -s $PWD/idt/res/plugin_demo/ecosystem/demo_ext_ecosystem/ idt/capture/ecosystem
$ idt capture -h
usage: idt capture [-h] [--platform {Android}] [--ecosystem {DemoExtEcosystem...
```
