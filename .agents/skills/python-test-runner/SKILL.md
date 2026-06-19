---
name: python-test-runner
description: >-
    Guidelines and instructions for building prerequisite example applications
    and running python-based integration and certification tests located in
    `src/python_testing/`. Use this skill to run important regression tests
    during developement or,  when building new example apps, or when there is a
    need to execute python test scripts or "certification test scripts"  or
    mentions of run_python_test.py and local.py.
---

# Python Test Runner

This skill provides expert guidelines for running Python-based integration and
certification tests located in `src/python_testing/`.

## Prerequisite 1: Activating the Python Environment

Before running any Python tests, you must compile (if one doesn't exist in out/)
and activate the project's Python virtual environment.

> [!IMPORTANT] Different runner methods may expect different virtual
> environments. The modern local runner script (`local.py`) hardcodes `out/venv`
> as its Python environment. Ensure you compile and activate this specific
> folder:

```bash
./scripts/build_python.sh -i out/venv --enable_ipv4 true
source out/venv/bin/activate
```

Be sure to check the arguments in the `build_python.sh` script before setting up
the virtual environment. Run the shell script with the `--help` argument to see
all available options that may be relevant for the tests.

---

## Prerequisite 2: Building the Example Application

Matter Python tests execute against a compiled device simulator application.
Before you can run any test locally, you must identify, map, and compile the
required application from source.

> [!NOTE] For many modern Python tests, the use of the **`all-devices-app`** is
> preferred, which allows simulating specific device type(s) on endpoints.
> However, many existing tests continue to use **`chip-all-clusters-app`**, and
> some domain-specific tests may require other specialized apps (e.g.,
> `chip-energy-management-app`).

### 1. Identify the Required Application

Open the Python test file (e.g., `src/python_testing/TC_FAN_3_3.py`) and locate
the **`=== BEGIN CI TEST ARGUMENTS ===`** block near the top of the file. Under
the `app` parameter, you will see the application defined as an environment
variable, such as:

-   `app: ${ALL_CLUSTERS_APP}`
-   `app: ${ALL_DEVICES_APP}`

These environment variables map to specific compiled applications. Locally, you
will need to know where these apps are built (usually in the `out/` directory)
to reference them correctly.

### 2. Map Environment Variable to a Build Target

To compile the required application, match the environment variable from the
test header to a buildable target defined in `scripts/build/build/targets.py`.

Typical mappings:

-   `${ALL_CLUSTERS_APP}` -> Target: `linux-x64-all-clusters-clang` (builds
    `chip-all-clusters-app`)
-   `${ALL_DEVICES_APP}` -> Target: `linux-x64-all-devices-clang` (builds
    `all-devices-app`)

To see the full list of compile targets available in the project:

```bash
scripts/run_in_build_env.sh "./scripts/build/build_examples.py targets"
```

### 3. Build the Target

Activate the environment and use the `build_examples.py` tool to compile the
application.

```bash
source scripts/activate.sh
./scripts/build/build_examples.py --target {build_target} build
```

Example:

```bash
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-x64-all-devices-clang build
```

Once compiled, the application binary will be located in its corresponding
target folder under `out/` (e.g.
`out/linux-x64-all-devices-clang/all-devices-app`). Keep track of this local
path, as you will pass it explicitly to the execution commands.

---

## Determining Test Arguments

The header comments at the top of each python test file define the parameters
used for both the example app and the script.

> [!TIP] When running locally and manually passing arguments, ignore or remove
> environment-variable-based arguments (like `${TRACE_APP}` or
> `${TRACE_TEST_JSON}`) from your command line. These are intended for CI
> pipelines.

### Excluding Arguments During Local Testing

When running tests manually on a local workstation, you can (and sometimes
should) omit certain CI-mandated arguments depending on your specific testing
goal:

-   **Omitting `--factory-reset` and `--commissioning-method`**: If you are
    running multiple test scripts sequentially against the _same_ active device
    session or commissioning window, you should omit these flags. Factory
    resetting or re-commissioning is unnecessary if the device is already
    configured and commissioned on the controller.
-   **Reusing KVS Storage**: If you wish to verify state persistence across
    application restarts (e.g. simulating a power cycle), make sure you do _not_
    pass `--factory-reset` or delete your `--KVS` database files.

---

## Identifying Test Header Parameters

The header comments at the top of each python test file define the parameters
used for both the example app and the script. You can understand what each
parameter does using this description:

-   **`app`**: Indicates the application to be used in the test.
    -   _Example_: `app: ${ALL_DEVICES_APP}`
-   **`factory-reset`**: Determines whether a factory reset should be performed
    before the test (i.e., wiping persistent KVS files).
    -   _Example_: `factory-reset: true`
-   **`quiet`**: Sets the verbosity level of the test run. When set to `true`,
    the test runner generates less output.
    -   _Example_: `quiet: true`
-   **`app-args`**: Specifies the arguments to be passed to the application
    during the test.
    -   _Example_: `--discriminator 1234 --KVS kvs1`
    -   **`--discriminator`**: Specifies the discriminator value used by the
        application during commissioning to uniquely identify the device during
        the discovery phase. This will change the pairing code for the device
    -   **`--KVS`**: Specifies the path to the Key-Value Store (KVS) file. The
        KVS is a persistent database used by the application to store state
        (such as commissioned fabrics, node IDs, etc.) so that they survive
        application restarts. Specifying a unique KVS path prevents state
        interference between concurrent test runs.
-   **`app-ready-pattern`**: Regular expression pattern to match against the
    application's stdout to determine when the application has completed
    initialization. The test runner blocks script execution until this pattern
    is found.
    -   _Example_: `"Manual pairing code: \\[\\d+\\]"`
-   **`app-stdin-pipe`**: Path to a named pipe that the test runner can use to
    pipe standard input command strings into the application process.
    -   _Example_: `dut-input`
-   **`script-args`**: Specifies the CLI arguments to be passed to the test
    script itself.
    -   _Example_:
        `--storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021`

---

## Python Script Command Line Arguments

When running the test script directly (using Python) or passing arguments inside
`--script-args`, you can customize the execution. Use `--help` on any script to
get a full list of arguments. Key parameters include:

-   **`--storage-path`**: Configures the file path where local state and
    credentials are persisted (e.g., `--storage-path admin_storage.json`).
    Providing this argument avoids state conflicts between concurrent runs.
    Defaults to `admin_storage.json` in the current working directory.
-   **`--commissioning-method`**: Specifies the method to use for commissioning
    (e.g. `on-network`). Because `chip-tool` and the python controller do not
    share credentials, the python script must commission the simulator target.
-   **Pairing Codes**:
    -   `--discriminator` (e.g. `1234`)
    -   `--passcode` (e.g. `20202021`)
    -   `--qr-code`
    -   `--manual-code`
-   **`--tests`**: Filters which test case(s) within the script to execute.
-   **`--PICS`**: Path to the PICS XML file or directory defining supported
    features.
-   **Custom Script Arguments (PIXITs)**: Used for passing custom config values
    to the script. Must be supplied in `key:value` format:
    -   `--bool-arg` (e.g. `--bool-arg pixit_name:False`)
    -   `--int-arg`
    -   `--float-arg`
    -   `--string-arg`
    -   `--json-arg`
    -   `--hex-arg`

---

## Running Tests: Method 1 (CI Meta Runner - Recommended)

The project provides a high-level local test harness script,
`./scripts/tests/local.py`. This is the **preferred and most automated method**
for running python tests locally because it pulls test metadata and CI arguments
directly from the headers of the Python test scripts.

### Setup Requirements

This script hardcodes the virtual environment location to `out/venv`. You must
compile and activate this specific virtual environment path before running the
command:

```bash
./scripts/build_python.sh -i out/venv --enable_ipv4 true
source out/venv/bin/activate
```

### Executing Tests

Use the `python-tests` subcommand and specify a filter using `--test-filter` to
target specific test suites:

```bash
./scripts/tests/local.py python-tests --test-filter {test_name}
```

### Mapping Apps using `--override-binary-path`

Because the script reads the CI blocks directly, it parses variables like
`app: ${ALL_CLUSTERS_APP}`. By default, it attempts to look for built binaries
in default output folders. If your compiled binaries are located in a
non-standard build path, you must map the CI variable to your local compiled
file using `--override-binary-path`:

```bash
./scripts/tests/local.py python-tests \
  --test-filter {test_name} \
  --override-binary-path {CI_APP_VARIABLE} {path_to_local_built_binary}
```

### Execution Example:

```bash
./scripts/tests/local.py python-tests \
  --test-filter TC_RR_1_1 \
  --override-binary-path ALL_CLUSTERS_APP out/linux-x64-all-clusters-no-ble/chip-all-clusters-app
```

### Notable Arguments for `local.py python-tests`

-   **`--test-filter`**: Runs only the test scripts that match this glob pattern
    (e.g., `TC_RR_1_1` or `TC_FAN_*`).
-   **`--override-binary-path`**: Maps a CI environment variable (e.g.,
    `ALL_CLUSTERS_APP`, `ALL_DEVICES_APP`) to a specific path where the
    application is built. Can be passed multiple times.
-   **`--fail-log-dir`**: Specifies a directory to write logs when a test fails
    (creates `{test_name}.out.log` and `{test_name}.err.log` inside the
    directory). If not specified, failed test logs are printed directly to
    stdout/stderr.
-   **`--skip`**: Excludes specific test names or patterns from running.
-   **`--include-nightly`**: Includes slow nightly tests that are skipped by
    default.
-   **`--keep-going`**: Instructs the runner to continue executing subsequent
    tests even if a previous test in the queue fails.
-   **`--help`**: View the help dialog for more information.

---

## Running Tests: Method 2 (Combined Runner)

This uses the `run_python_test.py` script directly to manage starting the app,
running the test, and collecting logs. Unlike `local.py`, you must explicitly
provide all arguments since it does not read metadata blocks from the files.

### Combined Runner Template

With the Python environment active:

```bash
./scripts/tests/run_python_test.py \
  --factory-reset \
  --app {path_to_compiled_app_binary} \
  --app-args "{app_arguments}" \
  --script {path_to_python_script} \
  --script-args "{script_arguments}"
```

### Combined Examples

#### Example A: Contact Sensor (All-Devices) Test

```bash
./scripts/tests/run_python_test.py \
  --factory-reset \
  --app ./out/linux-x64-all-devices-clang/all-devices-app \
  --app-args "--device contact-sensor:1 --discriminator 1234 --KVS kvs1" \
  --script src/python_testing/TC_IDM_2_3.py \
  --script-args "--storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values"
```

Note that the `--endpoint` script argument is relevant for most apps and tests.
Some tests may exclude an endpoint argument and use the default one (which often
is EP 0), but it is best to explicitly specify which endpoint should be used.

#### Example B: All Clusters App Test

```bash
./scripts/tests/run_python_test.py \
  --factory-reset \
  --app ./out/linux-x64-all-clusters-clang/chip-all-clusters-app \
  --app-args "--discriminator 1234 --KVS kvs1" \
  --script src/python_testing/TC_GC_2_2.py \
  --script-args "--storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --endpoint 1"
```

---

## Running Tests: Method 3 (Two Terminals)

This method is useful for interactive debugging, allowing you to monitor the
application's stdout logs side-by-side with the test script runner.

### Step A: Start the Example Application

In one terminal, run your locally built application binary:

```bash
./out/linux-x64-all-devices-clang/all-devices-app \
  --device contact-sensor:1 \
  --discriminator 1234 \
  --KVS kvs1
```

Note that the all devices app should always specify a device type and endpoint,
in the format `device-type:endpoint-number`

### Step B: Run the Python Test Script

In a second terminal, run the python script after activating the Python virtual
environment:

```bash
source out/venv/bin/activate

python3 src/python_testing/TC_IDM_2_3.py \
  --storage-path admin_storage.json \
  --commissioning-method on-network \
  --discriminator 1234 \
  --passcode 20202021 \
  --PICS src/app/tests/suites/certification/ci-pics-values
```
