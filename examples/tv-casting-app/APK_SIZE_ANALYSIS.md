# TV Casting App — APK Size Analysis

This document analyzes the native library and APK size impact of the
`optimize_apk_size` and `use_static_libcxx` build flags for the Matter TV
Casting App on Android (arm64-v8a).

---

## 1. Optimized vs. Non-Optimized Comparison (arm64-v8a)

| Metric                   | Default build               | Optimized build (phase 3)  | Reduction |
| ------------------------ | --------------------------- | -------------------------- | --------- |
|                          | `optimize_apk_size=false`   | `optimize_apk_size=true`   |           |
|                          | `use_static_libcxx=false`   | `use_static_libcxx=true`   |           |
| **`libTvCastingApp.so`** | 18 MB                       | 2.6 MB                     | 86 %      |
| **`libc++_shared.so`**   | 8.9 MB (shipped separately) | 1.3 MB (statically linked) | 85 %      |
| **Total native libs**    | 27.9 MB                     | 2.6 MB (single `.so`)      | 91 %      |

### What drives the reduction

| Optimization                           | Mechanism                                                                                                | Impact                                                                                                    |
| -------------------------------------- | -------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------- |
| Slim cluster-objects override          | Compile ~29 casting clusters instead of ~200+ via `chip_data_model_overrides_dir`                        | Major `.so` reduction                                                                                     |
| Slim TLV decoder overrides             | Compile TLV decoders for 18 casting clusters only via `chip_data_model_overrides_dir`                    | Removes link-time deps on ~200+ clusters while keeping `ChipClusters.java` read/subscribe APIs functional |
| Remove legacy chip-tool sources        | Exclude ~20 source files + tracing/JSON deps                                                             | Further `.so` reduction                                                                                   |
| Static libc++ with dead-code stripping | `−static-libstdc++` + `--gc-sections` + LTO                                                              | Eliminates separate 8.9 MB `libc++_shared.so`; only referenced symbols survive                            |
| Compiler / linker flags                | `-Os`, `-g0`, `-flto=thin`, `--icf=safe`, `-fvisibility=hidden`                                          | ~10–20 % further `.so` reduction                                                                          |
| R8 Java shrinking                      | `minifyEnabled true` in Gradle debug build                                                               | Reduces DEX size (Java / Kotlin)                                                                          |
| Cluster server exclusion (phase 3)     | Compile only 13 cluster server dirs via `chip_data_model_overrides_dir` + `cluster-servers-override.gni` | ~1.1 MB `.o` savings                                                                                      |
| Slim Accessors.cpp (phase 3)           | Compile accessors for 29 clusters only via `chip_data_model_overrides_dir` + `Accessors-override.cpp`    | ~826 KB `.o` savings                                                                                      |
| jsoncpp/jsontlv removal (phase 3)      | Not feasible — `AndroidCallbacks.cpp` and `AndroidInteractionClient.cpp` use jsontlv at runtime          | ~350 KB (not achievable)                                                                                  |

---

## 2. Optimized Build Breakdown

When built with `optimize_apk_size=true` and `use_static_libcxx=true`, the
resulting `libTvCastingApp.so` contains only the components required by the
casting use case.

### Matter core infrastructure

-   Transport layer (TCP/UDP, MRP reliable messaging)
-   Cryptographic primitives (HKDF, AES-CCM, ECDSA via mbedTLS / BoringSSL)
-   Secure channel (CASE, PASE session establishment)
-   Messaging layer (exchange management, protocol dispatch)
-   Interaction model engine (read / write / invoke / subscribe)

### Casting-specific clusters (29 clusters, ~116 `.ipp` includes)

**Infrastructure clusters (12):** AccessControl, AdministratorCommissioning,
BasicInformation, Binding, Descriptor, GeneralCommissioning, GeneralDiagnostics,
GroupKeyManagement, Groups, Identify, NetworkCommissioning,
OperationalCredentials

**Casting clusters (17):** AccountLogin, ApplicationBasic, ApplicationLauncher,
AudioOutput, Channel, ContentAppObserver, ContentControl, ContentLauncher,
KeypadInput, LevelControl, LowPower, MediaInput, MediaPlayback, Messages, OnOff,
TargetNavigator, WakeOnLan

**Shared utilities:** `shared/Structs.ipp` (always required)

### Simplified casting API

-   Core layer (~12 files): `CastingApp`, `CastingPlayer`,
    `CastingPlayerDiscovery`, `Endpoint`, `CommissionerDeclarationHandler`,
    `Types`
-   Support layer (~12 files): `CastingStore`, `ChipDeviceEventHandler`,
    `EndpointListLoader`, `AppParameters`, `DataProvider`
-   Minimal optimized sources: `ContentAppObserver`, `ZCLCallbacks`

### JNI wrappers (Android-specific)

-   `core/` JNI: `CastingApp-JNI`, `CastingPlayerDiscovery-JNI`,
    `MatterCastingPlayer-JNI`, `MatterEndpoint-JNI`
-   `support/` JNI: `Converters-JNI`, `JNIDACProvider`, `MatterCallback-JNI`,
    `RotatingDeviceIdUniqueIdProvider-JNI`

