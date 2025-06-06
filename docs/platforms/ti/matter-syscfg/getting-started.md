# Get started with SysConfig

System Configuration Tool (SysConfig) is a graphical interface for configuring
your project. Configuration files, C source files and header files are generated
based on the parameters configured in the SysConfig dashboard.

SysConfig should be used as a standalone tool for script-based builds (such as
for Matter).

The document will help for getting started in tailoring the `syscfg` file to
your application needs.

## Using Stand-alone SysConfig

You can find the standalone SysConfig tool on the
[SysConfig Standalone Installer Download Page](https://www.ti.com/tool/SYSCONFIG).

For a given device, the location of the Matter `syscfg` file is located here:
`/examples/platform/ti/sysconfig/`. For instance, an example running on
LP-EM-CC1354P10-6 has its `syscfg` located
[here](../../../../examples/platform/ti/sysconfig/chip_cc1354p10_6.syscfg).

This file is used by Matter build scripts, when it invokes SysConfig CLI. In
order to edit the file for your custom application configurations, you'll need
to open the file with the SysConfig GUI. Before doing so, you should add the
following line above the `/* Modules */` comment:

```
// @cliArgs --product <path to SDK>/.metadata/product.json --board /ti/boards/LP_EM_CC1354P10_6 --rtos freertos
```

As an example, after adding the necessary `<path to SDK>`, it would look like:

```
// @cliArgs --product /home/Documents/repos/connectedhomeip/third_party/ti_simplelink_sdk/repo_cc13xx_cc26xx/.metadata/product.json --board /ti/boards/LP_EM_CC1354P10_6 --rtos freertos
```

If the file (product.json) is not found, then you may need to first initialize
the submodules, which should pull in the code for
`ti_simplelink_sdk/repo_cc13xx_cc26xx` (and other relevant submodules):

```
git submodule update --init
```

Now you can open the file in SysConfig GUI. Once you are done, remove the
`@cliArgs` line then save the file. Now the Matter build scripts will use the
updated `.syscfg` file.

## Viewing SysConfig Output

To preview the code to be generated reference the `Generated Files` image, the
button at the top of the `Configuration` pane you will find a list of Generated
Files. After selecting a file you will be able, in real-time, to identify how
the changes in the GUI affect the generated files. This will help you review the
relationship between the GUI and the code.

<div style="text-align: center;">
  <img src="images/generated_files_ble.png" alt="Generated Files" width=30%>
  <div class="caption">Generated Files</div>
</div>

Whenever you re-build the project, SysConfig will re-generate the files. Because
of this, any changes made directly in the generated files will be overwritten.

## Unique Application Configurations

For a given device, any applications based on that device will share the same
`.syscfg` file. Consider the case where you have two applications, say
lighting-app and lock-app, and both are based on the same device, say CC2674R10.
By default, the same
[chip_cc2674r10.syscfg](../../../../examples/platform/ti/sysconfig/chip_cc2674r10.syscfg)
would be pulled in by both applications. But if pin functions are not identical
across the two applications, it'd be better to have two separate files, one per
application.

One way to achieve this:

1. Copy the file
   [chip_cc2674r10.syscfg](../../../../examples/platform/ti/sysconfig/chip_cc2674r10.syscfg)
   to the local application folder (for lighting-app you can copy it at
   `examples/lighting-app/ti/cc13x4_26x4/`).
2. Then in [BUILD.gn](../../../../examples/lighting-app/ti/cc13x4_26x4/BUILD.gn)
   modify the respective path of `sources` in `ti_sysconfig("sysconfig")` to the
   local copy from step 1 (e.g.
   `"${chip_root}/examples/lighting-app/ti/cc13x4_26x4/chip_cc2674r10.syscfg"`)
