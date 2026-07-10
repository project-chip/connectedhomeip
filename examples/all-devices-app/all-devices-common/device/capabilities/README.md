# Shared Device Capabilities

This directory contains reusable, abstract baseline classes, mixins, and
delegates (referred to as **Capabilities**) that simulated Matter devices in the
`device/types/` catalog use to avoid duplicating cluster implementation and
hardware abstraction wiring.

## Overview

Unlike files in `device/types/`, which map directly to spec-defined Matter
Device Types (e.g. _Dimmable Light_ or _Speaker_), capabilities are **internal
abstractions** representing shared functionalities.

For example:

-   **`dimmable-load`** (`DimmableLoad`): Encapsulates the Identify, OnOff
    (lighting context), LevelControl (lighting transitions), ScenesManagement,
    and Groups cluster behaviors.
-   **`fan-load`** (`FanLoad`): Encapsulates shared fan control logic.
-   **`on-off-load`** (`OnOffLoad`): Encapsulates basic on/off cluster behavior.
-   **`operational-state`**: Implements reusable logging delegates for
    operational state and Rvc operational state clusters.

## Design Rules

1. **No Sibling Dependencies**: Capabilities must compile independently of
   concrete device types in `device/types/`. They may depend _only_ on
   `device/api/`.
2. **Abstract Baseline**: Capabilities are abstract classes. Concrete device
   types subclass them to construct spec-aligned Matter endpoints.
