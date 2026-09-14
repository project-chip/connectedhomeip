# AI Agent Guidelines for Matter SDK

This file provides guidelines and instructions for AI agents working on the
Matter SDK codebase.

## General Principles

- **When in Rome**: Match the prevailing style of the code being modified. See
  [docs/style/CODING_STYLE_GUIDE.md](docs/style/CODING_STYLE_GUIDE.md).
- **Atomicity**: Make small, incremental changes. Do not mix refactoring with
  feature implementation.
- **No Filler Names**: Avoid names like "support", "common", "helpers", "util",
  "core". Use concrete names.
- **Error Handling**: Use `CHIP_ERROR` as the standard return type for fallible
  operations. Prefer `VerifyOrReturnError` and `ReturnErrorOnFailure` macros for
  concise error checking and propagation.
- Ensure resources are cleaned up appropriately, especially on early returns.
  Generally prefer RAII patterns for cleanup.
- **Logging**: Use the `ChipLog*` macros (e.g., `ChipLogProgress`,
  `ChipLogError`, `ChipLogDetail`) for logging. Ensure logs are appropriately
  categorized by module (e.g., `AppServer`, `InteractionModel`).

## Ignored Directories

When searching for files or code patterns, ignore the following directories
unless explicitly asked to look there:

- `third_party/` (contains external dependencies)
- `out/` (contains build artifacts)

## Code Review Instructions

- Do not comment on content for XML files or .matter content for clusters.
- The SDK implements an in-progress Matter specification that may be in flux and
  may not be available to all contributors. Assume the Matter specification is
  unknown and out of scope _unless_ you have explicit access to the latest
  version (e.g., via a specialized tool or skill).
- Avoid "pat on the back" style comments that just restate what the code is
  doing. Focus on suggesting concrete code improvements.
- Be concise. Do not over-explain code.
- Look for common typos and suggest fixes.
- Do not comment on whitespace or formatting (auto-formatters handle this).
- Review changes for embedded development:
    - Minimize use of heap allocation.
    - Optimize for resource usage (RAM/Flash).
    - Be cautious with complex templates that could lead to code bloat.

## API Stability

Public APIs often have consumers outside this tree, so source compatibility is
something to think about when changing or extending them. Breaking compatibility
is not strictly prohibited, but should only be done for good reasons. There is
no hard line between public and internal APIs: how much compatibility matters in
a given area depends on how likely it is to be used by external clients, and on
whether the API itself is considered stable or belongs to a feature still in
development.

Where compatibility shapes a decision, record it in a comment or the commit
message; a deliberate break and a deliberate workaround both read as accidents
otherwise.

## API preferences

- Prefer using `chip::Span` from `src/lib/support/Span.h` to pointer + size
  groups. Pass `Span` by value rather than const reference (treat it as a
  `string_view`)
- Use `"foo"_span` (i.e. `operator _span`) for const char spans instead of
  `fromCharString`.
- Prefer `std::optional` to `chip::Optional`
- Prefer `StringBuilder` from `src/lib/support/StringBuilder.h` to using
  `snprintf` for string formatting.

## Coding Style (Highlights)

Refer to [docs/style/CODING_STYLE_GUIDE.md](docs/style/CODING_STYLE_GUIDE.md)
for full details.

- **C++**: C++17 standard.
    - Use fixed-width integer types from `<cstdint>` for POD integer types
    - Avoid top-level `using namespace` in headers.
    - Use anonymous namespaces for file-internal classes/objects.
    - Avoid heap allocation and auto-resizing containers in core SDK.
- **Python**: Python 3.11 standard.
    - Use type hints on public APIs.
    - Include docstrings for public APIs.
- _Always_ include `{}` bracketing for control flows, even if using one liners
  (e.g. for `if`, `while`, `for` and such)

## Testing

- Unit tests are required for all changes unless unit testing is impossible
  (e.g., platform-specific code).
- Tests in `src/python_testing` and `src/app/tests/suites` which verify expected
  failures should clearly indicate why the failure is expected. Include a
  summary of the relevant specification requirements if possible.

## Architectural Constraints

### Code-Driven Clusters

Code-driven clusters are implementations in `src/app/clusters` that use
`DefaultServerCluster` as a base class. When developing them:

- `ReadAttribute`, `WriteAttribute`, and `InvokeCommand` are by API contract
  only called for existent paths. Do not add path validity checks — they
  increase code size and are redundant as long as `Attributes` or
  `AcceptedCommands` are correct.
- Ember APIs and generated ZAP accessors must not be used outside the
  `CodegenIntegration` layer. `CodegenIntegration.h/cpp` is the documented
  bridge between generated configuration and code-driven cluster logic. Avoid
  types like `EmberAfStatus` or functions like `emberAfContainsServer`,
  `emberAfReadAttribute`, or `emberAfWriteAttribute` in core cluster code.
- When adding files: codegen-specific files belong in
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

Most commands require an activated environment. The user may or may not have
already done that before running an agent harness; if `$PW_PROJECT_ROOT` is set
the environment is probably active.

### Environment Activation

You can run commands within the environment using `scripts/run_in_build_env.sh`:
`scripts/run_in_build_env.sh "command"`

