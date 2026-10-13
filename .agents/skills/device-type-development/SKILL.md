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
    ├── BUILD.gn            # two source sets: ":<device-type>" and ":simulated" (or ":logging" for pre-existing ones)
    ├── <TypeName>.h        # GENERIC device type: spec-mandated bits only
    ├── <TypeName>.cpp
    ├── impl/               # simulated example device; may import types/, never the reverse
    │   └── Simulated<TypeName>.{h,cpp}
    └── tests/              # optional unit tests for the generic type
```

Rules (andy31415 / soares-sergio, resolved across the studied PRs):

1.  **`types/<t>/<T>.h` = generic.** Must compile for any app. Hardcode nothing
    the spec does not mandate: features, optional attributes, and optional
    clusters are all injected (`Config.features` / `OptionalAttributeSet`) or
    deferred to a `virtual RegisterOptionalClusters()` hook that only subclasses
    implement. The base `Register()` registers mandatory clusters, then calls
    the hook; `Unregister()` mirror-hooks accordingly.
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
8.  **New `impl/` source sets are named `simulated`**, not `logging`
    (`air-purifier` still has `:logging`; keep old names, use `:simulated` for
    new types). `Simulated<Device>` combines the logging impl AND realistic
    behavior (see Simulation section).

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
-   No heap in device code: no `unique_ptr` members, no `std::function` timer
    callbacks (use delegate + backing storage) (#41607, #72119).
-   Wrap registration in `DeviceRegistrationTransaction` + `Commit()` for
    partial-install rollback (#73616, #72786); guard double-unregister via
    `GetEndpointId() != kInvalidEndpointId` (#74327); capture the endpoint
    before base teardown resets it (#73205); `VerifyOrDie(IsConstructed())` on
    cluster accessors used before `Register()` (#74417); registration order
    matters when one cluster's `Startup()` touches another's storage (#74284
    OnOff↔Thermostat race).
-   Inherited delegates:
    `Logging<Device> : private LoggingDelegate, public <LeafDevice>` — delegate
    bases (which pass `*this`) before the device base (#74278, #74327).
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
    registration blocks) and run `keep-sorted`-affected blocks in `targets.py` —
    order violations recur (#73616, #72552, #72526).
-   `BUILD.gn` depends on what it uses directly (`lib/support`,
    `app/persistence`), never transitive includes (#74284); posix side must
    reference the device's `:logging`/`:simulated` target it instantiates.
-   GN deps follow the consumer (#74286): when an optional cluster moves from
    the generic type into `impl/`, its cluster deps move out of the base
    source set into `:simulated`/`:logging` with it, and the generic header's
    includes and `Context` members referencing it go too (reviewers will ask
    "do we still need these here?").
-   No `Device` suffix on classes/filenames; spell device names out (no PICS
    acronyms) (#72918, #72111); constants `k`-prefixed with units, no `g_`
    prefixes; doc comments on public members only in `.h`, never duplicated in
    `.cpp` (#73616).
-   Compile-time configurability beats build flags: template-ID-keyed `Config`
    structs + variadic optional clusters + `static_assert` on unsupported IDs,
    keyed via `config.Get<CID>()` (#74417); never `namespace Detail` or helpers
    exposed through headers that `DeviceFactory.h` transitively includes
    (`namespace ...Internal`, #74417).

**Simulation quality**

-   No-op delegates are rejected ("too fake"): ship real conversions/mocks with
    a chip-tool verification recipe (#73825 PlanckianColorConverter). Timers
    must self-reschedule and transitions must be timer-driven so `Stop` is
    meaningful; clamp unsigned arithmetic; treat generated optionals as null
    (`ValueOr` + "not moving" path).
-   A Logging/Simulated impl that returns Unsupported from most delegate
    callbacks fails its purpose (#74286): every attribute/feature your config
    *enables* must have a real delegate surface — optional setpoint-limit
    getters returning `UnsupportedAttribute` broke `LoadSetpoints`,
    `SetpointRaiseLower`, and the TC tests in that PR.
-   Filter valid-but-unsupported enum values with the cluster-consistent
    error — `ConstraintError`, not `InvalidValue` — and don't let a config
    advertize a feature the filter rejects (#74286: Context advertizes
    `kAutoMode` while `SetSystemMode(Auto)` returns ConstraintError, and the
    persisted value resets to Off on startup).
-   **Config defaults must match `ci-pics-values`** (#74286): the yaml tests
    run against the shared CI PICS, so a device whose `Config{}` omits
    features/attributes the PICS enable fails its own CI (TSTAT `AutoMode`/
    `MinSetpointDeadBand`, TSUIC `ScheduleProgrammingVisibility`). Enable what
    the PICS promise, or adjust guards.
-   No srand-seeded "randomness" to mask CI flakes — reviewers call it out and
    it resurfaces (#73616).
-   Device factory presets (`ThreePanelDoorClosureConfig()`) belong in the
    device's `impl/`, not `DeviceFactory.h`; child-only device types (Closure
    Panel) are NOT independently enableable in `enabled_devices.cmake` (#74327).

---

## Step-by-Step Procedure

### Step 0 — Discover the app's own documentation first

Per the CLAUDE.md guidance, read before writing:

-   `examples/all-devices-app/docs/supported_device_types.md` — device type
    table + discovery methodology (which clusters are `[O]`/optional).
-   `examples/all-devices-app/docs/supported_clusters.md` — which clusters are
    code-driven in the SDK vs ember.
-   `examples/all-devices-app/ARCHITECTURE.md` / `docs/` index.

Record from your device type: its spec ID, every **server** cluster with
conformance M vs O, per-cluster feature maps and optional attributes, and the
split `Ready` / `Missing` against `supported_clusters.md`. If mandatory clusters
are missing, the type is **Blocked** — the blocker is a cluster PR, not a
device-type PR.

### Step 1 — Plan the PR series (this is where the two PRs lost weeks)

Split, in this order, each merging independently:

-   **PR 1 — skeleton (no optionality):** generic `types/<t>/<T>.h/.cpp` with
    mandatory clusters only, BUILD.gn, device-factory registration with a
    minimal creator, docs rows. No impl/, no pipes, no tests yet.
-   **PR 2 — simulated impl:** `impl/Simulated<Device>` implementing mandatory
    cluster behavior realistically.
-   **PR 3 — optionality:** optional clusters enabled through
    `RegisterOptionalClusters()` in the simulated subclass, optional
    attributes/feature bits turned on.
-   **PR 4 — cross-cluster wiring ("if-this-then-that")** between clusters
    (modes drive opstate, opstate drives progress reporting) and any state
    machines that emerged.
-   **PR 5 — CI:** yaml TC suites enabled and/or `python_testing` TC\_\* files
    extended, named-pipe handlers as needed.

Rationale: in #73205 the Service Area implementation, delegates, pipe simulation
and 15 test scripts all arrived together; reviewers had to untangle
generic-vs-specific, layout, and test-scope questions inside one 43-file diff,
serializing every fix commit behind an unmerged design debate. A skeleton lands
in days; later PRs reuse its settled vocabulary.

**Names come first:** if you are unsure about any file/class naming, raise that
naming as the _first_ PR (or at least the first comment) — class/location names
were the last thing to settle in both PRs and forced broad renames.

### Step 2 — Write the generic device type

Copy the shape of the newest clean exemplar,
`all-devices-common/device/types/window-covering/WindowCovering.h`:

```cpp
class WindowCovering : public SingleEndpoint
{
public:
    struct Context { TimerDelegate & timerDelegate; };
    struct Delegates { Clusters::WindowCovering::WindowCoveringDelegate & windowCovering;
                       Clusters::IdentifyDelegate & identify; };
    struct Config { BitFlags<Clusters::WindowCovering::Feature> features;
                    Clusters::WindowCovering::OptionalAttributeSet optionalAttributes = {};
                    Clusters::WindowCovering::Type type = Clusters::WindowCovering::Type::kUnknown; };

