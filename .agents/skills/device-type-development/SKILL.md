---
name: device-type-development
description: >
    Step-by-step guide for adding a new Matter device type to
    `examples/all-devices-app` (generic type class, simulated impl, build
    wiring, OOB accessors and named-pipe plumbing, CI tests, docs tables). The
    layout contract, PR-splitting strategy, and settled review-recurrence rules
    for device-type changes.
---

# Device Type Development

This guide covers adding a **device type** to `examples/all-devices-app`. It
complements `code-driven-cluster-development` (which covers clusters living in
`src/app/clusters`) — a device type PR usually does **not** touch
`src/app/clusters` unless a required cluster is still ember-based, in which case
that cluster must land first (see `code-driven-cluster-migration`).

---

## Layout Contract (do not violate)

Current-tree layout:

```
examples/all-devices-app/all-devices-common/device/
├── api/                    # DeviceInterface / SingleEndpoint base classes
├── capabilities/           # SHARED implementations reused by 2+ device types ONLY
└── types/<device-type>/
    ├── BUILD.gn            # source sets: ":<device-type>", a named impl set
    │                       #   (":simulated", ":logging", or a flavor name), and
    │                       #   optional ":posix"
    ├── <TypeName>.h        # GENERIC device type: spec-mandated bits only
    ├── <TypeName>.cpp
    ├── OOBAccessors.{h,cpp}          # optional: register accessors against this device
    ├── <Thing>Accessor.{h,cpp}       # optional: per-cluster OOB accessor, colocated
    │                                 #   (soil-sensor, power-source, water-valve)
    ├── NamedPipeTranslators.{h,cpp}  # optional: POSIX named-pipe registration (":posix")
    ├── impl/               # simulated example device; may import types/, never the reverse
    └── tests/              # unit tests for the generic type
```

> **Legacy device warning**: older device types in `device/types/` that lack an
> `impl/` folder predate the current architecture. **Do not copy them** (e.g.
> `chime` still lacks impl separation). Follow only newly structured exemplars:
> `window-covering`, `mode-select`, `robotic-vacuum-cleaner`,
> `thread-border-router`, `network-infrastructure-manager`,
> `air-quality-sensor`, `closure`, `closure-panel`. When naming conventions
> conflict between older and newer devices, copy the _newest_ device and name
> the impl after what it simulates.