### Java controller interaction model

-   `android_chip_im_jni` — interaction model JNI bindings
-   Slim TLV decoders (`CHIPAttributeTLVValueDecoder-override.cpp`,
    `CHIPEventTLVValueDecoder-override.cpp`) — hand-maintained files covering
    only the 18 casting clusters, selected at build time via
    `chip_data_model_overrides_dir`. These keep `ChipClusters.java`
    read/subscribe APIs functional at runtime while avoiding link-time
    dependencies on all ~200+ cluster implementations.

### Android platform layer

-   Android platform abstraction (BLE manager, preferences KVS, DNS-SD via NSD)
-   Logging, connectivity manager, system layer

### Data model and server infrastructure

-   App server, data model provider (`data-model:heap`)
-   Device attestation, credential management
-   ICD client handler/manager

### Static libc++ (dead-code-stripped)

-   Only the libc++ symbols actually referenced by the above components survive
    `--gc-sections` and thin LTO
-   Estimated overhead: ~0.5–1 MB (vs. 7.5 MB for the full `libc++_shared.so`)

### What is excluded (compared to default build)

-   ~164 non-casting cluster implementations (DoorLock, Thermostat,
    PumpConfigurationAndControl, etc.)
-   ~31 non-casting cluster server implementations (phase 3 — only 13 of ~44
    cluster server directories are compiled)
-   Full zap-generated TLV decoder files for all ~200+ clusters (replaced by
    slim 18-cluster versions)
-   Full zap-generated Accessors.cpp for all ~200+ clusters (phase 3 — replaced
    by slim 29-cluster `Accessors-override.cpp`)
-   Legacy chip-tool command sources (`Command.cpp`, `Commands.cpp`,
    `CHIPCommand.cpp`, `RemoteDataModelLogger.cpp`, etc.)
-   Tracing dependencies (`commandline`, `json`, `jsoncpp`)
-   Interaction model test suites
-   `ExamplePersistentStorage`, `ComplexArgumentParser`, `DataModelLogger`
-   Debug symbols (`-g0`)

---

## 3. SDK Footprint Projection

The example APK bundles a complete Android application shell around the casting
SDK. When a production app integrates the casting SDK as a library, significant
overhead is removed.

### APK overhead removed (example-app-only)

| Component                                                  | Estimated size | Notes                                                                    |
| ---------------------------------------------------------- | -------------- | ------------------------------------------------------------------------ |
| Example app UI (Activities, Fragments, layouts, drawables) | ~1–2 MB        | `MainActivity`, `ConnectionExampleFragment`, media player UI, etc.       |
| AndroidX + Material Design libraries                       | ~3–5 MB        | `appcompat`, `material`, `constraintlayout`, `legacy-support-v4`         |
| ZXing QR code library                                      | ~0.5–1 MB      | `com.google.zxing:core:3.3.0` — used only for QR code display in example |
| Example DAC provider                                       | < 0.1 MB       | `JNIDACProvider` with hardcoded test credentials                         |
| Gradle wrapper and build tooling                           | N/A            | Not in APK, but adds to build complexity                                 |
| Test dependencies                                          | N/A            | JUnit, Mockito, Robolectric — not in release APK                         |

### Native overhead removed

| Component                         | Notes                                                                                                                                                                                                |
| --------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Example-app-specific JNI wrappers | The `core/` and `support/` JNI `.cpp` files in `App/app/src/main/jni/cpp/` wrap the simplified API for the example UI; a production app would provide its own JNI layer or use the Java API directly |
| Compat-layer Java sources         | `com.chip.casting.*` compat classes (`TvCastingApp.java`, `AppParameters.java`, etc.) — deprecated API surface                                                                                       |

### What remains (SDK core)

| Component                                         | Estimated size   | Notes                                                                  |
| ------------------------------------------------- | ---------------- | ---------------------------------------------------------------------- |
| `libTvCastingApp.so` (arm64-v8a, optimized)       | 2.6 MB           | Measured post-phase 3 — primary size contributor                       |
| Java casting API classes (`com.matter.casting.*`) | ~0.1 MB          | `TvCastingApp.jar` is 91 KB                                            |
| `CHIPInteractionModel.jar`                        | ~7.3 MB (pre-R8) | Shrinks significantly with R8 — only casting-relevant classes retained |
| `AndroidPlatform.jar`                             | ~54 KB           | Platform abstraction                                                   |
| `CHIPAppServer.jar`                               | ~3.2 KB          | App server bootstrap                                                   |

### Estimated SDK footprint

| Scenario                                               | Estimated size |
| ------------------------------------------------------ | -------------- |
| Native library only (`libTvCastingApp.so`, arm64-v8a)  | 2.6 MB         |
| Native + Java classes (before R8)                      | ~10 MB         |
| Native + Java classes (after R8 shrinking by host app) | ~4–5 MB        |
| Compressed in APK (ZIP deflate, single ABI)            | ~2–3 MB        |

