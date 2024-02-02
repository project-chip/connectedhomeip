## NXP builder

The builder aims to offer a scalable way of defining targets for Matter
applications running on NXP platforms. This is achieved through a data-driven
approach, where boards, application and modifiers are defined in JSON files,
which are then used by the build examples script to generate and compile
corresponding targets.

### Overview

The JSON files are placed under the `platforms` folder. Each platform should
define its own JSON file, e.g. `k32w0.json` for K32W0.

Please note that there is a `common.json` file, which should contain modifiers
that are platform agnostic, such as Matter core specific gn args, e.g.
`chip_enable_ble`.

### Modifier vs variable modifier

A modifier is an argument that affects the target configuration. It can be used
to enable or disable certain features or to set compilation flags.

Normally, a modifier will have a boolean value, represented by `0/1` or
`false/true`. A variable modifier is a special modifier that can have any value.
In this case, the user has to specify the value in the `build_examples.py`
command.

The `build_examples.py` has an optional argument, `--variable-modifiers`, which
can be used to specify additional variable modifiers for a certain target:

```
./scripts/build/build_examples.py --target nxp-k32w1-contact-sensor-app-crypto-platform --variable-modifiers "sw-version=50000" build --copy-artifacts-to out/artifacts
```

Please note that the target name does not contain the modifier id `sw-version`.
The variable modifiers can be part of the target name or they can be omitted. It
is up to the user to specify it, in which case additional regex rules will be
checked when generating the targets, based on the `except if`/`only if`
statements.

The variable modifiers value can be expanded to multiple values:

```
--variable-modifiers "foo=bar dead=beef"
```

### Add support for another platform

A JSON schema is provided for standardizing the format. Modifiers that are
common between two or more platforms should be added in the `common.json` file,
to avoid duplicating the modifiers.

After creating the JSON file, the user can run
`build_examples.py targets | grep nxp` to obtain a list of supported targets and
manually check that the modifiers are appended to the list.
