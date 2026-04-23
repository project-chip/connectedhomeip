---
name: code-driven-cluster-migration
description: >
    Step-by-step procedure for migrating a legacy Ember-based Matter server
    cluster in src/app/clusters/ to the code-driven pattern. Use this skill when
    converting an existing cluster. All rules from
    code-driven-cluster-development still apply; this skill adds the
    migration-specific workflow on top.
---

# Code-Driven Cluster Migration

## 1. When to Invoke This Skill

Invoke this skill when you are converting an existing cluster under
`src/app/clusters/<folder>/` that does not yet contain a
`CodegenIntegration.cpp` file (note: the folder naming convention is
inconsistent — it may be `<name>-server` or simply `<name>`). If the cluster is
already migrated, use `code-driven-cluster-development` instead.

All rules from `code-driven-cluster-development` apply to the converted cluster.
This skill specifies the additional migration workflow: PR structure, backward
compatibility, and integration wiring.

---

## 2. The Three-PR Migration Workflow

Conversion **must** be split into at least three pull requests to ensure
reviewability. Mixing these phases is a common cause for review rejection.

| PR    | Contents                                                      | Validation Method                     |
| ----- | ------------------------------------------------------------- | ------------------------------------- |
| **1** | File renames and compat-stub creation **only**.               | `git diff --color-moved`              |
| **2** | Code reordering / anonymous-namespace moves **only**.         | `git diff --color-moved=dimmed-zebra` |
| **3** | Substantive conversion: new class, integration, tests, regen. | Functional review + `ninja check`     |

---

## 3. Phase 0: Discovery & Strategy

### 3.1 Baseline & Map Blast Radius

-   **Inventory Attributes**: Identify Mandatory (`M`), Optional (`O`), Fixed
    (`F`), and Nullable attributes from the cluster XML and generated
    `Attributes.h`.
-   **Map Consumers**: Search `examples/` for references to the legacy
    `<name>-server.h` or `class <Name>Server`. This determines your
    backward-compat strategy.

### 3.2 Backward-Compat Strategy

The standard approach preserves full include and API compatibility:

-   **CodegenIntegration.cpp**: Keep any legacy public APIs alive here as thin
    forwarders to the new cluster class. This is the preferred location for all
    backward-compat shims.
-   **Legacy header (`<name>-server.h`)**: If it existed, update it to
    `#include` the new `CodegenIntegration.h`. This preserves include-path
    compatibility for application code without duplicating logic.
-   **Breaking changes**: If direct Ember RAM writes (`Attributes::X::Set`) are
    used by apps and cannot be shimmed, you **must** update those apps in PR #3
    and document the change in a cluster `README.md`.

### 3.3 Pick a Reference PR

Identify the "shape" of your cluster and study the corresponding reference PR's
diff end-to-end.