The `CHIPInteractionModel.jar` (7.3 MB) is the largest Java component, but it
contains classes for all ~200+ clusters. When the host app applies R8 shrinking,
only the classes actually referenced by the casting API survive, reducing its
contribution to well under 1 MB.

For a production app shipping a single ABI (arm64-v8a), the casting SDK adds an
estimated **2–3 MB compressed** to the APK — down from **27.9 MB** of native
libraries in the default non-optimized build.

---

## 4. Build Instructions

### Prerequisites

All builds require the Matter SDK bootstrap environment:

```bash
source scripts/bootstrap.sh   # first time (installs tooling)
source scripts/activate.sh    # subsequent shells
```

Android builds additionally require `ANDROID_HOME` and `ANDROID_NDK_HOME`
environment variables pointing to a working Android SDK + NDK installation.

### Android — Default (non-optimized) build

```bash
./scripts/build/build_examples.py \
    --target android-arm64-tv-casting-app \
    build
```

This produces the full-size APK with `libc++_shared.so` shipped separately, all
~200+ cluster implementations, legacy chip-tool sources, and debug symbols.
Output lands in `out/android-arm64-tv-casting-app/`.

### Android — Size-optimized build

```bash
./scripts/build/build_examples.py \
    --target android-arm64-tv-casting-app-size-optimized \
    build
```

The `size-optimized` modifier sets the following GN args automatically:

-   `optimize_apk_size=true` — activates slim cluster and TLV decoder overrides,
    excludes legacy sources, enables `-Os`/`-g0`/LTO/ICF
-   `is_debug=false` — removes assert checks and extra logging
-   `matter_enable_tracing_support=false` — excludes tracing deps
-   `use_static_libcxx=true` — statically links libc++ with dead-code stripping

For the TV Casting App specifically, the build also sets:

-   `chip_data_model_overrides_dir` — points to the `tv-casting-common/`
    directory containing all slim override files (cluster-objects, TLV decoders,
    cluster servers, and accessors). The build system looks for well-known
    filenames inside this directory and uses them as source overrides.

Output lands in `out/android-arm64-tv-casting-app-size-optimized/`.

To build for other ABIs, replace `arm64` with `arm`, `x64`, or `x86`.

### Comparing sizes

After both builds complete:

```bash
#Native library sizes
ls -lh out/android-arm64-tv-casting-app/lib/jni/arm64-v8a/*.so
ls -lh out/android-arm64-tv-casting-app-size-optimized/lib/jni/arm64-v8a/*.so
```

Expected results (arm64-v8a, March 2026):

| File                 | Default     | Optimized                       |
| -------------------- | ----------- | ------------------------------- |
| `libTvCastingApp.so` | 19 MB       | 2.6 MB (measured, post-phase 3) |
| `libc++_shared.so`   | 8.9 MB      | 1.3 MB (static, folded in)      |
| **Total**            | **27.9 MB** | **2.6 MB**                      |

### Desktop — Build with slim cluster set (for cluster vetting)

The tv-casting-app desktop build already uses the slim
`cluster-objects-override.cpp` (set via `chip_data_model_overrides_dir` in the
platform `args.gni`). This makes it a fast way to verify that the reduced
cluster set compiles and links correctly without needing the Android SDK.

On macOS (Apple Silicon):

```bash
./scripts/build/build_examples.py \
    --target darwin-arm64-tv-casting-app-chip-casting-simplified \
    build
```

Output lands in `out/darwin-arm64-tv-casting-app-chip-casting-simplified/`. On
Linux x64 hosts, replace the target with
`linux-x64-tv-casting-app-chip-casting-simplified`.

The `-chip-casting-simplified` modifier selects the simplified casting API entry
point (`simple-app.cpp`) which does not pull in the legacy chip-tool command
infrastructure. Without this modifier, the build uses the old `main.cpp` which
registers all cluster commands (including UnitTesting) and will fail to link
against the slim cluster file.

The resulting `chip-tv-casting-app` binary uses the same
`cluster-objects-override.cpp` that the optimized Android build now uses, so a
successful build confirms the slim cluster file is self-consistent and no
required symbols are missing.

To run it:

```bash
out/darwin-arm64-tv-casting-app-chip-casting-simplified/chip-tv-casting-app
```

If you modify `cluster-objects-override.cpp` (e.g. adding or removing cluster
`.ipp` includes), rebuild the desktop target first — it compiles in seconds and
will surface any missing-symbol errors immediately, before waiting for a full
Android build.

---

## 5. Property Tests

The `examples/tv-casting-app/tests/` directory contains property-based tests
(Python, using `hypothesis` + `pytest`) that guard the slim decoder files and
build plumbing against regressions. They parse the source files directly — no
Android NDK or JNI runtime required.

### Running the tests

```bash
python -m pytest examples/tv-casting-app/tests/ -v
```

### What each test file validates