Rules (the usual comments that recur in device-type code reviews — "pay
attention to these"):

1.  **`types/<t>/<T>.h` = generic.** Must compile for any app. Hardcode nothing
    the spec does not mandate: features, optional attributes, and optional
    clusters are all injected (`Config.features` / `OptionalAttributeSet`),
    configured via variadic cluster-ID templates (see `AirQualitySensor.h` for
    the many-optionals form; plain virtual hooks are fine for 1–2 optionals), or
    deferred to a `virtual RegisterOptionalClusters()` hook that only subclasses
    implement. The base `Register()` registers mandatory clusters, then calls
    the hook before `provider.AddEndpoint()`; `Unregister()` mirror-hooks
    accordingly after `UnregisterDescriptor()`.
2.  **`impl/` is a strict one-way dependency**: `impl/` may import `types/`,
    `types/` must never import or construct anything from `impl/`. No
    simulation/json/pipe logic in the generic class.
3.  **`capabilities/` is for genuinely shared, multi-device-type pieces.** When
    in doubt, create no capability at all — put the delegate in the device's own
    `impl/`. Fewer layers beats more inheritance.
4.  **Prefer one `impl/<Flavored><Device>` class** implementing/owning all
    delegates over several delegate classes that point to each other.
    Cross-cluster state belongs in one object with one state machine, not
    booleans scattered across delegates. Composite multi-endpoint devices
    (closure, irrigation-system) are the exception: one parent + child parts.
5.  **Dependencies are references, not nullable pointers.** Pack them in a
    `Context` / `Delegates` struct (see `WindowCovering.h`) and inject from
    factory setup. Mandatory peers (e.g. Run Mode for RVC) are references;
    constructor sequencing guarantees them — no runtime null checks (the
    Identify delegate, for example, must be set at construction).
6.  **Model device state as a single enum** (`Docked`, `DockedCharging`,
    `Running`, `Paused`, …) with the transition diagram in a doc comment (see
    `SimulatedWindowCovering`). Two loosely-coupled booleans caused a real bug
    class in a past device type.
7.  **No globals inside device-type code.** Timer delegates, storage, etc. are
    injected.
8.  **Name the impl source set and class after the simulation**
    (`Simulated<T>`/`:simulated` for general simulators such as window-covering;
    `Logging<T>`/`:logging` for log-driven impls such as water-valve; a flavor
    name when multiple variants exist — `temperature-sensor:increasing`,
    `soil-sensor:increasing`). The flavor name beats the generic
    `simulated`/`logging` label.

---

## Settled Idioms (the usual review comments, in rule form)

**Dependency & lifecycle**

-   `Context` / `Delegates` structs with **references** (see
    `WindowCovering.h`); "inject all the things" — concrete member objects,
    inherited/hardcoded delegates, and globals are all rejected. Shared
    app-level state (e.g. BreadCrumbTracker) lives in `DeviceFactory::Context`,
    not per-device duplicates.
-   No heap in device code: use `LazyRegisteredServerCluster<T>` instead of
    `unique_ptr` cluster members, and never use `std::function` timer callbacks
    (use `TimerDelegate` + member state).
-   Wrap registration in `DeviceRegistrationTransaction` + `Commit()` for
    partial-install rollback; guard cluster teardown with
    `if (mCluster.IsConstructed())` and child-part unregistration via
    `GetEndpointId() != kInvalidEndpointId`; capture the endpoint ID before
    `UnregisterDescriptor()` if teardown needs it —
    `SingleEndpoint::UnregisterDescriptor()` resets `mEndpointId`;
    `VerifyOrDie(IsConstructed())` on cluster accessors used before
    `Register()`; registration order matters when one cluster's `Startup()`
    touches another's storage.
-   **`DeviceRegistrationEntry` owns the public lifecycle**: callers use
    `entry.Register(...)` / `entry.Unregister(...)`, which run the
    `AfterDeviceRegistration` / `BeforeDeviceUnregistration` hook pair as
    `static` template capabilities (`if constexpr`-gated, fold-expression order)
    **while the device is still valid**. Every hook class in the tree (including
    no-op hooks) must implement _both_ methods — a missing method breaks
    other-platform builds under fold instantiation.
-   Inherited delegates:
    `impl/<Logging|Simulated><Device> : private <Cluster>Delegate, public <LeafDevice>`
    — delegate bases (which pass `*this`) must be listed before the device base
    so they are constructed first; the newest form takes
    `(const Context&, const <X>Config& = {})` forwarding the config as
    `Base(context, *this, config)`.
-   Attributes: `SetAttributeValue()` = set + notify atomically, skip no-op
    churn; for correlated clusters, sync each side explicitly on transitions
    (e.g. power-Off → LocalTemperature NULL + SystemMode off); spec-nullable
    values use `DataModel::Nullable`, not `std::optional`; never report
    attribute changes for state you didn't actually change (e.g. a revert path
    wiping a _committed_ value); simulation must read current state via cluster
    getters, never private shadow caches, or OOB/in-band writes are clobbered by
    timers.
-   Test-event-trigger handlers: register at device startup, unregister at
    shutdown — never link-time-resolved globals; handler must match endpoint ID
    and fall through otherwise.
-   Errors: `LogErrorOnFailure` / propagate — **no new
    `TEMPORARY_RETURN_IGNORED`** (it is a legacy transition marker; grep your
    diff for it before submitting — reviewers keep flagging it); no `(void)`
    casts to swallow `CHIP_ERROR`.

**Build & naming hygiene**

-   Alphabetical order everywhere (`DeviceFactory.h`, `enabled_devices.*`,
    `oob-accessors` registration lists, `Kconfig.devices`, `targets.py`
    keep-sorted blocks) — order violations recur.
