---
name: code-driven-cluster-tdd-implementation
description: >
    Guidelines for implementing or migrating Matter server clusters to the
    code-driven pattern using Test-Driven Development (TDD).
---

# Code-Driven Cluster TDD Implementation

## When to use this skill

Use this skill when implementing or migrating Matter server clusters to the
code-driven pattern using Test-Driven Development (TDD).

> [!IMPORTANT] > **Preserve Legacy Behavior**: If migrating an existing cluster,
> constantly refer to the legacy code to ensure no functional drop or unexpected
> behavior changes.

## 1. Prerequisites

-   Read `code-driven-cluster-development` for core implementation patterns.
-   Read `matter-specification-access` for instructions on how to access the
    Matter specification and test plans, so that tests can be based on the spec.
-   If performing a **migration**, read `code-driven-cluster-migration` for
    general migration steps (renaming, directory layout). We assume you have
    already completed Phase 1 (Renames) and Phase 2 (Moves) before starting the
    TDD implementation.

## 2. Step-by-Step Implementation Workflow

Follow these steps for the substantive implementation using TDD:

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

### Step 4: Implement Writable Attributes and Commands (TDD)

For each writable attribute or command, follow this cycle:

1.  **Write a Failing Test**:
    -   **Attributes**: Use `tester.WriteAttribute()` and expect failure or
        success based on setup.
    -   **Commands**: Invoke via `tester.Invoke()` and assert failure (e.g.,
        `UnsupportedCommand`).
2.  **Implement**:
    -   Add the case in `WriteAttribute` or `InvokeCommand` switch.
    -   Decode the payload.
    -   Execute logic (update state or call delegate).
    -   Return `UnsupportedAttribute` or `UnsupportedCommand` in the `default`
        case.
3.  **Verify Success**: Run tests and ensure they pass.

### Step 5: Create CodegenIntegration Layer

1.  Create or update `CodegenIntegration.h` and `.cpp` in the cluster folder.
2.  Provide implementations for generated callbacks (e.g.,
    `Matter<ClusterName>ClusterInitCallback`).
3.  Use `CodegenClusterIntegration::RegisterServer` to bridge ZAP defaults to
    the new cluster instance.
4.  Maintain legacy classes (e.g., `ChimeServer`) as proxy wrappers if needed
    for backward compatibility.

### Step 6: Verification & ZAP Regen

1.  Run all unit tests.
2.  Update `config-data.yaml` and `zcl.json` as per
    `code-driven-cluster-migration` skill.
3.  Run `zap_regen_all.py` and commit all generated files.
4.  **Integration Testing**: Build an example app and test against `chip-tool`.
5.  **Verify Spec Conformance**: Refer to the relevant cluster specification
    `.adoc` file.
6.  **Verify Against Test Plan**: Refer to the relevant test plan `.adoc` file.

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

### 4.5 Handling Platform Events

-   **Failing Test**: Write tests that trigger specific platform events (e.g.,
    `kFailSafeTimerExpired`) directly on the platform event handler and verify
    expected side effects.
-   **Implementation**: Implement `OnPlatformEventHandler` and hook it up in
    `Startup`/`Shutdown` to listen for needed events.

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