| Test file                                 | What it catches                                                                                                                                                                                                                         |
| ----------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `test_cluster_set_membership.py`          | The slim attribute and event decoders contain case statements for exactly the 18 casting clusters — no more, no less. Catches accidental cluster additions or removals.                                                                 |
| `test_non_casting_error_handling.py`      | Both decoders have a `default:` case that sets the correct `CHIP_ERROR_IM_MALFORMED_*_PATH_IB` error and returns `nullptr`. Catches missing error paths for non-casting cluster IDs.                                                    |
| `test_interface_compatibility.py`         | The `#include` sets and function signatures (`DecodeAttributeValue`, `DecodeEventValue`) in the slim decoders match the full zap-generated decoders. Catches link-time incompatibilities with the JNI bridge.                           |
| `test_decode_logic_equivalence.py`        | For each of the 18 casting clusters, the switch-case body in the slim decoder is identical (modulo whitespace) to the full zap-generated decoder. Catches copy-paste drift when upstream zap templates change.                          |
| `test_gn_conditional_source_selection.py` | `src/controller/java/BUILD.gn` contains `if (chip_data_model_overrides_dir != "")` conditionals for TLV decoder overrides, with correct fallback to the zap-generated files. Catches broken GN plumbing.                                |
| `test_android_py_casting_args.py`         | `scripts/build/builders/android.py` sets `chip_data_model_overrides_dir` for `TV_CASTING_APP` when `optimize_size` is true, and preserves the existing cluster-objects override. Catches build-script regressions.                      |
| `test_preservation_non_casting_builds.py` | `common_flags.gni` declares `chip_data_model_overrides_dir` with empty-string default. The `args.gni` default (non-optimized) block does not set any overrides. Catches regressions that would break non-casting or non-Android builds. |

### When to run them

-   After modifying the slim decoders (`casting-CHIP*TLVValueDecoder.cpp`)
-   After changing the 18-cluster set in any file
-   After editing `BUILD.gn`, `android.py`, `config.gni`, or `args.gni` build
    plumbing
-   Before pushing a commit that touches any of the above

---

_Updated March 2026 with measured build sizes after slim TLV decoder integration
and phase 3 optimizations._ _Validates: Requirements 2.1, 2.2, 2.3, 2.4_

---

## 6. Phase 2 Optimizations (~2.8 MB → ~2.6 MB)

Phase 1 reduced `libTvCastingApp.so` from 19 MB to 2.8 MB. Phase 2 targets an
additional ~200 KB reduction by trimming non-essential infrastructure from the
already-slim binary. All changes are gated behind `optimize_apk_size=true`.

### Phase 2 optimization summary

| Optimization                        | Mechanism                                                                                                                                                                                                                                                                                                                                                                                                                                                      | Estimated savings        |
| ----------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------ |
| Cluster trimming (10 clusters)      | Remove `.ipp` includes for 10 non-essential infrastructure clusters from `cluster-objects-override.cpp` and disable their server-side definitions in `tv-casting-app.zap`                                                                                                                                                                                                                                                                                      | ~400–500 KB              |
| Disable extra data-model logging    | Set `chip_data_model_extra_logging=false` — excludes verbose log-formatting code and string tables for attribute/command path names                                                                                                                                                                                                                                                                                                                            | ~100–150 KB              |
| Host unit-test hooks (kept enabled) | `CONFIG_BUILD_FOR_HOST_UNIT_TEST=1` in `CHIPProjectAppConfig.h`, matching chip-tool. This was previously toggled to `0` in size-optimized builds via a per-target GN define, but that reached only some TUs and caused an ODR layout mismatch (CASESession / Server / FabricTable) that crashed at startup. It must be a single value in the shared project-config header. The non-size-optimized build also needs `1` for chip-tool's `InteractionModel.cpp`. | 0 KB (no longer toggled) |
| Disable RTTI                        | Set `enable_rtti=false` — removes `type_info` structures and vtable metadata generated by the C++ compiler                                                                                                                                                                                                                                                                                                                                                     | ~100–200 KB              |
| Remove ICD client dependencies      | Exclude `src/app/icd/client:handler` and `src/app/icd/client:manager` from the tv-casting-common build target                                                                                                                                                                                                                                                                                                                                                  | ~50–100 KB               |

### Clusters removed in phase 2

The following 10 infrastructure clusters were removed from the slim cluster file
and disabled in the ZAP server-side configuration. A casting client does not
serve these clusters — they are diagnostic, localization, and management
clusters irrelevant to the casting use case:

-   EthernetNetworkDiagnostics
-   SoftwareDiagnostics
-   WiFiNetworkDiagnostics
-   TimeSynchronization
-   TimeFormatLocalization
-   UnitLocalization
-   FixedLabel
-   UserLabel
-   IcdManagement
-   LocalizationConfiguration

The 12 commissioning-essential clusters (AccessControl,
AdministratorCommissioning, BasicInformation, Binding, Descriptor,
GeneralCommissioning, GeneralDiagnostics, GroupKeyManagement, Groups, Identify,
NetworkCommissioning, OperationalCredentials) and all 17 casting-specific
clusters are retained.

### Updated cluster counts (post-phase 2)

| Category                                | Count           | Change                |
| --------------------------------------- | --------------- | --------------------- |
| Infrastructure clusters (commissioning) | 12              | Unchanged             |
| Casting clusters                        | 17              | Unchanged             |
| Non-essential infrastructure (removed)  | 10              | Removed in phase 2    |
| **Total in slim cluster file**          | **29 + shared** | Down from 39 + shared |
| `.ipp` includes                         | ~116            | Down from ~157        |