-   `BUILD.gn` depends on what it uses directly (`lib/support`,
    `app/persistence`), never transitive includes; `device-factory/BUILD.gn`
    must publicly depend on the impl source set it instantiates
    (`mode-select:simulated`, `temperature-sensor:increasing`).
-   GN deps follow the consumer: when an optional cluster moves from the generic
    type into `impl/`, its cluster deps move out of the base source set into the
    impl set with it, and the generic header's includes and `Context` members
    referencing it go too (reviewers will ask "do we still need these here?").
-   No `Device` suffix on classes/filenames; spell device names out (no PICS
    acronyms); no nested-stutter types (`WaterValve::Listener`, not
    `WaterValve::WaterValveListener`); constants `k`-prefixed with units; doc
    comments on public members only in `.h`, never duplicated in `.cpp`; no word
    "legacy" in new code or comments.
-   Compile-time configurability beats build flags: template-ID-keyed `Config`
    structs + variadic optional clusters for many-optionals devices
    (`config.Get<CID>()`, see `AirQualitySensor`); named fixed-feature classes
    like `FullThermostat` when a cluster's C++ type encodes its feature map.
    Never `namespace Detail` or helpers visible through headers that
    `DeviceFactory.h` transitively includes (`namespace ...Internal` instead).
-   Name files after what they contain (a "Features" header that only held a
    concat helper was flagged in review).

**OOB accessor contract (newest practice)**

-   Handle-and-finish or get out of the way:
    `HandleAction(CharSpan action, ByteSpan tlvData)` returns the final
    `CHIP_ERROR` when the accessor handles the action, `std::nullopt` when it
    doesn't — _never_ `CHIP_IM_GLOBAL_STATUS(UnsupportedWrite)`, which blocks
    registry dispatch and Pigweed fall-through.
-   Constructor takes `(Cluster&, EndpointId)`, possibly a convenience
    `(Device&)` overload; accessors are colocated with the device in
    `device/types/<t>/<Thing>Accessor.{h,cpp}` and compiled into the base
    `source_set("<t>")` depending on `oob-accessors` core (soil-sensor,
    power-source, water-valve) — the older centralized per-cluster targets in
    `oob-accessors/clusters/BUILD.gn` are historical.
-   Parse with `OOBDataSerializer::ParseAttributeRequest`, decode with
    `DataModel::Decode`, validate enums/ranges with
    `VerifyOrReturnError(..., CHIP_IM_GLOBAL_STATUS(ConstraintError))`;
    in-band/OOB coexistence is one state — track convenience state like
    last-open level in the accessor, not the device class.
-   Always ship `Test<Thing>Accessor.cpp` in `oob-accessors/tests/` using typed
    `OOBDataSerializer::BuildSetAttributeRequest(path, value)`; include
    unhandled-cluster and out-of-range cases.
-   `OOBAccessorHook.h` ties accessor lifetime to the device via an RAII
    `WithDependency(&device)` scope; `Unregister(dep)` removes only that
    device's accessors so process-lifetime ones survive; the Noop registry
    mirrors the exact API including `[[nodiscard]]`.
-   The DataModel fallback (what happens when no accessor handles a write)
    belongs in the transport caller (Pigweed Attributes service / named-pipe
    dispatcher), _never_ inside `InMemoryOOBAccessorRegistry` — and injected
    `DataModel::Provider`, no globals.

**Simulation quality**

-   No-op delegates are rejected ("too fake"): ship real conversions/mocks with
    a chip-tool verification recipe (see `PlanckianColorConverter`). Timers
    self-reschedule and transitions are timer-driven so `Stop` is meaningful;
    clamp unsigned arithmetic; treat generated optionals as null (`ValueOr`
    path).
-   An impl that returns Unsupported from most delegate callbacks fails its
    purpose: every attribute/feature your config _enables_ needs a real delegate
    surface (optional setpoint-limit getters returning `UnsupportedAttribute`
    broke `LoadSetpoints`, `SetpointRaiseLower`, and the TC tests of that
    device). Likewise: rollback Unregister if `StartTimer` fails in `Register`.
-   Filter valid-but-unsupported enum values with `ConstraintError`, not
    `InvalidValue` — and don't let the config advertise a feature the filter
    rejects (an advertised-but-rejected mode resets to Off on startup).
