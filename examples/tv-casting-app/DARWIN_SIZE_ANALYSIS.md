# TV Casting App — Darwin(iOS / macOS) Size Analysis

This document analyzes the binary size of the Matter TV Casting framework on
Darwin (iOS/macOS) and describes how iOS developers can integrate a size-reduced
version of the casting library into their apps.

---

## 1. Measured Size Comparison (arm64, macOS host build)

| Metric                           | Default build | Optimized build | Reduction |
| -------------------------------- | ------------- | --------------- | --------- |
| `libTvCastingCommon.a` (archive) | 210 MB        | 144 MB          | 31 %      |
| `__TEXT` (actual code)           | 12.1 MB       | 2.8 MB          | 77 %      |
| `__DATA`                         | 0.4 MB        | 0.3 MB          | 25 %      |

The `.a` archive sizes (210 / 144 MB) include debug info, symbol tables, and
object-file metadata — they overstate the real app impact. The `__TEXT` row is
the aggregate machine-code size across all `.o` files in the archive, which
closely approximates what the linker pulls into the final framework binary
(before dead-code stripping removes even more).

The 77 % code-size reduction (12.1 → 2.8 MB) is consistent with the Android
results and represents the actual savings an iOS app would see in its
`MatterTvCastingBridge.framework` binary.

### How to measure

```bash
#Archive file size(includes metadata — less meaningful)
ls -lh out/darwin-casting-default/lib/libTvCastingCommon.a
ls -lh out/darwin-casting-optimized/lib/libTvCastingCommon.a

#Actual code + data size(sum of __TEXT and __DATA across all.o files)
size out/darwin-casting-default/lib/libTvCastingCommon.a \
  | awk 'NR>1 {t+=$1; d+=$2} END {printf "Default — text: %.1f MB  data: %.1f MB\n", t/1048576, d/1048576}'
size out/darwin-casting-optimized/lib/libTvCastingCommon.a \
  | awk 'NR>1 {t+=$1; d+=$2} END {printf "Optimized — text: %.1f MB  data: %.1f MB\n", t/1048576, d/1048576}'
```

### What drives the reduction

| Optimization                          | Mechanism                                                                                                                    |
| ------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------- |
| `optimize_apk_size=true`              | Excludes ~20 legacy chip-tool source files and their transitive deps (tracing, JSON, jsoncpp, interaction model test suites) |
| `is_debug=false`                      | Removes assert checks, bounds checking, extra logging                                                                        |
| `matter_enable_tracing_support=false` | Excludes tracing/perfetto dependencies                                                                                       |

---

## 2. Architecture Overview

The Darwin casting SDK is delivered as an Xcode framework
(`MatterTvCastingBridge.framework`) that wraps a GN-built static library
(`libTvCastingCommon.a`). The build flow is:

1. Xcode invokes `chip_xcode_build_connector.sh` as a Run Script phase
2. The script runs `gn gen` + `ninja` with args from `darwin/args.gni`
3. GN produces `libTvCastingCommon.a` (a complete static archive)
4. Xcode links the `.a` into `MatterTvCastingBridge.framework`
5. The framework is embedded in the host app (TvCasting example or your app)

Unlike Android, there is no separate `libc++_shared.so` — the Apple toolchain
statically links the C++ standard library by default.

---

## 3. What the Darwin Build Already Optimizes

The darwin `args.gni` already sets the slim cluster-objects override:

```python
chip_cluster_objects_source_override =
    "${chip_root}/examples/tv-casting-app/tv-casting-common/casting-cluster-objects.cpp"
```

This compiles only ~36 casting-relevant clusters instead of the full ~200+
generated `cluster-objects.cpp`. This is the same slim file used by the Android
size-optimized build.

Additionally, the darwin casting bridge uses its own zap-generated Objective-C
cluster objects (`MCClusterObjects.h/.mm`) that are scoped to only the casting
clusters defined in `config-data.yaml`:

-   Application Basic
-   Application Launcher
-   Content Launcher
-   Keypad Input
-   Level Control
-   Media Playback
-   On/Off
-   Target Navigator
-   Wake on LAN

These are the Objective-C API surfaces exposed to Swift/ObjC consumers. The C++
layer underneath (via `libTvCastingCommon.a`) also includes infrastructure
clusters (Descriptor, Binding, OperationalCredentials, etc.) needed for
commissioning and session management.

---

## 4. Optimization Parity with Android

The Android size-optimized build applies several optimizations. Most of these
are now also wired into the darwin build. The remaining gap is `-Os` (optimize
for size), which requires changes to the GN toolchain config and is better
handled as a follow-up:

| Optimization                                                 | Android status | Darwin status                                                           | Potential impact                                                                          |
| ------------------------------------------------------------ | -------------- | ----------------------------------------------------------------------- | ----------------------------------------------------------------------------------------- |
| `optimize_apk_size=true` (excludes legacy chip-tool sources) | ✅ Applied     | ✅ Applied (in `args.gni`)                                              | Significant — removes ~20 legacy source files and their transitive deps                   |
| `is_debug=false`                                             | ✅ Applied     | ✅ Applied (Release builds via Xcode connector)                         | Moderate — removes assert checks and extra logging                                        |
| `matter_enable_tracing_support=false`                        | ✅ Applied     | ✅ Applied (when `optimize_apk_size=true`)                              | Moderate — removes tracing/perfetto deps                                                  |
| `-Os` (optimize for size)                                    | ✅ Applied     | ❌ Not applied                                                          | 10–20% code size reduction                                                                |
| `-g0` (strip debug symbols)                                  | ✅ Applied     | Partial (strip script)                                                  | `strip_debug_symbols.sh` runs in Release but not Debug                                    |
| `-flto=thin` (link-time optimization)                        | ✅ Applied     | ✅ Applied (via tv-casting-common config when `optimize_apk_size=true`) | 5–15% further reduction via cross-TU inlining and dead code elimination                   |
| `-fvisibility=hidden`                                        | ✅ Applied     | ✅ Applied (via tv-casting-common config when `optimize_apk_size=true`) | Reduces exported symbol table, enables more aggressive dead-code stripping                |
| Slim TLV decoders (18 casting clusters)                      | ✅ Applied     | N/A                                                                     | Not applicable — darwin uses Objective-C zap-generated decoders, not the C++ TLV decoders |

### Why slim TLV decoders don't apply to Darwin

On Android, the Java controller uses JNI to call C++ TLV decoder functions
(`DecodeAttributeValue`, `DecodeEventValue`) that switch over cluster IDs. The
full zap-generated decoders handle all ~200+ clusters, pulling in massive
link-time dependencies. The slim decoders replace these with 18-cluster
versions.

On Darwin, the Objective-C bridge (`MatterTvCastingBridge`) uses its own
zap-generated `MCAttributeObjects.mm`, `MCCommandObjects.mm`, etc. that are
already scoped to the casting clusters via `config-data.yaml`. There is no
equivalent of the full C++ TLV decoder path to replace.

---

## 5. How to Enable Size Optimizations for Darwin

### Option A: Set `optimize_apk_size=true` in `darwin/args.gni`

Despite the Android-centric name, `optimize_apk_size` is a shared flag declared
in `tv-casting-common.gni` and controls behavior in the platform-agnostic
`tv-casting-common/BUILD.gn`. Setting it in the darwin args will:

-   Exclude legacy chip-tool command sources (~20 files)
-   Exclude tracing, JSON, and jsoncpp dependencies
-   Enable `-ffunction-sections`, `-fdata-sections`, `-fvisibility=hidden`,
    `-flto=thin` in the tv-casting-common config
-   Reduce the source set to only the simplified casting API

To enable, add to `examples/tv-casting-app/darwin/args.gni`:

```python
optimize_apk_size = true
```

### Option B: Pass additional GN args via the Xcode build connector

The `chip_xcode_build_connector.sh` script builds an `args` array that gets
passed to `gn gen`. You can add size-optimization flags there:

```bash
#In chip_xcode_build_connector.sh, add to the args array:
args+=(
    'optimize_apk_size=true'
    'is_debug=false'
    'matter_enable_tracing_support=false'
)
```

Or pass them as Xcode build settings that the script picks up.

### Option C: Manual `gn gen` for measurement

For measuring the impact without modifying the Xcode project, you can run the GN
build directly:

```bash
source scripts/activate.sh

#Default build(current state) — uses target_os = "mac" so no iOS SDK
#sysroot is needed.The compiled code is the same; only the Mach - O
#platform tag differs, so sizes are representative of iOS.
#
#NOTE : The import() must come first in the args string.GN does not
#allow setting a variable before importing a file that declares it
#with declare_args().
gn gen out/darwin-casting-default --args='import("//examples/tv-casting-app/darwin/args.gni") target_os="mac" target_cpu="arm64" build_tv_casting_common_a=true'
ninja -C out/darwin-casting-default lib/libTvCastingCommon.a

#Size - optimized build
gn gen out/darwin-casting-optimized --args='import("//examples/tv-casting-app/darwin/args.gni") target_os="mac" target_cpu="arm64" build_tv_casting_common_a=true optimize_apk_size=true is_debug=false matter_enable_tracing_support=false'
ninja -C out/darwin-casting-optimized lib/libTvCastingCommon.a

#Compare
ls -lh out/darwin-casting-default/lib/libTvCastingCommon.a
ls -lh out/darwin-casting-optimized/lib/libTvCastingCommon.a
```

To build for iOS specifically (requires Xcode with iOS SDK installed):