### Updated size comparison (arm64-v8a)

| Metric                | Default build | Phase 1 optimized | Phase 2 optimized | Phase 2 reduction |
| --------------------- | ------------- | ----------------- | ----------------- | ----------------- |
| `libTvCastingApp.so`  | 18 MB         | 2.8 MB            | 2.6 MB            | 7 % vs phase 1    |
| `libc++_shared.so`    | 8.9 MB        | 1.3 MB (static)   | 1.3 MB (static)   | —                 |
| **Total native libs** | **26.9 MB**   | **2.8 MB**        | **2.6 MB**        | **~7 %**          |

### Phase 2 GN arguments

These arguments are passed automatically by `android.py` when building the
`android-arm64-tv-casting-app-size-optimized` target:

| Argument                          | Value    | Set by                                                                              |
| --------------------------------- | -------- | ----------------------------------------------------------------------------------- |
| `chip_data_model_extra_logging`   | `false`  | `android.py` (TV_CASTING_APP only)                                                  |
| `enable_rtti`                     | `false`  | `android.py` (TV_CASTING_APP only)                                                  |
| `CONFIG_BUILD_FOR_HOST_UNIT_TEST` | `1`      | `CHIPProjectAppConfig.h` (global project-config include; never a per-target define) |
| ICD client deps                   | excluded | `BUILD.gn` conditional (when `optimize_apk_size=true`)                              |

### Phase 2 property tests

New test files in `examples/tv-casting-app/tests/`:

| Test file                        | What it validates                                                                 |
| -------------------------------- | --------------------------------------------------------------------------------- |
| `test_phase2_cluster_removal.py` | Removed clusters absent, retained clusters present, command metadata consistency  |
| `test_phase2_zap_clusters.py`    | ZAP server-side disabled for removed clusters, enabled for commissioning clusters |
| `test_phase2_android_py_args.py` | android.py passes phase 2 GN args for TV_CASTING_APP only                         |
| `test_phase2_build_config.py`    | CHIPProjectAppConfig.h conditional, BUILD.gn ICD dep conditional                  |
| `test_phase2_rtti_icd_safety.py` | No RTTI or ICD client usage in casting common C++ sources                         |

---

_Updated March 2026 with phase 2 optimizations: cluster trimming, extra logging,
unit-test hooks, RTTI, and ICD removal._

---

## 7. Phase 3 Optimizations (~2.6 MB → ~2.3 MB estimated)

Phase 2 reduced `libTvCastingApp.so` from 2.8 MB to 2.6 MB. Phase 3 targets an
additional ~2.3 MB of `.o` file bloat identified through binary analysis of the
size-optimized arm64 build. Four optimization areas were investigated; three are
implemented and one is documented as future work.

All changes are gated behind `optimize_apk_size=true`. Default builds and
non-casting-app targets remain unaffected.

### Phase 3 optimization summary

| Optimization              | Mechanism                                                        | `.o` file savings | Status       |
| ------------------------- | ---------------------------------------------------------------- | ----------------- | ------------ |
| Cluster server exclusion  | `chip_data_model_overrides_dir` + `cluster-servers-override.gni` | ~1.1 MB           | Implemented  |
| Slim Accessors.cpp        | `chip_data_model_overrides_dir` + `Accessors-override.cpp`       | ~826 KB           | Implemented  |
| jsoncpp/jsontlv removal   | Conditional dep in `src/controller/java/BUILD.gn`                | ~350 KB           | Not feasible |
| Controller code reduction | Requires controller library refactoring                          | ~1.3 MB           | Future work  |
| **Total implemented**     |                                                                  | **~1.9 MB (.o)**  |              |

The `.o` file savings represent uncompressed object file sizes before linking.
After LTO (thin link-time optimization), ICF (identical code folding), and
`--gc-sections` dead-code stripping, the actual `.so` reduction is smaller.
Measured result: `libTvCastingApp.so` remains at 2.6 MB (same as phase 2). The
linker was already stripping the unused cluster server code and accessor
functions in earlier phases via `--gc-sections` and LTO. The phase 3 changes
still provide value by reducing compile time, intermediate object file sizes,
and making the build explicit about what's needed — preventing regressions if
LTO/gc-sections behavior changes upstream.

### 7.1 Cluster server exclusion (~1.1 MB .o savings)

**Problem:** The `chip_data_model` GN template calls `zap_cluster_list.py` to
discover ALL server-side cluster implementations from the `.matter` file. The
casting app's `.matter` file declares ~44 clusters, but only ~13 need server
implementations (12 commissioning + ContentAppObserver). The remaining ~31
cluster server directories are compiled unnecessarily.

**Mechanism:** The `chip_data_model_overrides_dir` GN argument in
`src/app/common_flags.gni` points to a directory containing well-known override
files. When set to a non-empty path, the `chip_data_model` template imports
`cluster-servers-override.gni` from the override directory and uses its cluster
list instead of calling `zap_cluster_list.py`. Both `public_deps` (cluster
server sources) and `_app_config_dependent_sources` are scoped to the override
clusters only.

