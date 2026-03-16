# Migrating Ember Clusters to Code-Driven Implementations

This document provides a step-by-step guide for converting an existing
Ember-based cluster to a modern, code-driven implementation. It is recommended
to first read the guide on [Writing Clusters](./writing_clusters.md) for a
general overview of the code-driven cluster architecture.

---

## Migration Checklist

This checklist provides a granular, step-by-step process for migrating an Ember
cluster to a code-driven implementation.

### Part 0: Optimizing for an Easier Review

Before you begin the migration, consider structuring your changes into multiple,
smaller pull requests. This approach significantly simplifies the review
process, allowing reviewers to approve preliminary changes quickly. We recommend
the following sequence of PRs:

-   [ ] **PR 1: File Renames Only.**

    -   If your migration involves renaming files, submit a PR containing _only_
        the renames. A typical rename is from `<name>-server.cpp` to
        `<Name>Cluster.cpp`.
    -   **Note:** For backward compatibility with code generation, it is often
        best to **not** rename the header file.
    -   **Why:** This prevents `git diff` from becoming confused and showing the
        entire file as deleted and recreated, making the actual code changes
        impossible to review.
    -   _This type of PR can be reviewed and merged very quickly._

-   [ ] **PR 2: Code Movement Only.**

    -   If you plan to reorder functions or move code blocks (e.g., moving
        helper functions to an anonymous namespace), submit a PR with _only_
        these movements. Do not change any logic.
    -   **Why:** Reviewers can use tools like `git diff --color-moved` to verify
        that code has only been moved, not altered. This allows for a rapid
        review of structural changes.
    -   _This type of PR can also be fast-tracked._

-   [ ] **PR 3: The Core Logic Changes.**
    -   This PR should contain the actual migration logic: implementing the new
        cluster class, moving attribute storage, and converting command
        handlers.
    -   **Why:** With renames and code movements already handled, this PR will
        be much smaller and focused, allowing the reviewer to concentrate solely
        on the correctness of the migration logic.

This structure respects the reviewer's time and helps get your changes merged
faster. You can ask for an expedited review of the preliminary PRs in the
project's Slack channel.

### Part 1: Analysis and Design

-   [ ] **1.1: Understand the Existing Implementation:**

    -   [ ] Identify if the cluster uses a pure Ember implementation,
            `AttributeAccessInterface` (`AAI`), or `CommandHandlerInterface`
            (`CHI`).
        -   **Pure Ember Implementation:** For simple attributes, the Ember
            framework in `src/app/util` handles data storage and access. In a
            code-driven implementation, this data **must be moved into member
            variables** within your new cluster class.
        -   **AAI and CHI:** For more complex logic, `AAI` translates to the
            `ReadAttribute` and `WriteAttribute` methods, while `CHI` translates
            to the `InvokeCommand` method.
    -   [ ] Determine how attributes are stored (e.g., `persist`, `ram`,
            `callback`). This will inform how you handle data in the new
            implementation.
        -   If the value was `persist`, the new cluster should handle
            persistence (load in `Startup` and store during writes).
        -   If the value was `ram` and loaded from the ZAP UI,
            `CodegenIntegration.cpp` should load the value from ZAP via the
            generated `Accessors.h`. A common example is the `FeatureMap`.
        -   If the value is internal to the cluster (e.g., `ClusterRevision`),
            it should be marked as `External` by adding it to
            `attributeAccessInterfaceAttributes` in `zcl.json`.