-   **Config defaults must match `ci-pics-values`**: the yaml tests run against
    shared CI PICS, so a device whose `Config{}` omits features/attributes the
    PICS enable fails its own CI. Enable what the PICS promise, or adjust
    guards.
-   No srand-seeded "randomness" to mask CI flakes — reviewers call it out.
-   Device factory presets belong in the device's `impl/`, not
    `DeviceFactory.h`; child-only device types (`closure-panel`) are NOT
    independently enableable — `enabled_devices.*` lists parent-capable types
    only.
-   Delegates are implemented by the impl, never by the generic device class;
    remove duplicated cluster APIs from the device class — callers reach e.g.
    `Open()` through the `...Cluster()` getters.

---

## Step-by-Step Procedure

### Step 0 — Discover the app's own documentation first

Per `AGENTS.md` guidance, read before writing:

-   `examples/all-devices-app/docs/supported_device_types.md` — device type
    table + discovery methodology (which clusters are `[O]`/optional).
-   `examples/all-devices-app/docs/supported_clusters.md` — which clusters are
    code-driven in the SDK vs ember.
-   `examples/all-devices-app/docs/architecture.md`,
    `examples/all-devices-app/docs/adding_new_device.md` (the checklist of
    record — add a documented step for any new wiring location), and
    `examples/all-devices-app/docs/design/out_of_band_control.md`.

Record from your device type: its spec ID, every **server** cluster with
conformance M vs O, per-cluster feature maps and optional attributes, and the
split `Ready` / `Missing` against `supported_clusters.md`. If mandatory clusters
are missing, the type is **Blocked** — the blocker is a cluster PR, not a
device-type PR.

### Step 1 — Plan the PR series

Split, in this order, each merging independently and compiling cleanly:

-   **PR 1 — base device type + minimal simulated impl:** generic
    `types/<t>/<T>.{h,cpp}` with mandatory clusters only, a minimal compilable
    `impl/<Simulated|Logging|Flavor><T>.{h,cpp}` satisfying mandatory cluster
    `Delegate &` references, `BUILD.gn`, `DeviceFactory` + all build wiring,
    unit tests for `<T>`, and docs rows. No OOB/named pipes or complex
    multi-cluster state machines yet.
-   **PR 2 — realistic mandatory simulation:** full timer-driven / physically
    plausible mandatory cluster behavior in `impl/` (if non-trivial enough to
    separate from PR 1).
-   **PR 3 — optionality:** optional clusters enabled through
    `RegisterOptionalClusters()` in impl (or template parameters like
    `AirQualitySensor`), with optional attributes/feature bits turned on.
-   **PR 4 — cross-cluster wiring ("if-this-then-that")** between clusters
    (modes drive opstate, opstate drives progress reporting) and state machines.
-   **PR 5 — OOB / named pipes + CI:** `<Thing>Accessor`, `OOBAccessors`,
    `NamedPipeTranslators`, yaml TC suites enabled, and/or `python_testing`
    `TC_*.py` files extended.

Rationale: single-purpose PRs over a pre-agreed layout review fast; PRs that
introduce a new layout or placement decision inside a large mixed diff get that
design re-litigated mid-review and every fix serializes behind it. Device-type
PRs that mixed all of these together took weeks; a minimal base + impl lands in
days and later PRs reuse its settled vocabulary. Each follow-up PR must build
alone — a stray hunk referencing an API added in the _next_ split PR breaks the
intermediate builds.

**Names come first:** if you are unsure about any file/class naming, raise that
naming as the _first_ PR (or at least the first comment) — class/location names
are the last thing to settle and force broad renames.

### Step 2 — Write the generic device type

Start from the newest clean exemplar and _read that file_ rather than a code
template pasted here (inline code in skills drifts out of sync with the tree):
`all-devices-common/device/types/window-covering/WindowCovering.{h,cpp}`. The
invariants to reproduce, all visible there:

-   Inherits `SingleEndpoint`; members for each mandatory cluster are
    `LazyRegisteredServerCluster<T>`.