```bash
gn gen out/darwin-casting-ios --args='import("//examples/tv-casting-app/darwin/args.gni") target_os="ios" target_cpu="arm64" build_tv_casting_common_a=true sysroot="'"$(xcrun --sdk iphoneos --show-sdk-path)"'"'
ninja -C out/darwin-casting-ios lib/libTvCastingCommon.a
```

---

## 6. Framework Composition

### What's in `MatterTvCastingBridge.framework`

| Layer                         | Description                                                                                                                                                       |
| ----------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Objective-C API               | `MCCastingApp`, `MCCastingPlayer`, `MCCastingPlayerDiscovery`, `MCEndpoint`, `MCCluster`, `MCCommand`, `MCAttribute` — the public API surface for Swift/ObjC apps |
| Zap-generated cluster objects | `MCClusterObjects`, `MCCommandObjects`, `MCAttributeObjects`, `MCCommandPayloads` — typed wrappers for the 9 casting clusters                                     |
| Compat shim (legacy)          | `CastingServerBridge`, `ContentApp`, `DiscoveredNodeData`, etc. — deprecated API, can be excluded in new integrations                                             |
| `libTvCastingCommon.a`        | The GN-built static archive containing Matter core, casting common C++ layer, and all transitive deps                                                             |

### What's in `libTvCastingCommon.a`

The static archive is built with `complete_static_lib = true`, meaning it
bundles all transitive dependencies into a single `.a` file:

-   Matter core (transport, crypto, secure channel, interaction model)
-   Casting common C++ layer (CastingApp, CastingPlayer, Endpoint, etc.)
-   Cluster implementations (~36 via slim `casting-cluster-objects.cpp`)
-   App server, data model provider
-   Darwin platform layer (BLE, DNS-SD via Bonjour, KeyValueStore)
-   mbedTLS (crypto backend)
-   Device attestation, credential management

---

## 7. Integration Guide for iOS Developers

### Minimal integration (recommended)

1. Build `MatterTvCastingBridge.framework` from the Xcode workspace
2. Embed the framework in your app target
3. Import `MatterTvCastingBridge` in your Swift/ObjC code
4. Use the `MC*` API classes (`MCCastingApp`, `MCCastingPlayerDiscovery`, etc.)

The framework already contains only the casting-relevant clusters. No additional
cluster reduction is needed on the Objective-C side.

### Size-conscious integration

For apps where binary size is critical:

1. Enable `optimize_apk_size=true` in `darwin/args.gni` to exclude legacy
   chip-tool sources from `libTvCastingCommon.a`
2. Remove the compat-shim sources from the Xcode project if you're using only
   the new `MC*` API (not the deprecated `CastingServerBridge`)
3. Build in Release configuration to trigger `strip_debug_symbols.sh`
4. Enable Xcode's "Dead Code Stripping" (`DEAD_CODE_STRIPPING = YES`) and "Strip
   Linked Product" (`STRIP_INSTALLED_PRODUCT = YES`)

### What you can safely exclude

If you're building a new app (not migrating from the old API):

-   The entire `compat-shim/` directory (~26 files) — these are the deprecated
    `CastingServerBridge` API wrappers
-   `chip_build_libshell` can be set to `false` (already done when
    `optimize_apk_size=true`)

---

## 8. Comparison with Android

| Aspect                  | Android                                                     | Darwin (iOS/macOS)                                          |
| ----------------------- | ----------------------------------------------------------- | ----------------------------------------------------------- |
| Delivery format         | `.so` shared library + `.jar` files                         | `.framework` (static lib + ObjC headers)                    |
| C++ stdlib              | Separate `libc++_shared.so` (or static)                     | Always statically linked by Apple toolchain                 |
| Cluster API surface     | Java `ChipClusters.java` (all ~200+ clusters, shrunk by R8) | ObjC `MCClusterObjects` (9 casting clusters only)           |
| TLV decoders            | C++ `CHIPAttributeTLVValueDecoder.cpp` (full or slim)       | ObjC zap-generated `MCAttributeObjects.mm` (already scoped) |
| Slim cluster-objects    | ✅ via `chip_cluster_objects_source_override`               | ✅ via `chip_cluster_objects_source_override`               |
| Legacy source exclusion | ✅ via `optimize_apk_size=true`                             | ✅ via `optimize_apk_size=true` (Release builds)            |
| Build system            | GN → ninja → Gradle                                         | GN → ninja → Xcode                                          |

The darwin build is already in a better starting position than the default
Android build because:

1. The Objective-C cluster API is pre-scoped to casting clusters (no equivalent
   of the 200+ cluster `ChipClusters.java`)
2. The slim `casting-cluster-objects.cpp` is already set in `args.gni`
3. There's no separate C++ stdlib shared library to worry about

The main remaining opportunity is enabling `optimize_apk_size=true` to drop the
legacy chip-tool sources and their transitive dependencies.

---

_March 2026 — Initial analysis of Darwin casting framework size._
