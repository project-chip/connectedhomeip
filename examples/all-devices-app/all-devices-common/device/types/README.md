# Matter Simulated Device Types Catalog

This directory contains concrete C++ implementations of spec-defined **Matter
Device Types** (e.g. _OnOff Light_, _Dimmable Light_, _Robotic Vacuum Cleaner_,
_Aggregator_).

## Overview

Every directory under `device/types/` represents a single Matter Device Type.
These classes are typically **leaf types** that the application instantiation
layer (e.g., `DeviceFactory`) constructs.

-   Subclasses derive from foundational abstractions in `device/api/` (such as
    `SingleEndpoint` or `DeviceInterface`).
-   They inherit or compose reusable baselines from `device/capabilities/` to
    reduce duplication.

## Design Rules

1. **Spec Compliance**: Class names and folder names must correspond directly to
   standard Matter Device Types defined in the Matter Device Library
   Specification.
2. **No Name Stuttering**: Do not include the `Device` suffix in class or file
   names (e.g., use `Aggregator.h/cpp` and class `Aggregator` instead of
   `Aggregator`).
3. **Clean Composition**: Implement command handlers and attribute accessors by
   delegating logic to the underlying capabilities or platform overrides.
