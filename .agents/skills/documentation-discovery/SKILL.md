---
name: documentation-discovery
description:
    Essential guidelines instructing AI agents on how to incrementally discover,
    read, and consult top-level monorepo documentation and example-specific
    reference guides before altering code or executing test harnesses.
---

# Matter SDK Incremental Documentation Discovery Skill

## Operational Philosophy

The Matter SDK is an extensive, multi-layered repository. AI assistants and
contributors must avoid making implicit assumptions or guessing architectural
patterns. Instead, adopt an **Incremental Documentation Discovery** approach for
every task.

## Discovery Workflow

### Level 1: Monorepo Root Documentation

When investigating high-level SDK workflows, core specification rules, or
repository-wide testing schemas, always start by consulting the top-level
documentation sheets located in the monorepo root:

-   `docs/`: Umbrella directory housing authoritative architectural overviews,
    API preferences, and Interaction Model specification guides.
-   `ARCHITECTURE.md`: High-level structural overview of current monorepo
    operational layers.

### Level 2: Example-Specific Reference Guides

Whenever a task directs you to analyze, debug, extend, or operate on a concrete
example application (e.g., `examples/all-devices-app`, `examples/chip-tool`,
`examples/lighting-app`), do **not** rely solely on generic root guides.
Individual example applications maintain their own customized landing
documentation detailing custom CLI parameters, dynamic runtime composition
models, and operational test harnesses.

Before generating code, adding classes, or running test suites in an example
application, you **MUST** consult that application's landing `README.md` or its
dedicated `docs/` subdirectory.

## Prime Blueprint Example: `all-devices-app`

To demonstrate this incremental discovery protocol in practice, when working
with `examples/all-devices-app/`, agents must proactively read its dedicated
reference suite:

1. **`examples/all-devices-app/docs/architecture.md`**: Demonstrates the
   Code-Driven Data Model structure, explicit C++ `DeviceInterface` lifecycle
   registration (`Register`/`Unregister`), and explicit node teardown rules.
2. **`examples/all-devices-app/docs/starting_up.md`**: Outlines dynamic
   Interaction Model startup via CLI (`--device`), runtime node commissioning
   arguments, and network setup variables.
3. **`examples/all-devices-app/docs/adding_new_device.md`**: End-to-end tutorial
   on implementing a new simulated Matter device, deriving from
   `SingleEndpointDevice`, constructor dependency injection, `DeviceFactory`
   registration, and updating `targets.py` golden snapshots.
4. **`examples/all-devices-app/docs/testing.md`**: Factual verification
   methodologies demonstrating interactive commissioning via `chip-tool`,
   automated Python CI harnesses (`local.py`), and standalone test runners
   (`run_python_test.py`).
5. **`examples/all-devices-app/docs/custom_product_baseline.md`**: Transitioning
   from the dynamic runtime simulator to a fixed, static production firmware
   blueprint, optimizing RAM/Flash usage, and direct C++ member object
   instantiation.

## Strict Agent Invariants

-   **Never Alter Code Blindly**: Always verify example-specific documentation
    to match prevailing patterns before writing code.
-   **Canonical Output Paths**: When running compilation blocks or executing
    binaries discovered in documentation, ensure all commands reference
    canonical `./out/` output directories.