    WindowCovering(const Context & context, const Delegates & delegates, const Config & config);

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // getters returning live cluster references for programmatic control
    Clusters::WindowCovering::WindowCoveringCluster & WindowCoveringCluster() { ... }

protected:
    virtual CHIP_ERROR RegisterOptionalClusters(chip::EndpointId endpoint,
                                                CodeDrivenDataModelProvider & provider)
    { return CHIP_NO_ERROR; }
    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) {}

    const Context mContext;
    ...
    LazyRegisteredServerCluster<Clusters::WindowCovering::WindowCoveringCluster> mWindowCoveringCluster;
};
```

Key points:

-   Register mandatory clusters, call `RegisterOptionalClusters(...)`, then
    finish. **`Unregister` ordering trap:** the framework resets `mEndpointId`
    when base unregister runs; if subclass cleanup needs the endpoint (e.g. pipe
    handler unregistration), capture it _before_ invoking base teardown, and
    subclass `UnregisterOptionalClusters` does its work via captured state.
-   Mandatory-cluster defaults only (e.g. do **not** initialize optional
    attributes like current phase/mode from the base class — the spec does not
    require it and a wrong default logs failures at startup; subclasses init
    what they simulate).
-   Do not add JSON/pipe/simulation headers here (flagged directly by the
    reviewer in #73205).

### Step 3 — Write the simulated impl

`impl/Simulated<Device>.h/.cpp`, subclassing the generic type and
`RegisterOptionalClusters()` for the optional bits:

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
-   Everything needing app plumbing (named pipes, persistence) is _not_ here; it
    goes in `posix/` via the command delegate.

### Step 4 — Build wiring

For each device type touch **all** of (missing any one is a review item):

-   `all-devices-common/device/types/<t>/BUILD.gn` — two source sets as in the
    layout block above.
-   `all-devices-common/device-factory/enabled_devices.{cmake,gni}` — device
    sources list; check whether the cmake side also needs
    `enabled_devices_config.h.in` include-dir entries.
-   `all-devices-common/device-factory/DeviceFactory.h` — a
    `RegisterCreator("<kebab-name>", [this]() { ... })`.
-   `all-devices-common/device-factory/BUILD.gn` — the factory target must
    publicly depend on the concrete impl target it instantiates (e.g.
    `mode-select:simulated`); an unreferenced impl source set silently drops
    from the GN graph. If several simulated flavors of one device exist, the
    impl source set carries the flavor name, not plain `simulated`
    (`temperature-sensor:increasing`).
-   `examples/all-devices-app/zephyr/Kconfig.devices` — a
    `CONFIG_ALL_DEVICES_DEVICE_<NAME>` symbol, or Zephyr builds can never
    select the new device (#74286).
-   If you add build **targets** (`scripts/build/build/targets.py`), regenerate
    `scripts/build/testdata/all_targets_linux_x64.txt` with the tooling on the
    right platform — hand-edits reintroduced darwin rows into a linux list in
    #73430 (a bot Major finding).
-   Platform CMakeLists that aggregate shared include dirs (esp32, telink) when
    impl headers are shared.

### Step 5 — Simulation plumbing (posix named pipes)

-   Command handlers go **in `AppCommandDelegate.cpp`** for now ("when in Rome"
    — andy31415 explicitly deferred a refactor); do not invent new handler
    classes per device.
-   Register each cluster implementation in
    `posix/include/AllDevicesAppClusterImplementationRegistry.h` /
    `posix/ClusterTypeMappings.cpp`. Two `ModeBase` clusters on one endpoint
    (Run vs Clean) need disambiguation — prefer an extra template
    `<typename Mode>` tag over looking up by cluster id.
-   Give the simulated device a `Reset`-like pipe entrypoint that restores _all_
    sim state (timers, dock state, pre-pause state) — CI tests replay state
    repeatedly and a missing reset causes cross-test flakiness.

### Step 6 — Tests and CI

-   Enable the device type on `all-devices-app` targets in CI by adding the
    cluster's yaml suites. Only **server-side** suites: `*_Simulated.yaml` are
    client-PICS tests for simulated _client_ behavior — adding a server app
    there was explicitly rejected in #73430.
-   For `python_testing/TC_*.py` scripts, add the `all-devices-app` run config
    and include `EndpointId` in named-pipe commands (endpoint-scoped resets).
-   Unit tests for the generic type go in `types/<t>/tests/` mirroring
    `air-quality-sensor/tests/`.

### Step 7 — Docs tables

Update both tables in the same PR as their functional change:

-   `docs/supported_device_types.md`: an implemented optional cluster leaves the
    `Missing Clusters` column (the methodology is defined in the file header —
    follow it even when bots disagree; #73205 was "flagged" the opposite way by
    two different machines).
-   `docs/supported_clusters.md`: flip the cluster's row and recompute the
    summary counts (`Used in All-Devices`, total supported) — stale totals were
    a separate review comment.
-   Keep `supported_device_types.md` status notes consistent with
    `supported_clusters.md` in the same commit; reviewers notice drift between
    them.

---

## Pre-Review Self-Checklist

-   [ ] Base device type contains only spec-mandated state; features, optional
        attributes, and optional clusters all behind `Config` or
        `RegisterOptionalClusters()`.
-   [ ] `types/` imports nothing from `impl/`; `impl/` is a separate gn source
        set; anything in `capabilities/` is used by ≥2 device types.
-   [ ] `DeviceFactory` creator + `enabled_devices.{cmake,gni}` + app CMakeLists
        all updated.
-   [ ] Testdata (`all_targets_linux_x64.txt`) regenerated by tooling _if build
        targets changed_.
-   [ ] Sim state is one enum with a documented transition diagram; timers
        cancelled before restart; unsigned math clamped; optionals null-safe.
-   [ ] Every list index bounds-checked; every `Unregister` path cleans up
        subclass registrations _before_ base teardown loses the endpoint.
-   [ ] Pipe `Reset` restores all sim state.
-   [ ] `grep -c TEMPORARY_RETURN_IGNORED` over the diff = 0; no raw
        `(void)`-error swallows; no ember accessors outside CodegenIntegration.
-   [ ] Registration wired with `DeviceRegistrationTransaction` + `Commit()`,
        rollback-safe; no double-unregister; accessors guarded with
        `IsConstructed()`; delegate bases ordered before device base.
-   [ ] Test-event-trigger handler endpoint-checked, registered/unregistered
        with the device lifecycle; ci-pics-values updated if triggers used.
-   [ ] All parallel-config files touched: `enabled_devices.{cmake,gni}`,
        `enabled_devices_config.h.in`, `DeviceFactory.h` (alphabetical),
        `device-factory/BUILD.gn` (public dep on the impl target),
        `zephyr/Kconfig.devices`, `README.md` (+CLI list/`--help`), both docs
        tables with recomputed totals.
-   [ ] (Conditional — only if adding build targets) `targets.py` updated
        (keep-sorted) and its golden snapshot regenerated via tooling.
-   [ ] Only server-side yaml suites enabled; `_Simulated` suites untouched.
-   [ ] Docs tables (`supported_device_types.md`, `supported_clusters.md`)
        updated including computed totals.
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
