# Shared Device Capability Baselines

This directory contains **Shared Device Capability Baselines** (also referred to
as _abstract_ or _synthetic_ device classes).

Unlike the folders in the root of `devices/` which map 1-to-1 to real,
spec-defined Matter Device Types, the classes in this folder are internal
architectural building blocks used to group and reuse shared cluster behaviors.

---

## Purpose and Design Principles

This directory balances code reuse with strict domain modeling when simulating
devices in the Matter SDK.

In the Matter Device Library, different **Device Types** often require identical
sets of clusters. For example:

-   Both **Dimmable Light** (`0x0101`) and **Dimmable Plug-In Unit** (`0x010A`)
    require `OnOff` and `LevelControl` clusters.
-   Both **On/Off Light** (`0x0100`) and **On/Off Plug-In Unit** (`0x010A`)
    require the `OnOff` cluster.
-   **Fan** (`0x002B`), **Air Purifier** (`0x002D`), and **Extractor Hood**
    (`0x005A`) all require the `FanControl` cluster.

To model these simulated devices cleanly, the codebase adheres to two core
design principles:

### 1. Spec-Aligned Domain Modeling (Semantic Purity)

Each folder in the root of `devices/` corresponds strictly to a real Matter
Device Type. We do not introduce inheritance hierarchies between different leaf
device types (for example, a plug-in unit does not inherit from a light, as they
are semantically distinct physical products in the specification).

### 2. Code Reuse (DRY)

To avoid duplicating complex cluster simulation, scenes management, and
transition logic across multiple leaf devices, we extract these shared cluster
behaviors into the **abstract capability classes** in this directory.

Concrete leaf devices (like `DimmablePlugInUnitDevice` or `DimmableLightDevice`)
inherit directly from their corresponding capability baseline (like
`DimmableLoadDevice`), achieving perfect code reuse while maintaining a clean,
spec-aligned object model.

---

## Architectural Patterns

### 1. Delegate Injection Pattern

To keep these capability classes generic and reusable, they are **fully
abstract** and do not hardcode any specific mock behaviors (such as logging or
automatic state changes).

Instead, they accept references or pointers to **Interaction Model Delegates**
(e.g., `OnOffDelegate`, `LevelControlDelegate`) in their constructors. The
application or a subclass (like a logging mock) can then inject the appropriate
delegate implementation at runtime.

### 2. Symmetrical Logging Mocks (the `impl/` subfolders)

Each capability has a corresponding logging implementation in its `impl/`
subfolder (e.g., `LoggingDimmableLoadDevice`). These subclass the capability and
automatically bind logging delegates to all clusters. This makes it trivial to
instantiate "logging-by-default" simulated devices in the simulator factory.

---

## Directory Catalog

-   **[`on-off-load/`](file:///usr/local/google/home/andreilitvin/devel/connectedhomeip/examples/all-devices-app/all-devices-common/devices/capabilities/on-off-load)**:
    Shared baseline for any device type controlling an On/Off load. Manages the
    `OnOff`, `Identify`, and `OnOffEffect` clusters.
-   **[`dimmable-load/`](file:///usr/local/google/home/andreilitvin/devel/connectedhomeip/examples/all-devices-app/all-devices-common/devices/capabilities/dimmable-load)**:
    Shared baseline for any device type controlling a Dimmable (Level Control)
    load. Manages `OnOff`, `LevelControl`, `Identify`, and `OnOffEffect`
    clusters.
-   **[`fan-load/`](file:///usr/local/google/home/andreilitvin/devel/connectedhomeip/examples/all-devices-app/all-devices-common/devices/capabilities/fan-load)**:
    Shared baseline for any device type controlling a Fan load. Manages
    `FanControl`, `OnOff` (optional), and `Identify` clusters.
