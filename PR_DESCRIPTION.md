# Refactor Chime Cluster to Code-Driven Implementation

## Problem
The Chime Cluster implementation relied on legacy generated code structures and lacked a flexible, code-driven approach for handling attributes and commands. This made it difficult to extend, test, and maintain, especially with the need for custom logic like attribute persistence and delegate interaction.

## Solution
This PR refactors the `ChimeCluster` to follow the "Code-Driven" pattern, inheriting from `DefaultServerCluster`.

### Key Changes:
1.  **Code-Driven `ChimeCluster`**:
    *   Implemented `ChimeCluster` in `src/app/clusters/chime-server/ChimeCluster.cpp` and `.h`.
    *   Handles `ReadAttribute` and `WriteAttribute` for `SelectedChime`, `Enabled`, `ClusterRevision`, `FeatureMap`, and `InstalledChimeSounds`.
    *   Handles `InvokeCommand` for `PlayChimeSound`.
    *   Integrates with `SafeAttributePersistenceProvider` to persist `SelectedChime` and `Enabled` attributes across reboots.

2.  **Backwards Compatibility**:
    *   Retained the `ChimeServer` class as a wrapper around `RegisteredServerCluster<ChimeCluster>` in `CodegenIntegration.h` to minimize disruption to existing consumers.
    *   Maintained the protected member `mChimeServer` in `ChimeDelegate` (and exposed via `GetChimeCluster()`) to support existing application delegates (e.g., `camera-app`) that rely on this member name.

3.  **Testing**:
    *   Added `TestChimeCluster.cpp`: Comprehensive unit tests covering attribute reading/writing, command invocation, delegate interaction, and persistence.
    *   Added `TestChimeClusterBackwardsCompatibility.cpp`: specific tests to ensure the legacy `ChimeServer` wrapper works as expected.

4.  **Bug Fixes**:
    *   Ensured `mSelectedChime` and `mEnabled` are initialized to safe defaults (`0` and `true`) to handle cases where persistence loading fails (e.g., first boot).

## Verification
*   **Unit Tests**: `TestChimeCluster` and `TestChimeClusterBackwardsCompatibility` pass.
*   **Build**: `examples/camera-app/linux` builds successfully (verified manual check of `chime-manager.cpp` compatibility).

## Breaking Changes
None. The `ChimeDelegate` interface remains compatible, and `ChimeServer` wrapper bridges the gap for existing initializations.