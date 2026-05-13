---
name: code-driven-cluster-tdd-implementation
description: >
    Guidelines for implementing or migrating Matter server clusters to the
    code-driven pattern using Test-Driven Development (TDD).
---

# Code-Driven Cluster TDD Implementation

## 1. Philosophy

Test-Driven Development (TDD) ensures that the cluster implementation adheres
strictly to the specification and maintains full feature parity if migrating an
existing cluster. By writing failing tests first, we validate our understanding
of the requirements before implementing them.

> [!IMPORTANT] > **Preserve Legacy Behavior**: If you are migrating an existing
> cluster or implementing a new version that has an existing implementation,
> constantly refer to the legacy code (e.g., by keeping a `.legacy` copy of the
> original file) to ensure you are not dropping existing functionality or
> changing behavior unexpectedly.

## 2. Prerequisites

-   Read `code-driven-cluster-development` for core implementation patterns.
-   If performing a **migration**, read `code-driven-cluster-migration` for
    general migration steps (renaming, directory layout). We assume you have
    already completed Phase 1 (Renames) and Phase 2 (Moves) before starting the
    TDD conversion.

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
    -   Write a **failing** test expecting `Attributes()` to return the list of
        mandatory attributes.
    -   Implement `Attributes()` using `AttributeListBuilder` and generated
        metadata.
    -   Verify test passes.
2.  **AcceptedCommands()**:
    -   Write a **failing** test expecting `AcceptedCommands()` to return the
        list of supported commands.
    -   Implement `AcceptedCommands()` returning the list (conditional on
        features).
    -   Verify test passes.

### Step 3: Implement ReadAttribute (TDD)

1.  For each mandatory or supported attribute:
    -   Write a **failing** test reading the attribute via
        `tester.ReadAttribute()` and expecting a default or mocked value.
    -   Implement the case in `ReadAttribute` switch, fetching data from
        Delegate or member variables.
    -   Ensure the `default` case returns
        `Protocols::InteractionModel::Status::UnsupportedAttribute` directly.
    -   Verify test passes.

### Step 4: Implement WriteAttribute (TDD)

For each writable attribute:

1.  **Write a Failing Test**:
    -   Write the attribute via `tester.WriteAttribute()` and expect failure or
        success depending on test setup.
2.  **Implement**:
    -   Add the case in `WriteAttribute` switch.
    -   Decode the payload using `AttributeValueDecoder`.
    -   Update the member variable or call the delegate.
    -   Use `SetAttributeValue` to update member variables and automatically
        notify subscribers. For complex cases (like lists), use
        `NotifyAttributeChanged` directly.
    -   Ensure the `default` case returns
        `Protocols::InteractionModel::Status::UnsupportedAttribute` directly.
3.  **Verify Success**:
    -   Run tests and ensure they pass.

### Step 5: Implement Commands in InvokeCommand (TDD)

For each command:

1.  **Write a Failing Test**:
    -   Invoke the command via `tester.Invoke()`.
    -   Assert failure (e.g. `UnsupportedCommand` or specific error before
        implementation).
2.  **Implement**:
    -   Add the case in `InvokeCommand` switch.
    -   Decode payload.
    -   Perform spec checks (CASE session, FailSafe, busy, etc.). Use standard
        `if` blocks instead of early-return macros (like `VerifyOrReturnError`)
        if cleanup or state modification is needed before returning.
    -   Call the appropriate `Delegate` method.
    -   Ensure the `default` case returns
        `Protocols::InteractionModel::Status::UnsupportedCommand` directly (do
        not delegate to base class).
3.  **Verify Success**:
    -   Run tests and ensure they pass.

### Step 6: Handle Platform Events (TDD)

1.  Identify needed events (e.g. `kFailSafeTimerExpired`,
    `kCommissioningComplete`).
2.  Write tests that trigger these events directly on the platform event
    handler.
3.  Implement `OnPlatformEventHandler` and hook it up in `Startup`/`Shutdown`.
4.  Verify tests pass.

### Step 7: Create CodegenIntegration Layer

1.  Create or update `CodegenIntegration.h` and `.cpp` in the cluster folder.
2.  Provide implementations for the generated callbacks (e.g.,
    `Matter<ClusterName>ClusterInitCallback` and `ShutdownCallback`). Note that
    the `IntegrationDelegate` can typically be stack-allocated if it is only
    used for lookups during the function call and does not need to outlive the
    call.
3.  Use `CodegenClusterIntegration::RegisterServer` (recommended) or direct
    registration to bridge ZAP defaults (like `FeatureMap` and optional
    attributes) to the new cluster instance.
4.  If you are migrating or replacing an existing implementation that has a
    legacy class applications interact with directly (e.g., `ChimeServer` or
    `Identify`), maintain it in `CodegenIntegration` and refactor it to act as a
    proxy wrapper around the new code-driven cluster implementation. This
    ensures existing applications do not need to change.

### Step 8: Verification & ZAP Regen

1.  Run all unit tests.
2.  Update `config-data.yaml` and `zcl.json` as per
    `code-driven-cluster-migration` skill.
3.  Run `zap_regen_all.py` and commit all generated files.
4.  **Integration Testing** (If migrating an existing cluster):
    -   **Verify Example App Build**: Identify an example app that uses the
        cluster, and build it with its specific target (e.g.,
        `linux-x64-network-manager-boringssl`).
    -   **End-to-End Testing**: Run the app, and use the `chip-tool-testing`
        skill to commission and test it against `chip-tool`.
5.  **Ensure all tests pass** before committing or pushing changes.

## 4. Common TDD Scenarios

### 4.1 Security Checks (CASE/FailSafe)

-   **Failing Test**: Use `tester.SetSubjectDescriptor()` to simulate non-CASE
    session, or don't arm FailSafe, and expect specific error
    (`UnsupportedAccess`, `FailsafeRequired`).
-   **Implementation**: Add checks at top of `InvokeCommand`.

### 4.2 Async Completion & Breadcrumbs

-   **Failing Test**: Mock the async callback and verify that side effects (like
    setting breadcrumb via `BreadCrumbTracker`) occur.
-   **Implementation**: Use the appropriate interface or callback (e.g.,
    `OnCommandNameComplete`) to handle side effects. Return
    `Status::UnsupportedCommand` for unknown commands in `InvokeCommand`.

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

## 5. Best Practices for Test Design

-   **Focused Tests**: Keep tests small and focused on a single requirement.
    Avoid monolithic tests that verify multiple behaviors at once.
-   **Isolation**: Use mocks (e.g., `MockDelegate`, `MockBreadcrumbTracker`) to
    isolate cluster logic from platform dependencies. This ensures tests are
    fast and deterministic.
-   **Edge Cases**: Always test boundary values (min/max) and invalid inputs to
    verify spec compliance and error handling.

## 6. Common Commands Reference

### Environment Activation

```bash
source scripts/activate.sh
```

Or run commands directly in the environment:

```bash
scripts/run_in_build_env.sh "<command>"
```

### Running Unit Tests

To compile and run a specific cluster test:

```bash
scripts/run_in_build_env.sh "ninja -C out/linux-x64-tests-clang src/app/clusters/<cluster-folder>/tests:Test<ClusterName>Cluster.run"
```

### ZAP Regeneration

To regenerate files after updating templates or ZCL JSON:

```bash
scripts/run_in_build_env.sh "scripts/tools/zap_regen_all.py"
```