-   Dependencies packed in a `Context` struct (references only), cluster
    delegates in a `Delegates` struct (references only), and spec-gated knobs in
    a `Config` struct (`BitFlags<Feature>`, `OptionalAttributeSet`, any spec
    enum defaults).
-   Constructor takes the three structs; `Register()`/`Unregister()` override
    the `DeviceInterface` lifecycle hooks with the exact ordering below; public
    getters return live cluster references for programmatic/OOB control;
    optional clusters enter through the protected `RegisterOptionalClusters()` /
    `UnregisterOptionalClusters()` hooks.

Key lifecycle invariants in `Register()` and `Unregister()`:

-   **`Register(endpoint, provider, composition)`**:
    1.  `VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);`
    2.  `DeviceRegistrationTransaction transaction(*this, provider);`
    3.  `ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));`
        **first**.
    4.  `.Create(...)` and `ReturnErrorOnFailure(provider.AddCluster(...));` for
        each mandatory cluster.
    5.  `ReturnErrorOnFailure(RegisterOptionalClusters(endpoint, provider));`
    6.  `ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));`
        **last** — `CodeDrivenDataModelProvider` rejects `AddCluster` after
        `AddEndpoint` once started (`CHIP_ERROR_INCORRECT_STATE`).
    7.  `transaction.Commit();`
-   **`Unregister(provider)`**:
    1.  Call `UnregisterDescriptor(provider);` **first** —
        `CodeDrivenDataModelProvider` rejects `provider.RemoveCluster()` while
        the endpoint is still registered (`CHIP_ERROR_INCORRECT_STATE`).
    2.  **`mEndpointId` reset trap:**
        `SingleEndpoint::UnregisterDescriptor(provider)` resets
        `mEndpointId = kInvalidEndpointId`. Do not check `mEndpointId` inside
        `UnregisterOptionalClusters()`; instead guard each cluster with
        `if (mCluster.IsConstructed())`.
    3.  Call `UnregisterOptionalClusters(provider);`, then
        `LogErrorOnFailure(provider.RemoveCluster(...))` and `.Destroy()` on
        each constructed mandatory cluster.
    4.  _(Composite devices inheriting `DeviceInterface`, e.g.
        `closure/Closure.cpp` — `UnregisterParts`, or irrigation-system's
        `RegisterParts`/`UnregisterParts` with `EndpointIdAllocator`)_:
        Unregister child part endpoints **before** calling
        `UnregisterDescriptor(provider)` on the parent endpoint.
-   Mandatory-cluster defaults only (do **not** initialize optional attributes
    like current phase/mode from the base class — the spec does not require it
    and a wrong default logs failures at startup; subclasses init what they
    simulate).
-   Do not add JSON/pipe/simulation headers here.
-   Lifecycle goes through `DeviceRegistrationEntry` (`Register()` then
    `AfterDeviceRegistration`, `BeforeDeviceUnregistration` then
    `Unregister()`); your device should not run global registration itself.

### Step 3 — Write the simulated impl

`impl/<Simulated|Logging|Flavor><Device>.h/.cpp`, subclassing the generic type
and overriding `RegisterOptionalClusters()` / `UnregisterOptionalClusters()` for
optional bits (see `window-covering/impl/SimulatedWindowCovering.{h,cpp}` and
`robotic-vacuum-cleaner/impl/SimulatedRoboticVacuumCleaner.{h,cpp}`):

-   One state enum + transition diagram (doc comment) instead of parallel
    booleans.
-   Make the simulation _physically plausible with real timers_ — a covering
    that instantly jumps from lift 0 to 255 makes `Stop` meaningless; a tick
    timer advancing state by a step every N ms makes a mid-flight `Stop` freeze
    the current value. (A `TimerDelegate` must be `CancelTimer`d before every
    restart; re-arm defensively.)
-   Cross-cluster reactions ("if-this-then-that") live here as explicit methods,
    e.g. a pause handler stores the prior state **after** the base call succeeds
    (storing before leaves a stale-resume state); entering `Running` clears any
    docked/charging flags.
