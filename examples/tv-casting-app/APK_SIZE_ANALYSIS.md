# TV Casting App — APK Size Analysis

This document analyzes the native library and APK size impact of the
`optimize_apk_size` and `use_static_libcxx` build flags for the Matter TV
Casting App on Android (arm64-v8a).

---

## 1. Optimized vs. Non-Optimized Comparison (arm64-v8a)

| Metric                   | Default build               | Optimized build            | Reduction |
| ------------------------ | --------------------------- | -------------------------- | --------- |
|                          | `optimize_apk_size=false`   | `optimize_apk_size=true`   |           |
|                          | `use_static_libcxx=false`   | `use_static_libcxx=true`   |           |
| **`libTvCastingApp.so`** | 19 MB                       | 2.8 MB                     | 85 %      |
| **`libc++_shared.so`**   | 8.9 MB (shipped separately) | 1.3 MB (statically linked) | 85 %      |
| **Total native libs**    | 27.9 MB                     | 2.8 MB (single `.so`)      | 90 %      |

### What drives the reduction

| Optimization                           | Mechanism                                                                                                                                       | Impact                                                                                                    |
| -------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------- |
| Slim cluster-objects override          | Compile ~36 casting clusters instead of ~200+ via `chip_cluster_objects_source_override`                                                        | Major `.so` reduction                                                                                     |
| Slim TLV decoder overrides             | Compile TLV decoders for 18 casting clusters only via `chip_tlv_decoder_attribute_source_override` and `chip_tlv_decoder_event_source_override` | Removes link-time deps on ~200+ clusters while keeping `ChipClusters.java` read/subscribe APIs functional |
| Remove legacy chip-tool sources        | Exclude ~20 source files + tracing/JSON deps                                                                                                    | Further `.so` reduction                                                                                   |
| Static libc++ with dead-code stripping | `−static-libstdc++` + `--gc-sections` + LTO                                                                                                     | Eliminates separate 8.9 MB `libc++_shared.so`; only referenced symbols survive                            |
| Compiler / linker flags                | `-Os`, `-g0`, `-flto=thin`, `--icf=safe`, `-fvisibility=hidden`                                                                                 | ~10–20 % further `.so` reduction                                                                          |
| R8 Java shrinking                      | `minifyEnabled true` in Gradle debug build                                                                                                      | Reduces DEX size (Java / Kotlin)                                                                          |

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

### Casting-specific clusters (39 clusters, ~157 `.ipp` includes)

**Infrastructure clusters (21):** AccessControl, AdministratorCommissioning,
BasicInformation, Binding, Descriptor, EthernetNetworkDiagnostics, FixedLabel,
GeneralCommissioning, GeneralDiagnostics, GroupKeyManagement, Groups,
IcdManagement, Identify, LocalizationConfiguration, NetworkCommissioning,
OperationalCredentials, SoftwareDiagnostics, TimeFormatLocalization,
TimeSynchronization, UnitLocalization, UserLabel, WiFiNetworkDiagnostics

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
-   Slim TLV decoders (`casting-CHIPAttributeTLVValueDecoder.cpp`,
    `casting-CHIPEventTLVValueDecoder.cpp`) — hand-maintained files covering
    only the 18 casting clusters, selected at build time via
    `chip_tlv_decoder_attribute_source_override` and
    `chip_tlv_decoder_event_source_override`. These keep `ChipClusters.java`
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
-   Full zap-generated TLV decoder files for all ~200+ clusters (replaced by
    slim 18-cluster versions)
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
| `libTvCastingApp.so` (arm64-v8a, optimized)       | 2.8 MB           | Measured — primary size contributor                                    |
| Java casting API classes (`com.matter.casting.*`) | ~0.1 MB          | `TvCastingApp.jar` is 91 KB                                            |
| `CHIPInteractionModel.jar`                        | ~7.3 MB (pre-R8) | Shrinks significantly with R8 — only casting-relevant classes retained |
| `AndroidPlatform.jar`                             | ~54 KB           | Platform abstraction                                                   |
| `CHIPAppServer.jar`                               | ~3.2 KB          | App server bootstrap                                                   |

