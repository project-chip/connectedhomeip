# Changing Examples

The composition of most examples in the SDK is static and code generated.

The tool used to describe and change the composition of an example is called
ZAP. More information about ZAP and a walk-through of the tool can be found in
the [ZAP introduction](../zap_and_codegen/zap_intro.md). The composition of the
device is captured in a .zap file, which is readable by the ZAP tool. This is
then compiled into a human-readable .matter file, which is used to build the
static features of the example.

To change the composition of a device example, you need to

1. Change the zap file to implement your desired changes
2. Run the code generation tool to generate the .matter file
3. Re-build the example

## Changing the zap file in an example

Most examples in the SDK keep the .zap file in a sub-directory called
example-name-common (ex. lighting-common). To load an existing .zap file into
the ZAP tool, from the chip-root use

```
./scripts/tools/zap/run_zaptool.sh <location of the .zap file>
```

For example, to make changes to the lighting app, use:

```
./scripts/tools/zap/run_zaptool.sh examples/lighting-app/lighting-common/lighting-app.zap
```

This will open the ZAP GUI tool, which can be used to change the endpoint
composition, clusters, features, attributes, commands and events exposed by the
device.

Details of how to use the tool can be found in the
[ZAP Introduction](../zap_and_codegen/zap_intro.md).

## Running code generation

To compile the .matter file for use in building, use:

```
./scripts/tools/zap/generate.py <location of the .zap file>
```

For example, for changes to the lighting app, use:

```
./scripts/tools/zap/generate.py examples/lighting-app/lighting-common/lighting-app.zap
```

If there are changes to many .zap files, the following script can be used to
recompile the .zap files for all the examples and the controller.

```
 ./scripts/tools/zap_regen_all.py
```

## Rebuilding the example

After generating the .matter file, re-build the example. Instructions for
building examples are given in [Building your first example](./first_example.md)

## Ensuring device conformance

After changing the examples, it is important to ensure they remain spec
compliant. Although there are numerous certification tests to check the various
parts of the device, the tests most likely to be affected by changes to ZAP are
the conformance tests, which ensure that the device included meets the
conformance requirements for clusters and device types. To run conformance tests
against the example app, see [Testing](../testing/index.md). The tests that
ensure the device composition is spec compliant are found in
[Device Basic Composition Test](../../src/python_testing/TC_DeviceBasicComposition.py)
and
[Device Conformance Tests](../../src/python_testing/TC_DeviceConformance.py).