-   Read the current state through cluster getters before applying increments —
    no shadow caches (see `GetSoilMoistureMeasuredValue()`); accessor-level
    convenience state (e.g. last-open level) lives in the accessor.
-   Arithmetic on attribute-like values: clamp unsigned math
    (`x -= std::min(x, step)`), treat generated accessor optionals as possibly
    null (`ValueOr` + a "not moving" path) — never assume non-null in sim code.
-   Bounds-check every list index against its container size before access.
-   Transport/JSON parsing is _not_ here; external control goes through
    `<Thing>Accessor` + `posix/named_pipe/translators/` (see Step 5).

### Step 4 — Build wiring

For each device type touch **all** of (missing any one is a review item; keep
all lists alphabetically sorted):

-   `all-devices-common/device/types/<t>/BUILD.gn` — `source_set("<t>")` for
    `<T>.{h,cpp}` plus `OOBAccessors.{h,cpp}` and `<Thing>Accessor.{h,cpp}` if
    present; a named impl source set for the `impl/` device; and an optional
    `source_set("posix")` for `NamedPipeTranslators.{h,cpp}` (see
    `mode-select/BUILD.gn`; note `window-covering` currently has no `:posix`).
-   `all-devices-common/device-factory/enabled_devices_config.h.in` — add
    `#cmakedefine01 ALL_DEVICES_ENABLE_<UPPER_SNAKE>` (mandatory for CMake
    platforms).
-   `all-devices-common/device-factory/enabled_devices.gni` — add
    `[ "<kebab-name>", "<UPPER_SNAKE>" ]` to `_available_devices`.
-   `all-devices-common/device-factory/enabled_devices.cmake` — update **both**
    `ALL_DEVICES_DEVICE_SOURCES` (both `<T>.cpp` and `impl/<Impl><T>.cpp`) and
    `foreach(_key ...)` with `<kebab-name>:<UPPER_SNAKE>`.
-   `all-devices-common/device-factory/DeviceFactory.h` — include
    `<device/types/<t>/impl/<Impl><T>.h>` and register inside
    `if constexpr (ALL_DEVICES_ENABLE_<UPPER_SNAKE>)`:
    `RegisterCreator("<kebab-name>", [this]() { ... })`.
-   `all-devices-common/device-factory/BUILD.gn` — add
    `"${chip_root}/examples/all-devices-app/all-devices-common/device/types/<t>:<impl-set>"`
    to `public_deps` (an unreferenced impl source set silently drops from the GN
    graph).
-   `examples/all-devices-app/posix/BUILD.gn` — add `device/types/<t>` (and
    `device/types/<t>:posix` if `NamedPipeTranslators` exist) to `public_deps`.
-   `examples/all-devices-app/silabs/BUILD.gn` — add the
    `if (enable_<snake>_device)` dependency block.
-   `examples/all-devices-app/zephyr/Kconfig.devices` — add a plain
    `config ALL_DEVICES_DEVICE_<UPPER_SNAKE>` `bool` entry (one line; keep
    sorted, keep in sync with `enabled_devices.cmake` — devices have been found
    missing here before), or Zephyr builds can never select the new device.
-   If you add build **targets** (`scripts/build/build/targets.py`, inside
    `_ALL_DEVICES_APP_DEVICES` with `# keep-sorted`), regenerate
    `scripts/build/testdata/all_targets_linux_x64.txt` using
    `scripts/run_in_build_env.sh "python3 scripts/build/test.py"` — hand-edits
    have reintroduced darwin rows into a linux list before.

### Step 5 — Simulation plumbing (Out-of-Band Accessors & POSIX Named Pipes)

Follow `examples/all-devices-app/docs/design/out_of_band_control.md` and the
`robotic-vacuum-cleaner` / `mode-select` exemplars plus the newest accessor
exemplars (soil-sensor, power-source, water-valve):