| Your cluster looks like...               | Reference PR                                                                           |
| ---------------------------------------- | -------------------------------------------------------------------------------------- |
| Nullable measurement + min/max/tolerance | [#71424](https://github.com/project-chip/connectedhomeip/pull/71424) Relative Humidity |
| Mandatory list + delegate + commands     | [#43471](https://github.com/project-chip/connectedhomeip/pull/43471) Actions           |
| Multi-instance (per-endpoint state)      | [#43720](https://github.com/project-chip/connectedhomeip/pull/43720) Closure Dimension |
| Singleton, node-scoped                   | [#40422](https://github.com/project-chip/connectedhomeip/pull/40422) Basic Information |
| Command-heavy with delegate              | [#42331](https://github.com/project-chip/connectedhomeip/pull/42331) Chime             |
| Writable scalar + features               | [#42968](https://github.com/project-chip/connectedhomeip/pull/42968) Switch            |
| Runtime-only (no Ember defaults)         | [#71552](https://github.com/project-chip/connectedhomeip/pull/71552) Flow Measurement  |
| Identify/timer-driven                    | [#41232](https://github.com/project-chip/connectedhomeip/pull/41232) Identify          |

---

## 4. Implementation Phases

### 4.1 Phase 1: Renames (PR #1)

-   Rename `.cpp` and `.h` to PascalCase (`<Name>Cluster.cpp`).
-   Create a one-line compat stub at the old header path.
-   Update `BUILD.gn` with renamed files. **No logic changes.**

### 4.2 Phase 2: Moves (PR #2)

-   Move helpers to anonymous namespaces or group command handlers. **No logic
    changes.**

### 4.3 Phase 3: The Conversion (PR #3)

1. **Strip Shell**: Remove `emberAf...` calls and
   `MatterPostAttributeChangeCallback`.
2. **Implement Class**: Follow the patterns in
   `code-driven-cluster-development`.
3. **Integration Layer**: Create `CodegenIntegration.cpp` to bridge ZAP defaults
   to your new cluster.
4. **ZAP/Build Updates**:
    - Update `src/app/common/templates/config-data.yaml` (add to
      `CodeDrivenClusters`).
    - Update `src/app/zap-templates/zcl/zcl.json`
      (`attributeAccessInterfaceAttributes`).
5. **Regeneration**: Run `scripts/tools/zap_regen_all.py` and commit **all**
   generated files.

---

## 5. Unit Testing with ClusterTester

The `ClusterTester` helper class (in
`src/app/server-cluster/testing/ClusterTester.h`) is mandatory for testing
code-driven clusters. It abstracts TLV handling, memory management for views,
and fabric context.

### 5.1 Key Capabilities

-   **Automatic TLV Handling**: Encodes C++ types for writes/commands and
    decodes for reads.
-   **Memory Safety**: Maintains ownership of TLV data for views (like
    `CharSpan` or `List`) during the test scope.
-   **Type Safety**: Uses generated types (e.g., `Attributes::MyAttr::TypeInfo`)
    to ensure spec alignment.

### 5.2 Core Patterns

-   **Attributes**: Use `tester.ReadAttribute(Id, outValue)` and
    `tester.WriteAttribute(Id, inValue)`.
-   **Commands**: Use `tester.Invoke(requestStruct)`. It returns an
    `InvokeResult` containing both status and response data.
-   **Side Effects**:
    -   Verify events via `tester.GetNextGeneratedEvent()`.
    -   Verify dirty markings (reporting) via `tester.GetDirtyList()`.
-   **Fabric Scoping**: Use `tester.SetFabricIndex(idx)` to simulate actions
    from specific fabrics.

---

## 6. Ensuring Spec Compliance

Tests must not just "pass" — they must prove the cluster adheres to the Matter
Specification and Test Plans.

### 6.1 Consult the Source of Truth

Use the `matter-specification-access` skill to obtain and read the latest
specification and test plans.

-   **Validate Semantics**: Ensure `ReadAttribute` and `WriteAttribute` handle
    constraints (nullable, min/max, fixed) exactly as defined in the spec.
-   **Mirror Test Plans**: Use the `.adoc` test plans (e.g.,
    `src/cluster/<name>.adoc` in `chip-test-plans` repo) as the blueprint for
    your unit tests. If a test plan requires a `ConstraintError` for a specific
    value, your unit test must assert exactly that.

### 6.2 Mandatory Test Coverage

-   **Attribute Constraints**: Below min, at min, at max, above max.
-   **Nullable Handling**: Null sentinel rejection (e.g., `0xFFFF` for `uint16`)
    for nullable numeric types.
-   **Feature Gating**: Verify that optional attributes/commands are only
    present when the corresponding FeatureMap bits are set (using `Attributes()`
    / `AcceptedCommands()`).

---

## 7. Ember → Code-Driven Mapping Reference

| Ember / Legacy Construct       | Code-Driven Equivalent                              |
| ------------------------------ | --------------------------------------------------- |
| `emberAfReadAttribute`         | Member access + `encoder.Encode`                    |
| `Attributes::X::Set`           | `cluster->SetX(value)` (Forwarded)                  |
| `PreAttributeChangedCallback`  | Validation logic in `WriteAttribute`                |
| `PostAttributeChangeCallback`  | Automatic via `SetAttributeValue`                   |
| `class FooServer : public AAI` | `class <Name>Cluster : public DefaultServerCluster` |

---

## 8. Common Review Findings (Migration Specific)

1. **Mixed PRs**: Never combine renames with logic changes.
2. **Missing Regen**: Forgetting to commit files changed by `zap_regen_all.py`
   breaks the build for other apps.
3. **Double Notification**: Migrated clusters often have redundant
   `NotifyAttributeChanged` calls; use `SetAttributeValue` to avoid this.
4. **Incorrect Fallbacks**: When reading ZAP defaults in `CodegenIntegration`,
   ensure you handle read failures with safe, spec-compliant defaults.
5. **Missing README**: If you break backward compatibility, a `README.md`
   explaining the migration for app developers is mandatory.

---

## 9. References

-   **Implementation Patterns**:
    `.agents/skills/code-driven-cluster-development/SKILL.md`
-   **Migration Guide**: `docs/guides/migrating_ember_cluster_to_code_driven.md`