-   [ ] **1.2: Choose an Implementation Pattern:**

    -   [ ] Decide between a combined or modular implementation based on the
            cluster's complexity and resource constraints. See the
            [Writing Clusters](./writing_clusters.md#choosing-the-right-implementation-pattern)
            guide for more details.

-   [ ] **1.3: Create the File Structure:**
    -   [ ] Use an existing directory for the cluster or create a new one if
            missing at `src/app/clusters/<name>/`.
        -   **Note:** If the cluster was a pure Ember implementation, this
            directory may not exist. After creating it, you must add a mapping
            to `src/app/zap_cluster_list.json` under the `ServerDirectories`
            key.
    -   [ ] Add the following files:
        -   `<Name>Cluster.h`
        -   `<Name>Cluster.cpp`
        -   `CodegenIntegration.cpp`
        -   `tests/Test<Name>Cluster.cpp`
        -   `BUILD.gn`
        -   `tests/BUILD.gn`
        -   `app_config_dependent_sources.gni`
        -   `app_config_dependent_sources.cmake`

### Part 2: Implementation

-   [ ] **2.1: Implement the Cluster Class:**

    -   [ ] Define the `<Name>Cluster` class, inheriting from
            `DefaultServerCluster`.
    -   [ ] Add member variables for all attributes previously handled by Ember.
    -   [ ] Implement the `Startup` and `Shutdown` methods for resource
            management.

-   [ ] **2.2: Implement Attribute Logic:**

    -   [ ] Implement the `ReadAttribute` and `WriteAttribute` methods,
            translating any existing `AAI` logic.
    -   [ ] For persisted attributes, use the `AttributePersistence` helper to
            load in `Startup` and save on writes.
    -   [ ] **Crucially,** after a successful write, call a notification
            function (e.g., `NotifyAttributeChangedIfSuccess`) to ensure
            subscriptions work correctly.

-   [ ] **2.3: Implement Command Logic:**

    -   [ ] Implement the `InvokeCommand` method, translating any existing `CHI`
            or `emberAf...Callback` logic.

        -   **Example:** An `emberAf<CLUSTER>Cluster<COMMAND>Callback` function
            becomes a `case` in the `InvokeCommand`'s `switch` statement:

            ```cpp
            // This:
            emberAfAccessControlClusterReviewFabricRestrictionsCallback(...);

            // Becomes this in the `InvokeCommand` implementation:
            switch (request.path.mCommandId) {
              // ...
              case AccessControl::Commands::ReviewFabricRestrictions::Id:
                 // ...
            }
            ```

    -   [ ] Use a `switch` statement on the command ID to handle different
            commands.

-   [ ] **2.4: Implement Event Logic:**

    -   [ ] Replace any calls to `LogEvent` with
            `mContext->interactionContext.eventsGenerator.GenerateEvent` to make
            events unit-testable.
    -   [ ] Check if any events have non-default access permissions (e.g.,
            require Administrator access) and implement the `EventInfo` method
            if necessary.

### Part 3: Configuration and Integration

-   [ ] **3.1: Update Build Files:**

    -   [ ] Configure `BUILD.gn` and `tests/BUILD.gn` to include the new source
            files.
    -   [ ] Configure `app_config_dependent_sources.gni` and
            `app_config_dependent_sources.cmake` to integrate the cluster into
            the build system.

-   [ ] **3.2: Implement Codegen Integration:**

    -   [ ] In `CodegenIntegration.cpp`, use the `CodegenClusterIntegration`
            helper to read configuration values from the generated code.
    -   > **Note:** The `CodegenClusterIntegration` helper for optional
        > attributes only supports attribute IDs up to 31. For clusters with
        > higher attribute IDs, you will need a custom implementation.

-   [ ] **3.3: Update ZAP Configuration:**
    -   [ ] In `src/app/common/templates/config-data.yaml`, add the cluster to
            the `CodeDrivenClusters` array and remove it from
            `CommandHandlerInterfaceOnlyClusters` if it exists there.
    -   [ ] In `src/app/zap-templates/zcl/zcl.json` and
            `zcl-with-test-extensions.json`, add all non-list attributes to the
            `attributeAccessInterfaceAttributes` list.
    -   [ ] Run the ZAP regeneration script:
        ```bash
        ./scripts/run_in_build_env.sh 'scripts/tools/zap_regen_all.py'
        ```

### Part 4: Testing

-   [ ] **4.1: Write Unit Tests:**

    -   [ ] In `tests/Test<Name>Cluster.cpp`, add unit tests for the new
            implementation.
    -   [ ] Ensure all attributes, commands, and feature combinations are
            tested.

-   [ ] **4.2: Perform Integration Testing:**
    -   [ ] Integrate the cluster into an example application (e.g.,
            `all-clusters-app`).
    -   [ ] Manually validate the cluster's functionality using `chip-tool` or
            `matter-repl`.

## Improvements to Consider

Cluster migrations are often not a full re-write of the code but rather the
minimum necessary to make the code testable and decoupled from Ember/ZAP.
Therefore, the suggestions in this section are optional.

### Remove Build Time Switches

Some legacy cluster implementations contain build time switches (`#ifdef`)
throughout the code. This is undesirable because it makes it difficult to change
the behavior of an application in runtime.

When possible, avoid copying this pattern and think of new ways the same thing
could be achieved. For example, use configuration injected via constructor or
implement separate sub classes if code size increase is a concern and the build
time options cause a significant difference in the cluster behavior.