Alternatively, you can activate the environment in your shell:
`source scripts/activate.sh`

### Build and Test

- **List available targets**:
  `scripts/run_in_build_env.sh "./scripts/build/build_examples.py targets"`
- **Generate Ninja files**:
  `scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-tests-clang --quiet gen"`
- **Build and run all tests**:
  `scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-tests-clang --quiet build"`
- **Run a specific test**:
  `scripts/run_in_build_env.sh "ninja -C out/linux-x64-tests-clang --quiet path/to/test:test_name.run"`
    - Explicit example:
      `scripts/run_in_build_env.sh "ninja -C out/linux-x64-tests-clang src/app/clusters/occupancy-sensor-server/tests:TestOccupancySensingCluster.run"`
    - Compile and run can be separated (e.g. if running under some memory
      debugger or needing to set other options):

              ```bash
              scripts/run_in_build_env.sh "ninja -C out/linux-x64-tests-clang src/app/clusters/occupancy-sensor-server/tests:TestOccupancySensingCluster"`
              ./out/linux-x64-tests-clang/tests/TestOccupancySensingCluster
              ```

### Building Common Apps

- **chip-tool** (Interactive commissioning tool):
  `scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-chip-tool-clang --quiet build"`
- **all-clusters-app** (Feature-rich device simulator):
  `scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-all-clusters-clang --quiet build"`
- **all-devices-app** (Alternative feature-rich simulator):
  `scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-all-devices-clang --quiet build"`

### Joint Fabric Python Tests – Local

TC*JFDS*_ and TC*JF*_ tests start a JFA app and a JFC app on the same machine
and commission them over the network using mDNS/DNS-SD discovery. They require
three one-time setup steps before the first run.

#### Step 1 – Fix mDNS on the loopback interface (pick one option)

mDNS uses multicast UDP (`224.0.0.251`). In some environments (UTM NAT mode,
minimal Docker images) multicast does not reach the loopback interface and
commissioning times out after 3 s.

**Option A – Install avahi-daemon (recommended for UTM)**

```bash
sudo apt install avahi-daemon
sudo systemctl enable --now avahi-daemon
```

Verify that avahi reflects mDNS on loopback:

```bash
avahi-browse -a --terminate 2>/dev/null | head
```

**Option B – Switch UTM VM NIC to Bridged mode**

In UTM → VM settings → Network → Mode: change from _Shared Network_ to _Bridged
(advanced)_ and select the host Wi-Fi or Ethernet adapter. Bridged adapters pass
multicast packets through directly; no software change is required.

**Option C – Rebuild the Python venv with IPv4 enabled**

The default `build_python.sh` invocation may omit `--enable_ipv4`, which forces
IPv6-only mDNS and breaks same-host discovery when the loopback does not carry
IPv6 multicast.

```bash
scripts/build_python.sh -i out/venv --enable_ipv4 true
```

This takes ~5–10 minutes. It also fixes the venv for all other Python tests that
rely on on-network commissioning.

#### Step 2 – Build the JFA and JFC apps (arm64, one time)

```bash
scripts/run_in_build_env.sh \
  "./scripts/build/build_examples.py --target linux-arm64-jf-admin-app-clang build"

scripts/run_in_build_env.sh \
  "./scripts/build/build_examples.py --target linux-arm64-jf-control-app-clang build"
```

Binaries land at:

- `out/linux-arm64-jf-admin-app-clang/jfa-app`
- `out/linux-arm64-jf-control-app-clang/jfc-app`

#### Step 3 – Run the tests

Use the provided wrapper script, which handles the venv activation, binary-path
overrides, and a one-shot sync of the `matter.testing` package from source
(needed when the venv is older than the source tree):

```bash
# Run a single test
scripts/tests/run_jf_tests_local.sh --test-filter TC_JFDS_2_3

# Run all JF datastore tests
scripts/tests/run_jf_tests_local.sh --test-filter "TC_JFDS_*"
```

#### Troubleshooting

| Symptom                                                     | Likely cause                               | Fix                                                              |
| ----------------------------------------------------------- | ------------------------------------------ | ---------------------------------------------------------------- |
| `Timeout` after 3 s in JFC pairing                          | mDNS multicast not working                 | Apply Step 1 option A, B, or C                                   |
| `ImportError: cannot import name 'matter_test_args_parser'` | Stale venv                                 | Run the wrapper script – it syncs `matter.testing` automatically |
| `The path … does not exist` in `setup_class`                | Wrong binary paths                         | Check Step 2 build targets completed successfully                |
| `AttributeError: fabric_a_admin` in `teardown_class`        | `setup_class` failed before attribute init | Fixed in source; update venv sync                                |

## Development Resources

- [docs/guides/writing_clusters.md](docs/guides/writing_clusters.md)
- [docs/guides/migrating_ember_cluster_to_code_driven.md](docs/guides/migrating_ember_cluster_to_code_driven.md)
- [docs/testing/unit_testing.md](docs/testing/unit_testing.md)
- [docs/testing/integration_tests.md](docs/testing/integration_tests.md)