**Override file:**
`examples/tv-casting-app/tv-casting-common/cluster-servers-override.gni` defines
exactly 13 cluster server directories:

-   `access-control-server`, `administrator-commissioning-server`,
    `basic-information`, `bindings`, `descriptor`,
    `general-commissioning-server`, `general-diagnostics-server`,
    `group-key-management-server`, `groups-server`, `identify-server`,
    `network-commissioning`, `operational-credentials-server`,
    `content-app-observer`

**Default behavior:** When `chip_data_model_overrides_dir` is empty (default),
the template calls `zap_cluster_list.py` as before — no change for non-casting
builds.

### 7.2 Slim Accessors.cpp (~826 KB .o savings)

**Problem:** `chip_data_model.gni` hardcodes the full
`zzz_generated/app-common/app-common/zap-generated/attributes/Accessors.cpp`
which contains attribute getter/setter functions for ALL ~200+ clusters. Only
the 29 casting-relevant clusters' accessors are needed (12 commissioning + 17
casting).

**Mechanism:** The `chip_data_model_overrides_dir` GN argument in
`src/app/common_flags.gni` points to a directory containing well-known override
files. When set to a non-empty path, the `chip_data_model` template compiles
`Accessors-override.cpp` from the override directory instead of the full
generated Accessors.cpp.

**Override file:**
`examples/tv-casting-app/tv-casting-common/Accessors-override.cpp` is a
hand-maintained slim file containing attribute accessor functions for only the
29 casting-relevant clusters. It includes the same `Accessors.h` header and
provides identical function signatures for the retained clusters.

**Retained clusters (29):**

-   **Commissioning (12):** AccessControl, AdministratorCommissioning,
    BasicInformation, Binding, Descriptor, GeneralCommissioning,
    GeneralDiagnostics, GroupKeyManagement, Groups, Identify,
    NetworkCommissioning, OperationalCredentials
-   **Casting (17):** AccountLogin, ApplicationBasic, ApplicationLauncher,
    AudioOutput, Channel, ContentAppObserver, ContentControl, ContentLauncher,
    KeypadInput, LevelControl, LowPower, MediaInput, MediaPlayback, Messages,
    OnOff, TargetNavigator, WakeOnLan

**Default behavior:** When `chip_data_model_overrides_dir` is empty (default),
the full Accessors.cpp is compiled — no change for non-casting builds.

### 7.3 jsoncpp/jsontlv removal (~350 KB — not feasible)

**Problem:** `android_chip_im_jni` and the `jni` shared library in
`src/controller/java/BUILD.gn` both depend on `src/lib/support/jsontlv`, which
pulls in `jsoncpp` (~350 KB).

**Investigation:** Initial analysis suggested the casting app does not use
JSON-TLV conversion at runtime. However, build testing revealed that
`AndroidCallbacks.cpp` and `AndroidInteractionClient.cpp` — both core files in
`android_chip_im_jni` — call `JsonToTlv()`, `TlvToJson()`, and `ConvertTlvTag()`
directly. These functions are used for:

-   Converting command invoke payloads from JSON to TLV
    (`AndroidInteractionClient.cpp`)
-   Converting attribute/event report TLV to JSON for Java callbacks
    (`AndroidCallbacks.cpp`)
-   Converting write attribute payloads from JSON to TLV
    (`AndroidInteractionClient.cpp`)

These are core interaction model operations that the casting app uses at runtime
for sending commands, reading attributes, and subscribing to events on the
remote TV/player endpoint.

**Conclusion:** The jsontlv dependency **cannot be removed** without also
wrapping all call sites in `AndroidCallbacks.cpp` and
`AndroidInteractionClient.cpp` with conditionals and providing alternative code
paths — a significantly larger refactoring effort. The ~350 KB savings is
documented as a future optimization opportunity that would require restructuring
the JNI interaction model layer to separate JSON-based APIs from binary TLV-only
APIs.

### 7.4 Controller code reduction (~1.3 MB — future work)

Setting `chip_support_commissioning_in_controller=false` for the casting app
**will cause link failures**. The dependency chain is too entangled for a simple
flag flip. A clean split requires restructuring the controller library, which is
a larger refactoring effort suitable for a future phase.

#### Dependency chain

```
libTvCastingApp.so (examples/tv-casting-app/android/BUILD.gn :jni)
  └─ tv-casting-common (examples/tv-casting-app/tv-casting-common/BUILD.gn)
       └─ ${chip_root}/src/controller  (src/controller/BUILD.gn :controller)
```

#### What the casting app uses from `src/controller`

1. **`controller/CHIPCluster.h` (`ClusterBase`)** — Used by `BaseCluster.h` and
   `MediaClusterBase` for sending commands, reading attributes, and subscribing
   to the remote TV/player endpoint. In the `:interactions` source set, always
   compiled regardless of `chip_support_commissioning_in_controller`.

