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
version by the SDK (see `scripts/setup/zap.json` and
`scripts/tools/zap/zap_execution.py` for the minimum supported version).

Beyond basic zap file selection, there are also `.json` zap settings that define
additional cluster info: source XML files, sdk-access methods and data types.
There are only two such files currently in use:

-   `src/app/zap-templates/zcl/zcl.json` is the **default** one
-   `src/app/zap-templates/zcl/zcl-with-test-extensions.json` is used by
    `all-clusters-app` to show how a cluster extension may be configured with
    minimal changes from `zcl.json` (but it is different)

### Installing zap and environment variables

ZAP is generally installed as a third-party tool via CIPD during the build
environment bootstrap (see `scripts/setup/zap.json`), which makes `zap-cli`
available in `$PATH` when running in a build environment.

When matter scripts need to invoke `zap-cli` (for code generation) or `zap` (to
start the UI tool), they make use of the following environment variables to
figure out where the zap tool is located (in order of precedence):

-   if `$ZAP_DEVELOPMENT_PATH` is set, code assumes you are running zap from
    source. Use this if you develop zap. Zap has to be bootstrapped (generally
    `npm ci` but check zap documentation for this. Some scripts have a
    `--run-bootstrap` command line argument to do this for you)

-   if `$ZAP_INSTALL_PATH` is set, code assumes that `zap` or `zap-cli` is
    available in the given path. This is generally an unpacked release.

-   otherwise, scripts will assume `zap`/`zap-cli` is in `$PATH` (this is the
    case when running in a bootstrapped environment)

### Using a UI to edit `.zap` files

Generally you need to invoke zap with appropriate zcl and generate arguments.
Most of code generation is app specific, so you generally want something of the
form
`--gen src/app/zap-templates/app-templates.json --zcl $ZCL_JSON_FILE $ZAP_FILE_TO_EDIT`

Since this is tedious to type, the SDK provides a
`scripts/tools/zap/run_zaptool.sh` script to automate this:

```bash
# Ensure `zap` is in $PATH, specify the `--zap ZAP` option to `run_zaptool.sh` to specify the path to `zap`, set $ZAP_INSTALL_PATH, or set $ZAP_DEVELOPMENT_PATH
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
these files is available at `scripts/py_matter_idl` and `scripts/codegen.py`.
You can read the
[scripts/py_matter_idl/matter_idl/README.md](../scripts/py_matter_idl/matter_idl/README.md)
for details of how things work.

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
/scripts/tools/zap/generate.py                       \
    examples/bridge-app/bridge-common/bridge-app.zap
```

The above will just generate a `<app>.matter` file along side the `.zap` file,
as this is the only file that requires updates for applications. You can code
generate other things by passing in the `-t/--templates` argument to
generate.py. In those cases, you may also need to specify an output directory
via `-o/--output-dir`.

#### Flow for updating an application zap file:

```
# use zap UI to edit the file (or edit zap file in any other way)
./scripts/tools/zap/run_zaptool.sh $PATH_TO_ZAP_FILE

# re-generate .matter file. Note that for .matter file generation, output
# directory is NOT used
./scripts/tools/zap/generate.py $PATH_TO_ZAP_FILE
```

### Compile-time code generation / pre-generated code

A subset of code generation (both `codegen.py` and `zap-cli`) is done at compile
time or can use pre-generated output (based on gn/cmake arguments)

Rules for how `generate.py`/`codegen.py` is invoked at compile time are defined
at:

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
    of golden image tests in `scripts/py_matter_idl/matter_idl/tests`

The script to trigger code pre-generation is `scripts/codepregen.py` and
requires the pre-generation output directory as an argument

```bash
scripts/codepregen.py ${OUTPUT_DIRECTORY:-./zzz_pregenerated/}

# To generate a single output you can use `--input-glob`:

scripts/codepregen.py --input-glob "*all-clusters*" --input-glob "*controller*" ${OUTPUT_DIRECTORY:-./zzz_pregenerated/}
```

### External applications/zap files

#### Ensure you have a `.matter` file

Code generation generally will use both `.zap` or `.matter` files. If you only
have a `.zap` file, you can create the corresponding `.matter` file via:

