# AI Agent Guidelines for Matter SDK

This file provides guidelines and instructions for AI agents working on the
Matter SDK codebase.

## General Principles

-   **When in Rome**: Match the prevailing style of the code being modified. See
    [docs/style/CODING_STYLE_GUIDE.md](docs/style/CODING_STYLE_GUIDE.md).
-   **Atomicity**: Make small, incremental changes. Do not mix refactoring with
    feature implementation.
-   **No Filler Names**: Avoid names like "support", "common", "helpers",
    "util", "core". Use concrete names.
-   **Error Handling**: Use `CHIP_ERROR` as the standard return type for
    fallible operations. Prefer `VerifyOrReturnError` and `ReturnErrorOnFailure`
    macros for concise error checking and propagation.
-   Ensure resources are cleaned up appropriately, especially on early returns.
    Generally prefer RAII patterns for cleanup.
-   **Logging**: Use the `ChipLog*` macros (e.g., `ChipLogProgress`,
    `ChipLogError`, `ChipLogDetail`) for logging. Ensure logs are appropriately
    categorized by module (e.g., `AppServer`, `InteractionModel`).

## Ignored Directories

When searching for files or code patterns, ignore the following directories
unless explicitly asked to look there:

-   `third_party/` (contains external dependencies)
-   `out/` (contains build artifacts)

## Code Review Instructions

-   Do not comment on content for XML files or .matter content for clusters.
-   The SDK implements an in-progress Matter specification that may be in flux
    and may not be available to all contributors. Assume the Matter
    specification is unknown and out of scope _unless_ you have explicit access
    to the latest version (e.g., via a specialized tool or skill).
-   Avoid "pat on the back" style comments that just restate what the code is
    doing. Focus on suggesting concrete code improvements.
-   Be concise. Do not over-explain code.
-   Look for common typos and suggest fixes.
-   Do not comment on whitespace or formatting (auto-formatters handle this).
-   Review changes for embedded development:
    -   Minimize use of heap allocation.
    -   Optimize for resource usage (RAM/Flash).
    -   Be cautious with complex templates that could lead to code bloat.

## API preferences

-   Prefer using `chip::Span` from `src/lib/support/Span.h` to pointer + size
    groups. Pass `Span` by value rather than const reference (treat it as a
    `string_view`)
-   Use `"foo"_span` (i.e. `operator _span`) for const char spans instead of
    `fromCharString`.
-   Prefer `std::optional` to `chip::Optional`
-   Prefer `StringBuilder` from `src/lib/support/StringBuilder.h` to using
    `snprintf` for string formatting.

## Coding Style (Highlights)

Refer to [docs/style/CODING_STYLE_GUIDE.md](docs/style/CODING_STYLE_GUIDE.md)
for full details.

-   **C++**: C++17 standard.
    -   Use fixed-width integer types from `<cstdint>` for POD integer types
    -   Avoid top-level `using namespace` in headers.
    -   Use anonymous namespaces for file-internal classes/objects.
    -   Avoid heap allocation and auto-resizing containers in core SDK.
-   **Python**: Python 3.11 standard.
    -   Use type hints on public APIs.
    -   Include docstrings for public APIs.
-   _Always_ include `{}` bracketing for control flows, even if using one liners
    (e.g. for `if`, `while`, `for` and such)

## Testing

-   Unit tests are required for all changes unless unit testing is impossible
    (e.g., platform-specific code).
-   Tests in `src/python_testing` and `src/app/tests/suites` which verify
    expected failures should clearly indicate why the failure is expected.
    Include a summary of the relevant specification requirements if possible.

## Architectural Constraints

### Code-Driven Clusters

Code-driven clusters are implementations in `src/app/clusters` that use
`DefaultServerCluster` as a base class. When developing them:

-   `ReadAttribute`, `WriteAttribute`, and `InvokeCommand` are by API contract
    only called for existent paths. Do not add path validity checks — they
    increase code size and are redundant as long as `Attributes` or
    `AcceptedCommands` are correct.
-   Ember APIs and generated ZAP accessors must not be used outside the
    `CodegenIntegration` layer. `CodegenIntegration.h/cpp` is the documented
    bridge between generated configuration and code-driven cluster logic. Avoid
    types like `EmberAfStatus` or functions like `emberAfContainsServer`,
    `emberAfReadAttribute`, or `emberAfWriteAttribute` in core cluster code.
-   When adding files: codegen-specific files belong in
    `app_config_dependent_sources.cmake/gni`; all others belong in `BUILD.gn`.
    Ensure every file (especially headers) is listed in one of these — there
    should be no unreferenced files.

### Example Applications (Documentation Discovery)

When operating on or analyzing reference applications (such as
`examples/all-devices-app` or custom simulator tools), always inspect that
application's dedicated `docs/` folder or `ARCHITECTURE.md` file to understand
its dynamic runtime Interaction Model, specific CLI parameters, and recommended
product baseline patterns before modifying or generating code.

## Common Commands

Most commands require an activated environment.

### Environment Activation

You can run commands within the environment using `scripts/run_in_build_env.sh`:
`scripts/run_in_build_env.sh "command"`

Alternatively, you can activate the environment in your shell:
`source scripts/activate.sh`

### Build and Test

-   **List available targets**:
    `scripts/run_in_build_env.sh "./scripts/build/build_examples.py targets"`
-   **Generate Ninja files**:
    `scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-tests-clang --quiet gen"`