2. **`controller/CHIPCommissionableNodeController.h/.cpp`** — Used by
   `CastingPlayerDiscovery` to discover casting targets. Calls
   `DiscoverCommissioners()`, `StopDiscovery()`,
   `RegisterDeviceDiscoveryDelegate()`, and `GetDiscoveredCommissioner()`.

3. **`controller/DeviceDiscoveryDelegate.h`** — Header-only discovery delegate
   interface. Always available.

#### Why the flag flip breaks

Setting `chip_support_commissioning_in_controller=false` excludes **all** `.cpp`
files in the conditional, including two the casting app needs:

-   `AbstractDnssdDiscoveryController.cpp` — base class of
    `CommissionableNodeController`
-   `CHIPCommissionableNodeController.cpp` — discovery of casting targets

The linker fails with unresolved symbols for `DiscoverCommissioners()`,
`GetDiscoveredCommissioner()`, `OnNodeDiscovered()`, etc.

#### Files inside the conditional

| File                                      | Size (approx.) | Needed by casting?   |
| ----------------------------------------- | -------------- | -------------------- |
| `AbstractDnssdDiscoveryController.cpp`    | small          | **YES** — base class |
| `CHIPCommissionableNodeController.cpp`    | small          | **YES** — discovery  |
| `AutoCommissioner.cpp`                    | medium         | No                   |
| `CHIPDeviceControllerFactory.cpp`         | ~90 KB         | No                   |
| `CommissioneeDeviceProxy.cpp`             | medium         | No                   |
| `CommissionerDiscoveryController.cpp`     | medium         | No                   |
| `CommissioningDelegate.cpp`               | medium         | No                   |
| `ExampleOperationalCredentialsIssuer.cpp` | medium         | No                   |
| `SetUpCodePairer.cpp`                     | medium         | No                   |
| `CHIPDeviceController.cpp`                | ~970 KB        | No                   |
| `CommissioningWindowOpener.cpp`           | ~167 KB        | No                   |
| `CurrentFabricRemover.cpp`                | ~86 KB         | No                   |

#### JNI layer analysis

The casting app's `jni` shared library depends on
`${chip_root}/src/controller/java:android_chip_im_jni` (interaction model JNI
bindings), **not** on the full controller JNI (`libCHIPController.so`). The full
controller JNI is deeply entangled with commissioner code
(`AndroidDeviceControllerWrapper`, `CHIPDeviceController-JNI`, etc.), but the
casting app does not use it. The problem is the direct dependency from
`tv-casting-common` on `src/controller`.

#### Future refactoring approach

To achieve the ~1.3 MB savings, the controller library needs a finer-grained
split:

1. **Move discovery code to a separate `:discovery` source set** — Extract
   `AbstractDnssdDiscoveryController.cpp` and
   `CHIPCommissionableNodeController.cpp` into a new `:discovery` source set
   that is always compiled.

2. **Set `chip_support_commissioning_in_controller=false` after the split** —
   With discovery code extracted, the remaining conditional files are all
   commissioner-specific and can be safely excluded.

3. **Estimated effort:** Medium — requires modifying `src/controller/BUILD.gn`,
   updating `tv-casting-common/BUILD.gn` to depend on the split targets, and
   verifying no transitive header dependencies pull in commissioner code. The
   existing TODO comment in `BUILD.gn` notes that "these conditionals are NOT ok
   and should have been solved with separate source sets."

#### Estimated savings breakdown

| File                              | Compiled size (arm64, -Os) | Status                                  |
| --------------------------------- | -------------------------- | --------------------------------------- |
| `CHIPDeviceController.cpp`        | ~970 KB                    | Excludable after split                  |
| `CommissioningWindowOpener.cpp`   | ~167 KB                    | Excludable after split                  |
| `CHIPDeviceControllerFactory.cpp` | ~90 KB                     | Excludable after split                  |
| `CurrentFabricRemover.cpp`        | ~86 KB                     | Excludable after split                  |
| `AutoCommissioner.cpp`            | ~50 KB (est.)              | Excludable after split                  |
| Other commissioner files          | ~50 KB (est.)              | Excludable after split                  |
| **Total potential savings**       | **~1.3 MB**                | Requires controller library refactoring |

### Updated size comparison (arm64-v8a)

| Metric                | Default build | Phase 1         | Phase 2         | Phase 3 (est.)  | Phase 3 reduction |
| --------------------- | ------------- | --------------- | --------------- | --------------- | ----------------- |
| `libTvCastingApp.so`  | 18 MB         | 2.8 MB          | 2.6 MB          | 2.6 MB          | 0 % vs phase 2    |
| `libc++_shared.so`    | 8.9 MB        | 1.3 MB (static) | 1.3 MB (static) | 1.3 MB (static) | —                 |
| **Total native libs** | **26.9 MB**   | **2.8 MB**      | **2.6 MB**      | **2.6 MB**      | **0 %**           |

The phase 3 `.so` size is unchanged from phase 2. The ~1.9 MB of `.o` file
savings from cluster server exclusion and slim Accessors.cpp were already being
eliminated by the linker's `--gc-sections` and thin LTO in earlier phases. The
phase 3 changes reduce compile time and intermediate object sizes, and make the
build explicit about what's needed — preventing regressions if linker behavior
changes upstream.