```bash
scripts/tools/zap/generate.py ${ZAP_FILE_PATH}
```

The above will use the template `src/app/zap-templates/matter-idl.json` to
generate a `.matter` file corresponding to the input `.zap` file.

`.matter` files are designed to be human readable. It is recommended to take a
look at the generated file and see if it contains what is expected and also lint
it. If anything seems wrong, the `.zap` file should be fixed (`.matter`
represents the content of `.zap`). To lint use:

```bash
scripts/idl_lint.py ${MATTER_FILE_PATH}
```

#### Running pre-generation

If you have zap files outside the CHIP repository (i.e. not in `src` or
`examples`) you should provide the root of your application source.

```bash
scripts/codepregen.py --external-root ${PATH_TO_SOURCE_ROOT} ${OUTPUT_DIRECTORY:-./zzz_pregenerated/}
```

NOTE: `$PATH_TO_SOURCE_ROOT` should be a top-level directory containing
zap/matter files as the code pre-generation will generate files based on the
path inside the root:

-   if files are `$PATH_TO_SOURCE_ROOT/some/path/foo.zap` this will generate
    files into `$OUTPUT_DIRECTORY/some/path/foo/...`

### Using pre-generated code

Instead of generating code at compile time, the chip build system accepts usage
of a pre-generated folder. It assumes the structure that `codepregen.py`
creates. To invoke use:

-   `build_examples.py` builds accept `--pregen-dir` as an argument, such as:

    ```shell
    ./scripts/build/build_examples.py --target $TARGET --pregen-dir $PREGEN_DIR build
    ```

-   `gn` builds allow setting `chip_code_pre_generated_directory` as an
    argument, such as:

    ```shell
    gn gen --check --fail-on-unused-args --args='chip_code_pre_generated_directory="/some/pregen/dir"'
    ```

-   `cmake` builds allow setting `CHIP_CODEGEN_PREGEN_DIR` variable (which will
    get propagated to the underlying `gn` builds as needed), such as:

    ```shell

    west build --cmake-only \
        -d /workspace/out/nrf-nrf5340dk-light \
        -b nrf5340dk_nrf5340_cpuapp \
        /workspace/examples/lighting-app/nrfconnect
        -- -DCHIP_CODEGEN_PREGEN_DIR=/some/pregen/dir

    idf.py -C examples/all-clusters-app/esp32 \
        -B /workspace/out/esp32-m5stack-all-clusters \
        -DCHIP_CODEGEN_PREGEN_DIR=/some/pregen/dir \
        reconfigure

    cmake -S /workspace/examples/lighting-app/mbed \
          -B /workspace/out/mbed-cy8cproto_062_4343w-light \
          -GNinja \
          -DMBED_OS_PATH=/workspace/third_party/mbed-os/repo \
          -DMBED_OS_PATH=/workspace/third_party/mbed-os/repo \
          -DMBED_OS_POSIX_SOCKET_PATH=/workspace/third_party/mbed-os-posix-socket/repo \
          -DCHIP_CODEGEN_PREGEN_DIR=/some/pregen/dir
    ```

### Code generation unit testing

Code generation is assumed stable between builds and the build system aims to
detect changes in code gen using golden image tests.

#### `codegen.py` tests

These tests run against golden inputs/outputs from `scripts/idl/tests`.

`available_tests.yaml` contains the full list of expected generators and outputs
and the test is run via `test_generators.py`. Use the environment variable
`IDL_GOLDEN_REGENERATE` to force golden image replacement during running of
`ninja check`:

```shell
IDL_GOLDEN_REGENERATE=1 ninja check
```

#### `generate.py` tests

These tests run against golden inputs/outputs from `scripts/tools/zap/tests`.

`available_tests.yaml` contains the full list of expected generators and outputs
and the test is run via `scripts/tools/zap/test_generate.py`. Use the
environment variable `ZAP_GENERATE_GOLDEN_REGENERATE` to force golden image
replacement during running of `ninja check`.

```shell
ZAP_GENERATE_GOLDEN_REGENERATE=1 ninja check
```

Alternatively, the golden image can also be re-generated by running the
stand-alone test in a bootstrapped environment:

```shell
./scripts/tools/zap/test_generate.py --output out/gen --regenerate
```