-   **Build and run all tests**:
    `scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-tests-clang --quiet build"`
-   **Run a specific test**:
    `scripts/run_in_build_env.sh "ninja -C out/linux-x64-tests-clang --quiet path/to/test:test_name.run"`

    -   Explicit example:
        `scripts/run_in_build_env.sh "ninja -C out/linux-x64-tests-clang src/app/clusters/occupancy-sensor-server/tests:TestOccupancySensingCluster.run"`
    -   Compile and run can be separated (e.g. if running under some memory
        debugger or needing to set other options):

              ```bash
              scripts/run_in_build_env.sh "ninja -C out/linux-x64-tests-clang src/app/clusters/occupancy-sensor-server/tests:TestOccupancySensingCluster"`
              ./out/linux-x64-tests-clang/tests/TestOccupancySensingCluster
              ```

### Building Common Apps

-   **chip-tool** (Interactive commissioning tool):
    `scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-chip-tool-clang --quiet build"`
-   **all-clusters-app** (Feature-rich device simulator):
    `scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-all-clusters-clang --quiet build"`
-   **all-devices-app** (Alternative feature-rich simulator):
    `scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-all-devices-clang --quiet build"`

### Building Joint Fabric Apps

-   **jf-admin-app** (Joint Fabric Administrator):
    `scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-jf-admin-app-clang --quiet build"`
-   **jf-control-app** (Joint Fabric Controller):
    `scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-jf-control-app-clang --quiet build"`

### Running Python Integration Tests

Python tests in `src/python_testing/` require a separate venv and built example
apps. Build the venv (re-run whenever Python bindings change):

```bash
./scripts/build_python.sh -i out/venv --enable_ipv4 true
source out/venv/bin/activate
```

Run tests using the `local.py` harness, mapping CI binary variables to local
build outputs with `--override-binary-path`:

```bash
source out/venv/bin/activate
./scripts/tests/local.py python-tests \
  --test-filter "TC_JFADMIN*" \
  --override-binary-path JF_ADMIN_APP     out/linux-x64-jf-admin-app-clang/jfa-app \
  --override-binary-path JF_CONTROL_APP   out/linux-x64-jf-control-app-clang/jfc-app \
  --override-binary-path ALL_CLUSTERS_APP out/linux-x64-all-clusters-clang/chip-all-clusters-app
```

## Pre-commit Test Policy

Before committing or pushing, run the tests that match the scope of changes.
Rebuild any affected example apps before running Python tests against them.

| Changed area                                            | Tests to run                                                                                  |
| ------------------------------------------------------- | --------------------------------------------------------------------------------------------- |
| `src/app/clusters/joint-fabric-administrator-server/**` | C++ unit tests: `joint-fabric-administrator-server/tests:tests` **and** Python: `TC_JFADMIN*` |
| `examples/jf-admin-app/**`                              | Rebuild `linux-x64-jf-admin-app-clang`; Python: `TC_JFADMIN*`                                 |
| `examples/jf-control-app/**`                            | Rebuild `linux-x64-jf-control-app-clang`; Python: `TC_JFADMIN*`, `TC_JFDS*`                   |
| `examples/common/pigweed/rpc_services/JointFabric.h`    | Rebuild jf-admin-app and jf-control-app; Python: `TC_JFADMIN*`                                |
| `src/app/server/JointFabricAdministrator.h`             | C++ unit tests: `joint-fabric-administrator-server/tests:tests` **and** Python: `TC_JFADMIN*` |
| `src/credentials/**`                                    | `ninja -C out/linux-x64-tests-clang src/credentials/tests:tests`                              |
| `src/app/clusters/<cluster-name>/**`                    | C++ unit tests for that cluster; Python `TC_<CLUSTER>*` if they exist                         |
| General SDK changes                                     | `./scripts/build/build_examples.py --target linux-x64-tests-clang --quiet build`              |

### Minimum checklist for Joint Fabric changes

```bash
# 1. Rebuild affected apps
scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-jf-admin-app-clang --quiet build"
scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-jf-control-app-clang --quiet build"

# 2. C++ unit tests
scripts/run_in_build_env.sh "ninja -C out/linux-x64-tests-clang \
  src/app/clusters/joint-fabric-administrator-server/tests:TestAddICACInstallation.run \
  src/app/clusters/joint-fabric-administrator-server/tests:TestJCMCommissionee.run"

# 3. Python integration tests
source out/venv/bin/activate
./scripts/tests/local.py python-tests \
  --test-filter "TC_JFADMIN*" \
  --override-binary-path JF_ADMIN_APP     out/linux-x64-jf-admin-app-clang/jfa-app \
  --override-binary-path JF_CONTROL_APP   out/linux-x64-jf-control-app-clang/jfc-app \
  --override-binary-path ALL_CLUSTERS_APP out/linux-x64-all-clusters-clang/chip-all-clusters-app
```

See also `/home/robert/matter/AGENTS.md` for workspace-level setup instructions.

## Development Resources

-   [docs/guides/writing_clusters.md](docs/guides/writing_clusters.md)
-   [docs/guides/migrating_ember_cluster_to_code_driven.md](docs/guides/migrating_ember_cluster_to_code_driven.md)
-   [docs/testing/unit_testing.md](docs/testing/unit_testing.md)
-   [docs/testing/integration_tests.md](docs/testing/integration_tests.md)
