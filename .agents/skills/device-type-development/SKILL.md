---
name: device-type-development
description: >
    Step-by-step guide for adding a new Matter device type to
    `examples/all-devices-app` (generic type class, simulated impl, build
    wiring, simulation plumbing, CI tests, docs tables). Encodes the layout and
    PR-splitting rules derived from resolved review threads on PR #73205 (RVC
    Service Area) and PR #73430 (Window Covering).
---

# Device Type Development

This guide covers adding a **device type** to `examples/all-devices-app`. It
complements `code-driven-cluster-development` (which covers clusters living in
`src/app/clusters`) — a device type PR usually does **not** touch
`src/app/clusters` unless a required cluster is still ember-based, in which case
that cluster must land first (see `code-driven-cluster-migration`).

Measured cost of doing it wrong: PR #73205 (RVC + Service Area) ran **42 days /
71 commits**, PR #73430 (Window Covering) **57 days / 58 commits** — most of it
mid-review restructuring after layout and scope questions the steps below
resolve up-front. A study of ~41 device-type PRs (Nov 2025 → Oct 2026; including
PRs #41607, #43768, #72733, #43408) shows fast merges (<2d) all had pre-agreed
scope + layout, while the slow ones (49/71/99d) were placement or ambiguity PRs,
not big ones (a 13-device, 172-file PR #72645 merged in 6 days).

---

## Layout Contract (do not violate — this drove multi-day review loops)

```
examples/all-devices-app/all-devices-common/device/
├── api/                    # DeviceInterface / SingleEndpoint base classes
├── capabilities/           # SHARED implementations reused by 2+ device types ONLY
│   └── on-off-load/{impl, ...}    # device-specific delegates must NOT go here
└── types/<device-type>/
    ├── BUILD.gn            # source sets: ":<device-type>", ":simulated", and optional ":posix"
    ├── <TypeName>.h        # GENERIC device type: spec-mandated bits only
    ├── <TypeName>.cpp
    ├── OOBAccessors.{h,cpp}          # optional: platform-neutral OOB accessor registration
    ├── NamedPipeTranslators.{h,cpp}  # optional: POSIX named-pipe translator registration (":posix")
    ├── impl/               # simulated example device; may import types/, never the reverse
    │   └── Simulated<TypeName>.{h,cpp}
    └── tests/              # unit tests for the generic type
```

> **Legacy Device Warning**: Older device types in `device/types/` that lack an
> `impl/` folder or still name their target `:logging` / `Logging<TypeName>`
> predate the current architecture. **Do not copy them.** Follow only newly
> structured exemplars (`window-covering`, `mode-select`,
> `robotic-vacuum-cleaner`, `thread-border-router`,
> `network-infrastructure-manager`, `air-quality-sensor`, `closure`,
> `closure-panel`).

Rules (andy31415 / soares-sergio, resolved across the studied PRs):

1.  **`types/<t>/<T>.h` = generic.** Must compile for any app. Hardcode nothing
    the spec does not mandate: features, optional attributes, and optional
    clusters are all injected (`Config.features` / `OptionalAttributeSet`),
    configured via variadic cluster-ID templates (see `AirQualitySensor.h`,
    #74417), or deferred to a `virtual RegisterOptionalClusters()` (or
    `RegisterAdditionalClusters()`) hook that only subclasses implement. The
    base `Register()` registers mandatory clusters, then calls the hook before
    `provider.AddEndpoint()`; `Unregister()` mirror-hooks accordingly after
    `UnregisterDescriptor()`.
2.  **`impl/` is a strict one-way dependency**: `impl/` may import `types/`,
    `types/` must never import or construct anything from `impl/`. No
    simulation/json/pipe logic in the generic class.
3.  **`capabilities/` is for genuinely shared, multi-device-type pieces.** When
    in doubt, create no capability at all — put the delegate in the device's own
    `impl/`. Fewer layers beats more inheritance.
4.  **Prefer one `Simulated<Device>` class** implementing/owning all delegates
    (opstate, modes, service area…) over several delegate classes that point to
    each other. Cross-cluster state belongs in one object with one state
    machine, not booleans scattered across delegates.
5.  **Dependencies are references, not nullable pointers.** Pack them in a
    `Context` / `Delegates` struct (see `WindowCovering.h`) and inject from
    `main`/factory setup. Mandatory peers (e.g. Run Mode for RVC) are
    references; constructor sequencing guarantees them — no runtime null checks.
6.  **Model device state as a single enum** (`Docked`, `DockedCharging`,
    `Running`, `Paused`, …) with the transition diagram in a doc comment. Two
    loosely-coupled booleans (`mDocked`/`mCharging`) caused a real bug class in
    #73205.
7.  **No globals inside device-type code.** Timer delegates, storage, etc. are
    injected.
8.  **New `impl/` source sets are named `simulated` (`Simulated<Device>`)**.
    Do not introduce new `:logging` source sets or `Logging<Device>` classes —
    `Simulated<Device>` combines logging and realistic timer/state-machine
    behavior in `impl/` (unless multiple distinct flavor presets exist, in which
    case the target carries the flavor name).

---

## Settled Idioms (each one was litigated; cite them proactively, don't wait to be told)

Every rule below came from accepted review findings across the studied PRs —
following them removes comment classes that cost days:

**Dependency & lifecycle**

-   `Context` / `Delegates` structs with **references** (see
    `WindowCovering.h`); "inject all the things" — concrete member objects,
    inherited/hardcoded delegates, and globals are all rejected (#74236,
    #74284). Shared app-level state (e.g. BreadCrumbTracker) lives in
    `DeviceFactory::Context`, not per-device duplicates (#74455).
-   No heap in single-endpoint device code: use `LazyRegisteredServerCluster<T>`
    instead of `unique_ptr` cluster members, and never use `std::function` timer
    callbacks (use `TimerDelegate` + member state) (#41607, #72119). (Composite
    multi-endpoint `impl/` classes like `SimulatedClosure` may hold
    `std::vector<std::unique_ptr<ChildDevice>>` for dynamic child parts.)
-   Wrap registration in `DeviceRegistrationTransaction` + `Commit()` for
    partial-install rollback (#73616, #72786); guard cluster teardown with
    `if (mCluster.IsConstructed())` and child-part unregistration via
    `GetEndpointId() != kInvalidEndpointId` (#74327); capture the endpoint ID
    before `UnregisterDescriptor()` if teardown needs it because
    `SingleEndpoint::UnregisterDescriptor()` resets `mEndpointId` to
    `kInvalidEndpointId` (#73205); `VerifyOrDie(IsConstructed())` on cluster
    accessors used before `Register()` (#74417); registration order matters when
    one cluster's `Startup()` touches another's storage (#74284 OnOff↔Thermostat
    race).
-   Inherited delegates:
    `Simulated<Device> : private <Cluster>Delegate, public <LeafDevice>` —
    delegate bases (which pass `*this`) must be listed before the device base so
    they are constructed first (#74278, #74327).
-   Attributes: `SetAttributeValue()` = set + notify atomically, skip no-op
    churn (#43408); for correlated clusters, sync each side explicitly on
    transitions (power-Off → LocalTemperature NULL + SystemMode off, #74284);
    spec-nullable values use `DataModel::Nullable`, not `std::optional`
    (#72119); never report `MatterReportingAttributeChangeCallback` for state
    you didn't actually change (RevertActiveDataset wiping a _committed_ dataset
    is a real reviewer catch, #74236).
-   Test-event-trigger handlers: register at device startup, unregister at
    shutdown — never link-time-resolved globals (#73616, #74379); handler must
    match endpoint ID (fall through otherwise) (#74327, #74379).
-   Errors: `LogErrorOnFailure` / propagate — **no new
    `TEMPORARY_RETURN_IGNORED`** (it's a legacy transition marker; grep your
    diff for it before submitting — flagged in 4+ PRs); no `(void)` casts to
    swallow `CHIP_ERROR`.

**Build & naming hygiene**

-   Alphabetical order everywhere (`DeviceFactory.h`, `enabled_devices.*`,
    `OOBAccessorHook.h`, `posix/named_pipe/Hook.h`, registration blocks) and run
    `keep-sorted`-affected blocks in `targets.py` — order violations recur
    (#73616, #72552, #72526).
-   `BUILD.gn` depends on what it uses directly (`lib/support`,
    `app/persistence`), never transitive includes (#74284); `device-factory`
    must reference the device's `:simulated` target it instantiates.
-   GN deps follow the consumer (#74286): when an optional cluster moves from
    the generic type into `impl/`, its cluster deps move out of the base source
    set into `:simulated` with it, and the generic header's includes and
    `Context` members referencing it go too (reviewers will ask "do we still
    need these here?").
-   No `Device` suffix on classes/filenames; spell device names out (no PICS
    acronyms) (#72918, #72111); constants `k`-prefixed with units, no `g_`
    prefixes; doc comments on public members only in `.h`, never duplicated in
    `.cpp` (#73616).
-   Compile-time configurability beats build flags: template-ID-keyed `Config`
    structs + variadic optional clusters + `static_assert` on unsupported IDs,
    keyed via `config.Get<CID>()` (#74417); never `namespace Detail` or helpers
    exposed through headers that `DeviceFactory.h` transitively includes
    (`namespace ...Internal`, #74417). When a cluster's C++ type encodes its
    feature map in template parameters (e.g., `ThermostatCluster`), name a
    fixed-feature device class `Full<Device>` (`FullThermostat`, #74286) so the
    plain `<Device>` name stays reserved for a generic class.

**Simulation quality**

-   No-op delegates are rejected ("too fake"): ship real conversions/mocks with
    a chip-tool verification recipe (#73825 PlanckianColorConverter). Timers
    must self-reschedule and transitions must be timer-driven so `Stop` is
    meaningful; clamp unsigned arithmetic; treat generated optionals as null
    (`ValueOr` + "not moving" path).
-   A Simulated impl that returns Unsupported from most delegate callbacks fails
    its purpose (#74286): every attribute/feature your config _enables_ must
    have a real delegate surface — optional setpoint-limit getters returning
    `UnsupportedAttribute` broke `LoadSetpoints`, `SetpointRaiseLower`, and the
    TC tests in that PR.
-   Filter valid-but-unsupported enum values with the cluster-consistent error —
    `ConstraintError`, not `InvalidValue` — and don't let a config advertize a
    feature the filter rejects (#74286: Context advertizes `kAutoMode` while
    `SetSystemMode(Auto)` returns ConstraintError, and the persisted value
    resets to Off on startup).
-   **Config defaults must match `ci-pics-values`** (#74286): the yaml tests run
    against the shared CI PICS, so a device whose `Config{}` omits
    features/attributes the PICS enable fails its own CI (TSTAT `AutoMode`/
    `MinSetpointDeadBand`, TSUIC `ScheduleProgrammingVisibility`). Enable what
    the PICS promise, or adjust guards.
-   No srand-seeded "randomness" to mask CI flakes — reviewers call it out and
    it resurfaces (#73616).
-   Device factory presets (`SimulatedClosure::ThreePanelCabinetClosureConfig()`)
    belong in the device's `impl/`, not `DeviceFactory.h`; child-only device
    types (`closure-panel`) are NOT independently enableable in
    `enabled_devices.cmake` (#74327).

---

## Step-by-Step Procedure

### Step 0 — Discover the app's own documentation first

Per `AGENTS.md` guidance, read before writing:

-   `examples/all-devices-app/docs/supported_device_types.md` — device type
    table + discovery methodology (which clusters are `[O]`/optional).
-   `examples/all-devices-app/docs/supported_clusters.md` — which clusters are
    code-driven in the SDK vs ember.
-   `examples/all-devices-app/docs/architecture.md`,
    `examples/all-devices-app/docs/adding_new_device.md`, and
    `examples/all-devices-app/docs/design/out_of_band_control.md`.

Record from your device type: its spec ID, every **server** cluster with
conformance M vs O, per-cluster feature maps and optional attributes, and the
split `Ready` / `Missing` against `supported_clusters.md`. If mandatory clusters
are missing, the type is **Blocked** — the blocker is a cluster PR, not a
device-type PR.

### Step 1 — Plan the PR series (this is where the two PRs lost weeks)

Split, in this order, each merging independently and compiling cleanly:

-   **PR 1 — base device type + minimal simulated impl:** generic
    `types/<t>/<T>.{h,cpp}` with mandatory clusters only, a minimal compilable
    `impl/Simulated<T>.{h,cpp}` satisfying mandatory cluster `Delegate &`
    references (since `DeviceFactory::Context` only carries `identifyDelegate`
    and `timerDelegate`), `BUILD.gn`, `DeviceFactory` + build wiring, unit tests
    for `<T>`, and docs rows. No OOB/named pipes or complex multi-cluster state
    machines yet.
-   **PR 2 — realistic mandatory simulation:** full timer-driven / physically
    plausible mandatory cluster behavior in `impl/Simulated<T>` (if non-trivial
    enough to separate from PR 1).
-   **PR 3 — optionality:** optional clusters enabled through
    `RegisterOptionalClusters()` in `impl/Simulated<T>` (or template parameters
    like `AirQualitySensor`), with optional attributes/feature bits turned on.
-   **PR 4 — cross-cluster wiring ("if-this-then-that")** between clusters
    (modes drive opstate, opstate drives progress reporting) and any state
    machines that emerged.
-   **PR 5 — OOB / named pipes + CI:** `OOBAccessors`, `NamedPipeTranslators`,
    yaml TC suites enabled, and/or `python_testing` `TC_*.py` files extended.

Rationale: in #73205 the Service Area implementation, delegates, pipe simulation
and 15 test scripts all arrived together; reviewers had to untangle
generic-vs-specific, layout, and test-scope questions inside one 43-file diff,
serializing every fix commit behind an unmerged design debate. A minimal base +
impl lands in days; later PRs reuse its settled vocabulary.

**Names come first:** if you are unsure about any file/class naming, raise that
naming as the _first_ PR (or at least the first comment) — class/location names
were the last thing to settle in both PRs and forced broad renames.

### Step 2 — Write the generic device type

Copy the shape of the newest clean exemplar,
`all-devices-common/device/types/window-covering/WindowCovering.{h,cpp}`:

```cpp
class WindowCovering : public SingleEndpoint
{
public:
    struct Context { TimerDelegate & timerDelegate; };
    struct Delegates { Clusters::WindowCovering::WindowCoveringDelegate & windowCovering;
                       Clusters::IdentifyDelegate & identify; };
    struct Config { BitFlags<Clusters::WindowCovering::Feature> features;
                    Clusters::WindowCovering::OptionalAttributeSet optionalAttributes = {};
                    Clusters::WindowCovering::Type type = Clusters::WindowCovering::Type::kUnknown;
                    Clusters::WindowCovering::EndProductType endProductType = Clusters::WindowCovering::EndProductType::kUnknown; };

    WindowCovering(const Context & context, const Delegates & delegates, const Config & config);
    ~WindowCovering() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public getters returning live cluster references for programmatic/OOB control
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::WindowCovering::WindowCoveringCluster & WindowCoveringCluster() { return mWindowCoveringCluster.Cluster(); }

protected:
    virtual CHIP_ERROR RegisterOptionalClusters(chip::EndpointId endpoint,
                                                CodeDrivenDataModelProvider & provider)
    { return CHIP_NO_ERROR; }
    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) {}

    const Context mContext;

private:
    const Delegates mDelegates;
    const Config mConfig;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::WindowCovering::WindowCoveringCluster> mWindowCoveringCluster;
};
```

Key lifecycle invariants in `Register()` and `Unregister()`:

-   **`Register(endpoint, provider, composition)`**:
    1.  `VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);`
    2.  `DeviceRegistrationTransaction transaction(*this, provider);`
    3.  `ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));` **first**.
    4.  `.Create(...)` and `ReturnErrorOnFailure(provider.AddCluster(...));` for each mandatory cluster.
    5.  `ReturnErrorOnFailure(RegisterOptionalClusters(endpoint, provider));`
    6.  `ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));` **last** — `CodeDrivenDataModelProvider` rejects `AddCluster` after `AddEndpoint` once started (`CHIP_ERROR_INCORRECT_STATE`).
    7.  `transaction.Commit();`
-   **`Unregister(provider)`**:
    1.  Call `UnregisterDescriptor(provider);` **first** — `CodeDrivenDataModelProvider` rejects `provider.RemoveCluster()` while the endpoint is still registered (`CHIP_ERROR_INCORRECT_STATE`).
    2.  **`mEndpointId` reset trap:** `SingleEndpoint::UnregisterDescriptor(provider)` resets `mEndpointId = kInvalidEndpointId`. Do not check `mEndpointId` inside `UnregisterOptionalClusters()`; instead guard each cluster with `if (mCluster.IsConstructed())`.
    3.  Call `UnregisterOptionalClusters(provider);`, then `LogErrorOnFailure(provider.RemoveCluster(...))` and `.Destroy()` on each constructed mandatory cluster.
    4.  *(Composite devices inheriting `DeviceInterface`, e.g. `closure/Closure.cpp`)*: Unregister child part endpoints (`UnregisterParts`) **before** calling `UnregisterDescriptor(provider)` on the parent endpoint.
-   Mandatory-cluster defaults only (do **not** initialize optional attributes
    like current phase/mode from the base class — the spec does not require it
    and a wrong default logs failures at startup; subclasses init what they
    simulate).
-   Do not add JSON/pipe/simulation headers here (flagged directly by the
    reviewer in #73205).

### Step 3 — Write the simulated impl

`impl/Simulated<Device>.h/.cpp`, subclassing the generic type and overriding
`RegisterOptionalClusters()` / `UnregisterOptionalClusters()` for optional bits
(see `window-covering/impl/SimulatedWindowCovering.{h,cpp}` and
`robotic-vacuum-cleaner/impl/SimulatedRoboticVacuumCleaner.{h,cpp}`):

-   One state enum + transition diagram (doc comment) instead of parallel
    booleans.
-   Make the simulation _physically plausible with real timers_ — andy31415 on
    #73430: a covering that instantly jumps from lift 0 to 255 makes `Stop`
    meaningless. E.g. a tick timer advancing state by a step every N ms; a
    mid-flight `Stop` freezes the current value. (A `TimerDelegate` must be
    `CancelTimer`d before every restart; re-arm defensively.)
-   Cross-cluster reactions ("if-this-then-that") live here as explicit methods,
    e.g. `HandlePauseStateCallback` stores prior state **after** the base call
    succeeds (storing before created a stale-resume bug in #73205); entering
    `Running` clears docked/charging (didn't → stale "Docked" report).
-   Arithmetic on attribute-like values: clamp unsigned math
    (`x -= std::min(x, step)`), treat generated accessor optionals as possibly
    null (`ValueOr` + a "not moving" path) — never assume non-null in sim code.
-   Bounds-check every list index against its container size before access
    (three separate gemini findings in #73205 were the same missing check).
-   Transport/JSON parsing is _not_ here; external control goes through
    `OOBAccessors` and `posix/named_pipe/translators/` (see Step 5).

### Step 4 — Build wiring

For each device type touch **all** of (missing any one is a review item; keep
all lists alphabetically sorted):

-   `all-devices-common/device/types/<t>/BUILD.gn` — define `source_set("<t>")`
    for `<T>.{h,cpp}` (plus `OOBAccessors.{h,cpp}` if present),
    `source_set("simulated")` for `impl/Simulated<T>.{h,cpp}` with
    `public_deps = [ ":<t>", ... ]`, and optional `source_set("posix")` for
    `NamedPipeTranslators.{h,cpp}` (see `mode-select/BUILD.gn` and
    `window-covering/BUILD.gn`).
-   `all-devices-common/device-factory/enabled_devices_config.h.in` — add
    `#cmakedefine01 ALL_DEVICES_ENABLE_<UPPER_SNAKE>` (mandatory for CMake
    platforms).
-   `all-devices-common/device-factory/enabled_devices.gni` — add
    `[ "<kebab-name>", "<UPPER_SNAKE>" ]` to `_available_devices`.
-   `all-devices-common/device-factory/enabled_devices.cmake` — update **both**
    `ALL_DEVICES_DEVICE_SOURCES` (both `<T>.cpp` and `impl/Simulated<T>.cpp`)
    and `foreach(_key ...)` with `<kebab-name>:<UPPER_SNAKE>`.
-   `all-devices-common/device-factory/DeviceFactory.h` — include
    `<device/types/<t>/impl/Simulated<T>.h>` and register inside
    `if constexpr (ALL_DEVICES_ENABLE_<UPPER_SNAKE>)`:
    `RegisterCreator("<kebab-name>", [this]() { ... })`.
-   `all-devices-common/device-factory/BUILD.gn` — add
    `"${chip_root}/examples/all-devices-app/all-devices-common/device/types/<t>:simulated"`
    to `public_deps` (an unreferenced `:simulated` target silently drops from
    the GN graph).
-   `examples/all-devices-app/posix/BUILD.gn` — add
    `device/types/<t>` (and `device/types/<t>:posix` if `NamedPipeTranslators`
    exist) to `public_deps`.
-   `examples/all-devices-app/silabs/BUILD.gn` — add the `if (enable_<snake>_device)`
    dependency block.
-   `examples/all-devices-app/zephyr/Kconfig.devices` — add a
    `CONFIG_ALL_DEVICES_DEVICE_<UPPER_SNAKE>` symbol with
    `imply CHIP_CODE_DRIVEN_<CLUSTER>_SERVER` for each required code-driven
    cluster, or Zephyr builds can never select the new device (#74286).
-   If you add build **targets** (`scripts/build/build/targets.py`, inside
    `_ALL_DEVICES_APP_DEVICES` with `# keep-sorted`), regenerate
    `scripts/build/testdata/all_targets_linux_x64.txt` using
    `scripts/run_in_build_env.sh "python3 scripts/build/test.py"` — hand-edits
    reintroduced darwin rows into a linux list in #73430 (a bot Major finding).

### Step 5 — Simulation plumbing (Out-of-Band Accessors & POSIX Named Pipes)

Follow `examples/all-devices-app/docs/design/out_of_band_control.md` and the
`mode-select` / `robotic-vacuum-cleaner` exemplars:

-   **Cluster OOB Accessor (`all-devices-common/oob-accessors/clusters/`)**:
    -   Implement `<Cluster>OOBAccessor.{h,cpp}` inheriting from
        `OOBAccessorInterface` (`Execute(action, payload)`). For standard
        attribute updates, use action `"SetAttribute"` and deserialize TLV via
        `OOBDataSerializer`.
    -   Add a per-cluster `source_set` in
        `all-devices-common/oob-accessors/clusters/BUILD.gn` (#74452) and add
        the `.cpp` to
        `all-devices-common/oob-accessors/all_devices_config.cmake`.
-   **Device OOB Registration (`device/types/<t>/OOBAccessors.{h,cpp}`)**:
    -   Implement `void RegisterOOBAccessors(<Device> & device, OOBAccessorRegistry & registry)`.
        (If simulation hooks exist only on `Simulated<Device>`, provide a no-op
        overload for `<Device> &` and the real registration overload for
        `Simulated<Device> &`; see `robotic-vacuum-cleaner/OOBAccessors.cpp`.)
    -   Include `<device/types/<t>/OOBAccessors.h>` in
        `all-devices-common/oob-accessors/OOBAccessorHook.h` (`OOBAccessorHook`
        automatically ties accessor lifetime to the device via
        `WithDependency(&device)` and unregisters on teardown, #74648).
    -   Add `device/types/<t>/OOBAccessors.cpp` to
        `all-devices-common/oob-accessors/all_devices_config.cmake`.
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
    cluster's yaml suites in `src/app/tests/suites/certification/Test_TC_*.yaml`:
    -   **Trap when adding a `CI:` block (#73430):** YAML tests without a `CI:`
        block run against `all-clusters` by default. Adding a `CI:` block
        overrides that default, so you **must** include both the existing
        `app: all-clusters` entry and the new `app: all-devices` (with
        `app-args: "--device <kebab-name>:1"`) entry.
    -   Only enable **server-side** suites: `*_Simulated.yaml` are client-PICS
        tests for simulated _client_ behavior — adding a server app there was
        explicitly rejected in #73430.
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
    -   For an **newly implemented optional cluster**: remove it from the
        `Missing Clusters` column (the methodology is defined in the file
        header — follow it even when bots disagree; #73205 was "flagged" the
        opposite way by two different machines).
-   `examples/all-devices-app/docs/supported_clusters.md`: flip the cluster's
    `Used in All-Devices` column to `Yes`, add the device to `Notes`, and
    recompute the `**Total**` summary row at the bottom — stale totals were a
    separate review comment.
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
-   [ ] `types/` imports nothing from `impl/`; `impl/` is a separate
        `source_set("simulated")` using `Simulated<Device>`; anything in
        `capabilities/` is used by ≥2 device types.
-   [ ] Registration wired with `DeviceRegistrationTransaction` + `Commit()`:
        `RegisterDescriptor` first, mandatory + optional `AddCluster` next,
        `provider.AddEndpoint` last.
-   [ ] `Unregister` calls `UnregisterDescriptor(provider)` **first** before
        `UnregisterOptionalClusters` and `provider.RemoveCluster`; clusters
        guarded with `IsConstructed()`; delegate bases ordered before device
        base.
-   [ ] All parallel build/config files touched (alphabetical):
        `device/types/<t>/BUILD.gn`, `enabled_devices_config.h.in`,
        `enabled_devices.gni`, `enabled_devices.cmake` (both sources and
        `foreach`), `DeviceFactory.h`, `device-factory/BUILD.gn` (`:simulated`),
        `posix/BUILD.gn`, `silabs/BUILD.gn`, `zephyr/Kconfig.devices`.
-   [ ] If OOB / NamedPipe support is added: `<Cluster>OOBAccessor` in
        `oob-accessors/clusters/BUILD.gn` + `oob-accessors/all_devices_config.cmake`,
        `device/types/<t>/OOBAccessors.{h,cpp}` in `OOBAccessorHook.h` +
        `oob-accessors/all_devices_config.cmake`, and
        `device/types/<t>/NamedPipeTranslators.{h,cpp}` (`:posix`) in
        `posix/named_pipe/Hook.h` + `posix/BUILD.gn`.
-   [ ] (Conditional — only if adding build targets) `targets.py` updated
        (keep-sorted) and `all_targets_linux_x64.txt` regenerated via
        `scripts/run_in_build_env.sh "python3 scripts/build/test.py"`.
-   [ ] Sim state is one enum with a documented transition diagram; timers
        cancelled before restart; unsigned math clamped; optionals null-safe;
        every list index bounds-checked.
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
-   [ ] No dead code / unused declarations left behind (both PRs accrued "is
        this used?" comments).
-   [ ] PR description states its series position and scope contract; any
        unrelated fix has been spun out to its own PR with rationale.

## Reviewing These PRs Efficiently

Expect the gemini / copilot / coderabbit bots to post many threads, several of
them false positives (bounds checks, doc-table methodology). Resolve threads
immediately after addressing with a short confirmation; for a deliberate change
reply once with the reasoning (methodology reference) rather than re-litigating
— see the repo's established style: plain, short human replies.
