---
name: code-driven-cluster-tdd-migration
description: >
    Guidelines for migrating Matter server clusters to the code-driven pattern
    using Test-Driven Development (TDD).
---

# Code-Driven Cluster TDD Migration

## 1. Philosophy

Test-Driven Development (TDD) ensures that the migrated cluster maintains full
feature parity with the legacy implementation and adheres strictly to the
specification. By writing failing tests first, we validate our understanding of
the requirements before implementing them.

> [!IMPORTANT] > **Preserve Legacy Behavior**: Constantly refer to the legacy
> implementation (e.g., by keeping a `.legacy` copy of the original file) to
> ensure you are not dropping existing functionality or changing behavior
> unexpectedly, unless explicitly required by spec updates.

## 2. Prerequisites

-   Read `code-driven-cluster-migration` skill for general migration steps
    (renaming, directory layout).
-   Read `code-driven-cluster-development` for core implementation patterns.

## 3. Step-by-Step Conversion Workflow (Phase 3)

Once Phase 1 (Renames) and Phase 2 (Moves) are complete, follow these steps for
the substantive conversion (Phase 3) using TDD:

### Step 1: Initial Boilerplate & Tests Setup

1.  Create the new cluster class inheriting from `DefaultServerCluster`.
2.  Create a basic test fixture in `tests/Test<ClusterName>Cluster.cpp` using
    `ClusterTester`.
3.  Verify that the cluster compiles and tests can run (even if empty).

### Step 2: Implement Static Metadata Methods (TDD)

1.  **Attributes()**:
    -   Write a test expecting `Attributes()` to return the list of mandatory
        attributes (and optional ones based on feature flags).
    -   Implement `Attributes()` using `AttributeListBuilder` and generated
        metadata.
    -   Verify test passes.
2.  **AcceptedCommands()**:
    -   Write a test expecting `AcceptedCommands()` to return the list of
        supported commands.
    -   Implement `AcceptedCommands()` returning the list (conditional on
        features).
    -   Verify test passes.

### Step 3: Implement ReadAttribute (TDD)

1.  For each mandatory or supported attribute:
    -   Write a test reading the attribute via `tester.ReadAttribute()` and
        expecting a default or mocked value.
    -   Implement the case in `ReadAttribute` switch, fetching data from
        Delegate or member variables.
    -   Verify test passes.

### Step 4: Implement Commands in InvokeCommand (TDD)

For each command:

1.  **Write a Failing Test**:
    -   Invoke the command via `tester.Invoke()`.
    -   Assert failure (e.g. `UnsupportedCommand` or specific error before
        implementation).
2.  **Implement**:
    -   Add the case in `InvokeCommand` switch.
    -   Decode payload.
    -   Perform spec checks (CASE session, FailSafe, busy, etc.).
    -   Call the appropriate `Delegate` method.
3.  **Verify Success**:
    -   Run tests and ensure they pass.

### Step 5: Handle Platform Events (TDD)

1.  Identify needed events (e.g. `kFailSafeTimerExpired`,
    `kCommissioningComplete`).
2.  Write tests that trigger these events directly on the platform event
    handler.
3.  Implement `OnPlatformEventHandler` and hook it up in `Startup`/`Shutdown`.
4.  Verify tests pass.

### Step 6: Create CodegenIntegration Layer

1.  Create or update `CodegenIntegration.h` and `.cpp` in the cluster folder.
2.  Provide implementations for the generated callbacks (e.g.,
    `Matter<ClusterName>ClusterInitCallback` and `ShutdownCallback`).
3.  Use `CodegenClusterIntegration::RegisterServer` (recommended) or direct
    registration to bridge ZAP defaults (like `FeatureMap` and optional
    attributes) to the new cluster instance.
4.  If the cluster has a legacy class that applications interact with directly
    (e.g., `ChimeServer` or `Identify`), maintain it in `CodegenIntegration` and
    refactor it to act as a proxy wrapper around the new code-driven cluster
    implementation. This ensures existing applications do not need to change.

### Step 7: Verification & ZAP Regen

1.  Run all unit tests.
2.  Update `config-data.yaml` and `zcl.json` as per
    `code-driven-cluster-migration` skill.
3.  Run `zap_regen_all.py` and commit all generated files.
4.  Verify by building an example app that uses the cluster.

## 4. Common TDD Scenarios

### 4.1 Security Checks (CASE/FailSafe)

-   **Failing Test**: Use `tester.SetSubjectDescriptor()` to simulate non-CASE
    session, or don't arm FailSafe, and expect specific error
    (`UnsupportedAccess`, `FailsafeRequired`).
-   **Implementation**: Add checks at top of `InvokeCommand`.

### 4.2 Async Completion & Breadcrumbs

-   **Failing Test**: Mock the async callback and verify that side effects (like
    setting breadcrumb via `BreadCrumbTracker`) occur.
-   **Implementation**: Use `BreadCrumbTracker` interface in
    `OnActivateDatasetComplete`.

### 4.3 Timer-Driven Logic

-   **Failing Test**: Verify that time passes and state changes (e.g.,
    `IdentifyTime` decrements).
-   **Mocking**: Do NOT use `sleep()`. Use `TimerDelegate` and
    `TimerDelegateMock` to advance the mock clock and trigger timer callbacks
    synchronously.

### 4.4 List Attributes

-   **Failing Test**: Read the list attribute and verify its contents.
-   **Implementation**: Use `ListEncodeHelper` and delegate methods to fetch
    items by index and encode them.

## 5. Best Practices

-   Keep tests small and focused on a single requirement.
-   Use mocks (e.g. `MockDelegate`, `MockBreadcrumbTracker`) to isolate cluster
    logic from platform dependencies.
-   Ensure all tests pass before committing.