-   **Per-cluster OOB Accessor (`device/types/<t>/<Thing>Accessor.{h,cpp}`,
    newest placement)**:
    -   `<Thing>Accessor : public OOBAccessor` overriding
        `std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData)`;
        return `std::nullopt` for anything not yours. For attribute writes,
        filter on the SetAttribute action and parse via
        `OOBDataSerializer::ParseAttributeRequest`. For custom actions use a
        semantic action string (e.g. `"SetOccupancy"`).
    -   Compiled into the device's base `source_set("<t>")` which `public_deps`
        `oob-accessors` core; registration via the sibling
        `device/types/<t>/OOBAccessors.{h,cpp}` free function
        `RegisterOOBAccessors(<Device> & device, OOBAccessorRegistry & registry)`.
        (If simulation hooks exist only on the impl, provide a no-op overload
        for `<Device> &` and the real one for the impl type; see
        `robotic-vacuum-cleaner/OOBAccessors.cpp`.)
    -   `oob-accessors/OOBAccessorHook.h` includes the device's `OOBAccessors.h`
        — `OOBAccessorHook` automatically ties accessor lifetime to the device
        via `WithDependency(&device)` and unregisters on teardown.
    -   Add both `.cpp` files to
        `all-devices-common/oob-accessors/all_devices_config.cmake`.
    -   The centralized per-cluster source sets under
        `oob-accessors/clusters/BUILD.gn` (`:on-off`, `:occupancy`, `:rvc`, …)
        are the older placement — reuse what's there for those clusters, but put
        _new_ accessors next to the device.
-   **POSIX Named-Pipe Translator (`posix/named_pipe/translators/`)**:
    -   Implement `<Cluster>Translator.{h,cpp}` inheriting from
        `NamedPipe::Translator` (`Match` + `Translate` from JSON to TLV), wired
        into `posix/named_pipe/BUILD.gn` and unit-tested in
        `posix/named_pipe/tests/TestNamedPipeTranslators.cpp`.
    -   Implement `device/types/<t>/NamedPipeTranslators.{h,cpp}`
        (`RegisterNamedPipeTranslators` calling
        `dispatcher.EnsureTranslatorRegistered<NamedPipe::<Cluster>Translator>()`),
        built in `source_set("posix")` in `device/types/<t>/BUILD.gn`, and
        include `<device/types/<t>/NamedPipeTranslators.h>` in
        `posix/named_pipe/Hook.h`.
-   Give the simulated device a `Reset`-like OOB/pipe entrypoint that restores
    _all_ sim state (timers, dock state, pre-pause state) — CI tests replay
    state repeatedly and a missing reset causes cross-test flakiness.

### Step 6 — Tests and CI

-   Enable the device type on `all-devices-app` targets in CI by adding the
    cluster's yaml suites in
    `src/app/tests/suites/certification/Test_TC_*.yaml`:
    -   **Trap when adding a `CI:` block:** YAML tests without a `CI:` block run
        against `all-clusters` by default. Adding a `CI:` block overrides that
        default, so you **must** include both the existing `app: all-clusters`
        entry and the new `app: all-devices` (with
        `app-args: "--device <kebab-name>:1"`) entry.
    -   Only enable **server-side** suites: `*_Simulated.yaml` are client-PICS
        tests for simulated _client_ behavior — adding a server app there was
        explicitly rejected in review.
-   For `python_testing/TC_*.py` scripts, add the `all-devices-app` run config
    and include `EndpointId` in named-pipe commands (endpoint-scoped resets).
-   Unit tests for the generic type go in `device/types/<t>/tests/` mirroring
    `air-quality-sensor/tests/`, and must be registered in `examples/BUILD.gn`
    under `chip_test_group("example_tests")`.

### Step 7 — Docs tables

Update all documentation tables in the same PR as their functional change:

-   `examples/all-devices-app/docs/supported_device_types.md`:
    -   For a **new device type**: move its row from
        `## Unimplemented Device Types (M total)` to
        `## Implemented Device Types (N total)`, update both `(N total)` and
        `(M total)` header counts, and renumber the `#` column in both tables.
    -   For a **newly implemented optional cluster**: remove it from the
        `Missing Clusters` column (the methodology is defined in the file header
        — follow it even when bots disagree; bots have "flagged" this the
        opposite way before).
