# Matter Device Types Catalog

This directory contains C++ representations of spec-defined **Matter Device
Types** (e.g., _OnOff Light_, _Dimmable Light_, _Laundry Washer_, _Aggregator_).

## Directory Structure & Inheritance

Every directory under `device/types/<foo>/` separates the generic Matter Device
Type definition from concrete simulation or hardware implementations:

```text
device/types/foo/
├── Foo.h / Foo.cpp                 # Generic base device class (mandatory spec clusters only)
├── OOBAccessors.h / .cpp           # Out-of-Band control & inspection accessors
├── NamedPipeTranslators.h / .cpp   # JSON CLI command translators
└── impl/
    └── LoggingFoo.h / .cpp         # Self-contained simulation subclass for DeviceFactory
```

### Strict Inheritance Direction

```text
impl/LoggingFoo (Simulation) -> Foo (Base Device Type) -> SingleEndpoint
```

1. **Base Device Type (`Foo.h`)**:
    - Inherits directly from `SingleEndpoint` (or a shared capability in special
      cases).
    - Defines the Matter `DeviceTypeEntry` (Device Type ID and revision).
    - Owns **only mandatory clusters** for that device type and accepts abstract
      `Delegate &` references in its constructor.
    - Contains zero simulation, logging, or hardware pin logic.
2. **Concrete Implementation (`impl/LoggingFoo.h` or `EmulatedFoo.h`)**:
    - **Inherits from `Foo` (`device_type`), never the reverse.**
    - Implements cluster delegates (inheriting privately from delegate bases
      _before_ `public Foo` for base-from-member initialization safety).
    - Instantiated by `DeviceFactory` for simulations; hardware targets subclass
      `Foo` with hardware drivers.

## Design Rules

1. **Spec Compliance & Naming**: Class and folder names correspond directly to
   Matter Device Library Specification names without the `Device` suffix (e.g.,
   `LaundryWasher`, not `LaundryWasherDevice`).
2. **Optional Clusters via Extension Hooks**: Do not place optional clusters in
   the base `Foo` class. Because `CodeDrivenDataModelProvider` requires all
   clusters on an endpoint to be added _before_ `provider.AddEndpoint()` is
   called, base classes expose virtual hooks inside `Register()` and
   `Unregister()`:
    - `RegisterAdditionalClusters(EndpointId, CodeDrivenDataModelProvider &)`
    - `UnregisterAdditionalClusters(CodeDrivenDataModelProvider &)` Subclasses
      in `impl/` override these hooks to attach optional clusters.
3. **Prefer Direct `SingleEndpoint` Inheritance**: Avoid creating new shared
   endpoint base classes in `device/capabilities/`. Prefer flat, self-contained
   device classes inheriting from `SingleEndpoint`, sharing delegate helper
   mixins in `impl/` when needed.
