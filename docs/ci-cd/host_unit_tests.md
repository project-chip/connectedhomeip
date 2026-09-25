# Host unit tests in CI

Which configurations of the host unit-test suite (`ninja check` / `gn_tests.sh`
/ `build_examples.py --target *-tests*`) run in CI, when, and why.

**Keep this file in sync:** `scripts/tools/list_unit_test_steps.py` lists every
CI step that runs unit tests into `host_unit_tests.steps.txt`. When a workflow
change alters that list, run it with `--update` and update the tables below in
the same PR. The `code-lints` job in `lint.yml` runs it with `--check`.

Release branches (`v*-branch`) run their own copy of each workflow. On push they
behave like a master push; scheduled (nightly) workflows do not run on them.

## Linux and macOS host

| Configuration                                        | Job (workflow)                                                 | PR           | Master push | Nightly | Why                                                 |
| ---------------------------------------------------- | -------------------------------------------------------------- | ------------ | ----------- | ------- | --------------------------------------------------- |
| gcc, OpenSSL, `enable_rtti`                          | Build on Linux (python_lib) (`build.yaml`)                     | yes          | yes         | –       | Builds the Python controller; also runs the suite   |
| gcc, OpenSSL, `use_coverage`                         | Build on Linux (coverage) (`build.yaml`)                       | yes          | yes         | –       | Codecov report                                      |
| gcc, OpenSSL, memory debug checks, platform tests    | Build on Linux (gcc_debug) (`build.yaml`)                      | –            | yes         | CodeQL  | Only run of `chip_config_memory_debug_checks`       |
| gcc, OpenSSL, `chip_logging=false`, platform tests   | Build on Linux (gcc_debug) (`build.yaml`)                      | yes          | –           | –       | Logging-off build                                   |
| gcc, detail / progress / error logging off (compile) | Build on Linux (gcc_debug) (`build.yaml`)                      | –            | yes         | –       | Logging-level builds                                |
| gcc release (`is_debug=false`)                       | Build on Linux (fake, gcc_release, clang, simulated)           | –            | yes         | –       | Release-mode build                                  |
| clang ASan, OpenSSL                                  | Build on Linux (fake, gcc_release, clang, simulated)           | yes          | –           | –       | Sanitizer run on every PR                           |
| clang ASan, TSan, UBSan, OpenSSL                     | Build on Linux (fake, gcc_release, clang, simulated)           | –            | yes         | –       | Full sanitizer set after merge                      |
| fake platform (`linux-fake-tests`)                   | Build on Linux (fake, gcc_release, clang, simulated)           | yes          | yes         | –       | Platform-independent tests                          |
| clang ASan, PSA                                      | Crypto Backend Unit Tests (`unit_integration_test.yaml`)       | crypto paths | yes         | –       | Crypto backend                                      |
| clang ASan, mbedTLS                                  | Crypto Backend Unit Tests (`unit_integration_test.yaml`)       | crypto paths | yes         | –       | Crypto backend                                      |
| clang ASan, BoringSSL, ICD + ARL + rotating id       | Nightly Unit Tests - Linux (various features) (`nightly.yaml`) | –            | –           | yes     | Feature flags; ~66m, `TestRead` waits on ICD timers |
| clang MSan                                           | Nightly MSAN Tests - Linux (`nightly.yaml`)                    | –            | –           | yes     | Uninitialized-memory checks                         |
| macOS ASan, BoringSSL, `target_os="all"`             | Build on Darwin (clang, simulated) (`build.yaml`)              | yes          | yes         | –       | macOS host                                          |

"Crypto paths": `src/credentials`, `src/crypto`, `src/pw_backends`,
`third_party/pigweed`, `third_party/mbedtls`, `unit_integration_test.yaml`.
"Platform tests": `chip_build_all_platform_tests=true` (Silabs host tests).

## Embedded targets

| Configuration          | Job (workflow)                               | PR         | Master push | Why          |
| ---------------------- | -------------------------------------------- | ---------- | ----------- | ------------ |
| ESP32 QEMU, mbedTLS    | ESP32_QEMU (`qemu.yaml`)                     | yes        | yes         | ESP32 target |
| Tizen QEMU             | Tizen (`qemu.yaml`)                          | yes        | yes         | Tizen target |
| Zephyr native_sim, PSA | nRF Connect SDK (`examples-nrfconnect.yaml`) | some paths | yes         | nRF target   |

## Open questions

-   gcc runs the full suite three times on every PR (python_lib, coverage,
    gcc_debug `chip_logging=false`). Is more than one needed?
-   Release branches get no nightly coverage (#74387).
