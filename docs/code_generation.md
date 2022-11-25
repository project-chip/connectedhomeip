# Code generation

## Code generation inputs (`*.zap` files)

Matter code relies on code generation for cluster-specific data types and
callbacks. Generally this is split into:

-   Data serialization for structures/lists/commands. This applies to both
    client-side and server-side structures and objects
-   Callback setup using the Ember-based framework. This generally applies to
    server-side processing and the code generation defines what processing needs
    to be done when a specific command is received or an attribute is read and
    what memory should be allocated for storing cluster attributes

Code generation depends on the clusters that are needed by an application. Every
application configures the specific set of endpoints and clusters it needs based
on the device type it supports. The selection of the supported clusters and
attributes (as optional attributes may be omitted to save memory) is generally
stored in `*.zap` files.

The selection of enabled clusters and files is done using
[ZAP](https://github.com/project-chip/zap). You can download a recent release of
zap from its [releases page](https://github.com/project-chip/zap/releases). It
is recommended to download a release that is in sync with the currently in use
version by the SDK (see `integrations/docker/images/chip-build/Dockerfile` and
check the `ZAP_VERSION` setting).

Beyond basic zap file selection, there are also `.json` zap settings that define
additional cluster info: source XML files, sdk-access methods and data types.
There are only two such files currently in use:

-   `src/app/zap-templates/zcl/zcl.json` is the **default** one
-   `src/app/zap-templates/zcl/zcl-with-test-extensions.json` is used by
    `all-clusters-app` to show how a cluster extension may be configured with
    minimal changes from `zcl.json` (but it is different)

### Installing zap and environment variables

Matter scripts may need to invoke `zap-cli` (for code generation) or `zap` (to
start the UI tool). For this, scrips need to know where to find the commands. In
the following order, the scripts process these environment variables:

-   if `$ZAP_DEVELOPMENT_PATH` is set, code assumes you are running zap from
    source. Use this if you develop zap. Zap has to be bootstrapped (generally
    `npm ci` but check zap documentation for this. Some scripts have a
    `--run-bootstrap` command line argument to do this for you)

-   if `$ZAP_INSTALL_PATH` is set, code assumes that `zap` or `zap-cli` is
    available in the given path. This is generally an unpacked release.

-   otherwise, scripts will assume `zap`/`zap-cli` is in `$PATH`

### Using a UI to edit `.zap` files

Generally you need to invoke zap with appropriate zcl and generate arguments.
Most of code generation is app specific, so you generally want something of the
form
`--gen src/app/zap-templates/app-templates.json --zcl $ZCL_JSON_FILE $ZAP_FILE_TO_EDIT`

Since this is tedious to type, the SDK provides a
`scripts/tools/zap/run_zaptool.sh` script to automate this:

```bash
# Ensure zap is in $PATH or set $ZAP_INSTALL_PATH or $ZAP_DEVELOPMENT_PATH
./scripts/tools/zap/run_zaptool.sh examples/lighting-app/lighting-common/lighting-app.zap
```

### Human-readable code generation inputs (`*.matter`)

`.zap` files are large json files that are generally not human readable. As a
result, the Matter SDK also keeps an equivalent `*.matter` file along side
`.zap` files that contain the same data as `.zap` files, targeted specifically
for matter:

-   They are designed to be human readable, looking like a IDL (think protobuf
    or android `aidl`, thrift idl etc.)

-   We strive to make them contain only Matter-specific data (`.zap` files
    contain more generic data and is designed to be ZigBee backwards compatible)

Currently `.matter` files are generated from `.zap` files during the application
specific codegen.

### `*.matter` parsing and codegen

`*.matter` files are both human and machine readable. Code that can process
these files is available at `scripts/idl` and `scripts/codegen.py`. You can read
the [scripts/idl/README.md](../scripts/idl/README.md) for details of how things
work.

`scripts/codegen.py` can generate various outputs based on an input `*.matter`
file.

The split between `.zap` and `.matter` currently exists as an experiment of code
generation technologies. Currently `.matter`-based Python code generation:

-   has fewer third party dependencies than `zap`, which installs a significant
    number of `npm` packages.
-   runs significantly faster than zap
-   offers more flexible code generation (can generate multiple files per
    cluster for example, without which some compiles would run out of RAM on
    large compilations)
-   has a more flexible templating language
-   has human readable (and potentially editable) input
-   is more easily provable deterministic (`zap` uses an underlying sqlite
    database and some legacy assumptions from zigbee have historically caused
    non-determinism)
-   uses a synchronous processing model which is potentially easier to develop
    for
-   has lower complexity, is unit tested and uses typing extensively

Ideally, the project would be to have a single code generation method in the
long term that has all the benefits and none of the drawbacks. We are not there
yet, however we likely want:

-   Flexible codegen (we will need to split output by clusters or other rules)
-   Human-readable inputs that enable code reviews and audits
-   Rules that a script can validate based on CSA data model (ensure mandatory
    attribute settings are followed, ensure proper device type adherence, ensure
    correct cluster and data type definitions)
-   Easy to maintain and develop for chosen languages/templates/codegen in
    general

## Code generation outputs and templates

Code that is generated:

-   **Application-specific**:

    -   ZAP generation is based on `.zap` files in `examples/` and generates
        server-side processing data: what cluster callbacks to set up, what RAM
        to reserve for attribute storage etc.

    -   `Codegen.py` will also generate a subset of application-specific files

-   **Automated tests**: embedded client-side tools (`chip-tool` and
    `darwin-framework-tool`) generate test-definition data. Each use their own
    `examples/${TOOL}/templates/tests/templates.json` to drive what gets
    generated.

-   **Controller clusters** target: the file
    `src/controller/data_model/controller-clusters.zap` contains a set of
    cluster selections to which all applications would potentially have access.
    These are generally used as `all clusters selection` and the intent is to
    allow any application to access any cluster as a `client side`.

    Client/controllers will codegen based on this, like **tools**, **tests**,
    **java**, **python** etc.

## Running codegen

### ZAP file generation

Generating all possible code (all categories above) using zap tool can be done
via:

```bash
./scripts/tools/zap_regen_all.py
```

This can be slow (several minutes). The regen tool allows selection of only
tests so that yaml test development goes faster.

```bash
./scripts/tools/zap_regen_all.py --type tests
./scripts/tools/zap_regen_all.py --type tests --tests chip-tool
```

Additionally, individual code regeneration can be done using
`./scripts/tools/zap/generate.py`:

```bash
/scripts/tools/zap/generate.py examples/bridge-app/bridge-common/bridge-app.zap -o zzz_generated/bridge-app/zap-g
enerated
```

### `*.matter` code generation

`*.matter` code generation can be done either at compile time or it can use
pre-generated output.

Rules for how `codegen.py` is invoked and how includes/sources are set are
defined at:

-   `src/app/chip_data_model.cmake`
-   `src/app/chip_data_model.gni`

Additionally, `build/chip/esp32/esp32_codegen.cmake` adds processing support for
the 2-pass cmake builds used by the Espressif `idf.py` build system.

## Pre-generation

Code pre-generation can be used:

-   when compile-time code generation is not desirable. This may be for
    importing into build systems that do not have the pre-requisites to run code
    generation at build time or to save the code generation time at the expense
    of running code generation for every possible zap/generation type
-   To check changes in generated code across versions, beyond the comparisons
    of golden image tests in `scripts/idl/tests`

The script to trigger code pre-generation is `scripts/code_pregenerate.py` and
requires the pre-generation output directory as an argument

```bash
scripts/code_pregenerate.py ${OUTPUT_DIRECTORY:-./zzz_pregenerated/}
```