### Phase 3 GN arguments

All phase 3 overrides are controlled by the consolidated
`chip_data_model_overrides_dir` argument, which is passed automatically by
`android.py` when building the `android-arm64-tv-casting-app-size-optimized`
target:

| Argument                        | Value                                  | Set by                             |
| ------------------------------- | -------------------------------------- | ---------------------------------- |
| `chip_data_model_overrides_dir` | Path to `tv-casting-common/` directory | `android.py` (TV_CASTING_APP only) |

When `chip_data_model_overrides_dir` is set, the build system looks for the
following well-known filenames inside the directory and uses them as source
overrides. When unset (empty string), all consumers fall back to the full
zap-generated sources.

| Well-known filename                         | Purpose                                     | Consumer                       |
| ------------------------------------------- | ------------------------------------------- | ------------------------------ |
| `cluster-objects-override.cpp`              | Slim cluster objects (~29 clusters)         | `src/app/common/BUILD.gn`      |
| `CHIPAttributeTLVValueDecoder-override.cpp` | Slim attribute TLV decoder (18 clusters)    | `src/controller/java/BUILD.gn` |
| `CHIPEventTLVValueDecoder-override.cpp`     | Slim event TLV decoder (18 clusters)        | `src/controller/java/BUILD.gn` |
| `cluster-servers-override.gni`              | Cluster server directory list (13 clusters) | `src/app/chip_data_model.gni`  |
| `Accessors-override.cpp`                    | Slim attribute accessors (29 clusters)      | `src/app/chip_data_model.gni`  |

All GN arguments have inert defaults (empty string) so that non-casting builds
are unaffected.

### Phase 3 override files

| File                           | Location             | Purpose                                                  |
| ------------------------------ | -------------------- | -------------------------------------------------------- |
| `cluster-servers-override.gni` | `tv-casting-common/` | Lists the 13 required cluster server directories         |
| `Accessors-override.cpp`       | `tv-casting-common/` | Slim accessors for 29 casting-relevant clusters          |
| `cluster-objects-override.cpp` | `tv-casting-common/` | (existing, phase 1) Slim cluster objects for 29 clusters |

### Regenerating slim override files

The slim override files can be regenerated from the full zap-generated sources
using the `generate_slim_overrides.py` script. This is useful after upstream ZAP
template changes modify the full generated files.

```bash
# From the repo root, after running `source scripts/activate.sh`:
python3 examples/tv-casting-app/tv-casting-common/generate_slim_overrides.py \
    examples/tv-casting-app/tv-casting-common/casting-cluster-config.yaml \
    --output-dir examples/tv-casting-app/tv-casting-common/
```

The script reads `casting-cluster-config.yaml` which defines three cluster
lists:

-   `casting_clusters` (29 clusters) — used for `cluster-objects-override.cpp`
    and `Accessors-override.cpp`
-   `tlv_decoder_clusters` (18 clusters) — used for the slim TLV decoder files
-   `cluster_servers` (13 directories) — used for `cluster-servers-override.gni`

To add or remove clusters, edit the YAML config and re-run the script. The
script produces byte-identical output when run with the same inputs
(idempotent).

### Phase 3 property tests

Property-based tests for phase 3 are defined in the design document and validate
the correctness of the new override mechanisms. They follow the same pattern as
phase 1/2 tests in `examples/tv-casting-app/tests/`:

| Property                              | What it validates                                                                       |
| ------------------------------------- | --------------------------------------------------------------------------------------- |
| Cluster server set membership         | Override list contains exactly the 13 required clusters, no extras, no duplicates       |
| Slim accessors cluster set membership | Slim accessors file contains exactly the 29 casting clusters' namespaces                |
| Slim accessors signature equivalence  | Function signatures in slim file match full Accessors.cpp for all retained clusters     |
| Phase 3 args target isolation         | `android.py` sets phase 3 args only for TV_CASTING_APP; new GN args have inert defaults |

Additional unit-level validations:

| Validation                       | What it checks                                                                                                          |
| -------------------------------- | ----------------------------------------------------------------------------------------------------------------------- |
| Override dir in GNI              | `common_flags.gni` declares `chip_data_model_overrides_dir` with empty default                                          |
| Data model template conditionals | `chip_data_model.gni` has conditionals for accessors and cluster server overrides using `chip_data_model_overrides_dir` |
| Default preservation             | Default builds (`optimize_apk_size=false`) are unaffected by all phase 3 changes                                        |

### When to run phase 3 tests

-   After modifying `Accessors-override.cpp` or `cluster-servers-override.gni`
-   After changing `chip_data_model.gni`, `common_flags.gni`, `android.py`, or
    `src/controller/java/BUILD.gn`
-   Before pushing a commit that touches any phase 3 files

```bash
python -m pytest examples/tv-casting-app/tests/ -v
```

---

_Updated April 2026 with Phase 3 optimizations: cluster server exclusion, slim
Accessors.cpp, and controller code investigation. jsoncpp/jsontlv removal was
investigated but found not feasible due to runtime usage in the JNI layer._