-   `examples/all-devices-app/docs/supported_clusters.md`: flip the cluster's
    `Used in All-Devices` column to `Yes`, add the device to `Notes`, and
    recompute the `**Total**` summary row at the bottom — stale totals were a
    separate review comment before.
-   `examples/all-devices-app/README.md`: add the device to the **Supported
    Devices** list and the `--device` CLI help output block.
-   Keep `supported_device_types.md` status notes consistent with
    `supported_clusters.md` in the same commit; reviewers notice drift between
    them.

---

## Pre-Review Self-Checklist

-   [ ] Base device type contains only spec-mandated state; features, optional
        attributes, and optional clusters all behind `Config`, template
        parameters, or `RegisterOptionalClusters()`.
-   [ ] `types/` imports nothing from `impl/`; `impl/` is a separate named
        source set whose class is named after what it simulates; anything in
        `capabilities/` is used by ≥2 device types.
-   [ ] Registration wired with `DeviceRegistrationTransaction` + `Commit()`:
        `RegisterDescriptor` first, mandatory + optional `AddCluster` next,
        `provider.AddEndpoint` last.
-   [ ] `Unregister` calls `UnregisterDescriptor(provider)` **first** before
        `UnregisterOptionalClusters` and `provider.RemoveCluster`; clusters
        guarded with `IsConstructed()`; delegate bases ordered before device
        base; lifecycle goes through `DeviceRegistrationEntry`'s after/before
        hooks.
-   [ ] All parallel build/config files touched (alphabetical):
        `device/types/<t>/BUILD.gn`, `enabled_devices_config.h.in`,
        `enabled_devices.gni`, `enabled_devices.cmake` (both sources and
        `foreach`), `DeviceFactory.h`, `device-factory/BUILD.gn` (impl set),
        `posix/BUILD.gn`, `silabs/BUILD.gn`, `zephyr/Kconfig.devices`.
-   [ ] If OOB / NamedPipe support is added: `<Thing>Accessor` compiled in
        `source_set("<t>")` with a dep on `oob-accessors`,
        `device/types/<t>/OOBAccessors.{h,cpp}` included from
        `oob-accessors/OOBAccessorHook.h`, both `.cpp` files in
        `oob-accessors/all_devices_config.cmake`, and
        `device/types/<t>/NamedPipeTranslators.{h,cpp}` (`:posix`) in
        `posix/named_pipe/Hook.h` + `posix/BUILD.gn`.
-   [ ] (Conditional — only if adding build targets) `targets.py` updated
        (keep-sorted) and `all_targets_linux_x64.txt` regenerated via
        `scripts/run_in_build_env.sh "python3 scripts/build/test.py"`.
-   [ ] Sim state is one enum with a documented transition diagram; timers
        cancelled before restart; unsigned math clamped; optionals null-safe;
        every list index bounds-checked; simulation reads state via cluster
        getters, not shadow caches.
-   [ ] OOB/pipe `Reset` restores all sim state.
-   [ ] `grep -c TEMPORARY_RETURN_IGNORED` over the diff = 0; no raw
        `(void)`-error swallows; no ember accessors outside CodegenIntegration.
-   [ ] Test-event-trigger handler endpoint-checked, registered/unregistered
        with the device lifecycle; `ci-pics-values` updated if triggers used.
-   [ ] Only server-side yaml suites enabled (`all-clusters` entry preserved
        when adding a `CI:` block); `_Simulated.yaml` suites untouched; unit
        tests added to `examples/BUILD.gn`.
-   [ ] Docs (`supported_device_types.md`, `supported_clusters.md`, `README.md`)
        updated including recomputed totals and table numbering.
-   [ ] No dead code / unused declarations left behind (devices have accrued "is
        this used?" comments before).
-   [ ] PR description states its series position and scope contract; any
        unrelated fix has been spun out to its own PR with rationale; this split
        PR builds alone.

## Reviewing These PRs Efficiently

Expect automated review bots to post many threads, several of them false
positives (bounds checks, doc-table methodology). Resolve threads immediately
after addressing with a short confirmation; for a deliberate change reply once
with the reasoning (methodology reference) rather than re-litigating — the
repo's established style is plain, short human replies.