### Estimated SDK footprint

| Scenario                                               | Estimated size |
| ------------------------------------------------------ | -------------- |
| Native library only (`libTvCastingApp.so`, arm64-v8a)  | 2.8 MB         |
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

-   `chip_cluster_objects_source_override` — slim cluster-objects (~36 clusters)
-   `chip_tlv_decoder_attribute_source_override` — slim attribute TLV decoder
    (18 casting clusters)
-   `chip_tlv_decoder_event_source_override` — slim event TLV decoder (18
    casting clusters)

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

| File                 | Default     | Optimized                  |
| -------------------- | ----------- | -------------------------- |
| `libTvCastingApp.so` | 19 MB       | 2.8 MB                     |
| `libc++_shared.so`   | 8.9 MB      | 1.3 MB (static, folded in) |
| **Total**            | **27.9 MB** | **2.8 MB**                 |

### Desktop — Build with slim cluster set (for cluster vetting)

The tv-casting-app desktop build already uses the slim
`casting-cluster-objects.cpp` (set via `chip_cluster_objects_source_override` in
the platform `args.gni`). This makes it a fast way to verify that the reduced
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
`casting-cluster-objects.cpp` that the optimized Android build now uses, so a
successful build confirms the slim cluster file is self-consistent and no
required symbols are missing.

To run it:

```bash
out/darwin-arm64-tv-casting-app-chip-casting-simplified/chip-tv-casting-app
```

If you modify `casting-cluster-objects.cpp` (e.g. adding or removing cluster
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

| Test file                                 | What it catches                                                                                                                                                                                                                                                       |
| ----------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `test_cluster_set_membership.py`          | The slim attribute and event decoders contain case statements for exactly the 18 casting clusters — no more, no less. Catches accidental cluster additions or removals.                                                                                               |
| `test_non_casting_error_handling.py`      | Both decoders have a `default:` case that sets the correct `CHIP_ERROR_IM_MALFORMED_*_PATH_IB` error and returns `nullptr`. Catches missing error paths for non-casting cluster IDs.                                                                                  |
| `test_interface_compatibility.py`         | The `#include` sets and function signatures (`DecodeAttributeValue`, `DecodeEventValue`) in the slim decoders match the full zap-generated decoders. Catches link-time incompatibilities with the JNI bridge.                                                         |
| `test_decode_logic_equivalence.py`        | For each of the 18 casting clusters, the switch-case body in the slim decoder is identical (modulo whitespace) to the full zap-generated decoder. Catches copy-paste drift when upstream zap templates change.                                                        |
| `test_gn_conditional_source_selection.py` | `src/controller/java/BUILD.gn` contains `if (override != "")` conditionals for both attribute and event decoder overrides, with correct fallback to the zap-generated files. Catches broken GN plumbing.                                                              |
| `test_android_py_casting_args.py`         | `scripts/build/builders/android.py` sets the slim decoder override paths for `TV_CASTING_APP` when `optimize_size` is true, does not disable `matter_enable_tlv_decoder_cpp`, and preserves the existing cluster-objects override. Catches build-script regressions.  |
| `test_preservation_non_casting_builds.py` | `config.gni` declares the override args with empty-string defaults and keeps `matter_enable_tlv_decoder_cpp = true`. The `args.gni` default (non-optimized) block does not set any overrides. Catches regressions that would break non-casting or non-Android builds. |

### When to run them

-   After modifying the slim decoders (`casting-CHIP*TLVValueDecoder.cpp`)
-   After changing the 18-cluster set in any file
-   After editing `BUILD.gn`, `android.py`, `config.gni`, or `args.gni` build
    plumbing
-   Before pushing a commit that touches any of the above

---

_Updated March 2026 with measured build sizes after slim TLV decoder
integration._ _Validates: Requirements 2.1, 2.2, 2.3, 2.4_
