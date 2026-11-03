# Shared Device Capabilities (Special Case)

This directory contains shared functional baselines (`OnOffLoad`,
`DimmableLoad`, `FanLoad`) and reusable delegate implementations
(`impl/Logging*Delegate.h`).

## Overview & Inheritance Chain

Unlike `device/types/`, which maps directly to spec-defined Matter Device Types
(e.g., _Dimmable Light_ or _Extractor Hood_), capabilities are internal
abstractions shared by a few closely related load families:

```text
impl/LoggingOnOffLight -> OnOffLight (Device Type) -> OnOffLoad (Capability) -> SingleEndpoint
```

-   **`on-off-load`** (`OnOffLoad`): Shared OnOff, Identify, Groups, and
    ScenesManagement wiring.
-   **`dimmable-load`** (`DimmableLoad`): Adds LevelControl transitions on top
    of OnOff/Identify/Scenes/Groups.
-   **`fan-load`** (`FanLoad`): Shared FanControl, Identify, Groups, and
    optional OnOff wiring.

## Architectural Guidelines (When NOT to Use Capabilities)

1. **Capabilities Are a Special Case, Not the Default**: New device types in
   `device/types/` should inherit **directly from `SingleEndpoint`**, not from a
   capability class. Deep inheritance chains couple unrelated device types,
   complicate constructor/context plumbing, and make cluster lifecycles harder
   to follow.
2. **Prefer Explicit Cluster Declarations over Endpoint Base Classes**:
   Duplicating a few `LazyRegisteredServerCluster` member declarations across
   device types is simpler and more maintainable than introducing a shared
   capability base class.
3. **Share Delegate Implementations, Not Endpoint Bases**: When sharing cluster
   behavior or logging stubs across multiple device types, place reusable
   delegate classes in `capabilities/<name>/impl/` (e.g.,
   `LoggingOnOffDelegate`, `LoggingDimmableDelegate`) and inherit/compose those
   delegates inside `device/types/<foo>/impl/LoggingFoo`.
4. **No Sibling Dependencies**: Files in `device/capabilities/` must compile
   independently of concrete device types in `device/types/`. They may depend on
   `device/api/` and shared Matter cluster or data-model APIs.
