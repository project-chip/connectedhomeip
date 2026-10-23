### ccache in CI: troubleshooting and controls

This document applies to GitHub Actions CI only.

The CI uses ccache to accelerate C/C++ builds. The cache is stored via GitHub
Actions Cache with weekly rotation, and ccache validates every object retrieval
for correctness.

Common tasks (CI)

-   Disable ccache for a run:
    -   Add `[no-ccache]` in the commit message, or
    -   Manually dispatch the `Tests` workflow with `disable_ccache=true`.
-   Force cache rotation for a run:
    -   Manually dispatch `Tests` with `cache_suffix` set to any string (e.g., a
        short hash). This appends to the cache key so a new cache archive is
        created.

When master builds fail

-   The Linux job automatically retries without ccache if the initial build
    fails on `master`.

Debugging tips (CI)

-   Print stats: use the existing `ccache -s` step to inspect hit/miss rates.
-   Compiler/version changes: protected by `CCACHE_COMPILERCHECK=content`, so
    caches are automatically missed on compiler changes.
-   Time macros: CI uses `CCACHE_SLOPPINESS=time_macros`. If your build embeds
    `__DATE__`/`__TIME__`, be aware objects may be reused across runs for these
    macros.

Notes

-   External cache keys rotate weekly; ccache still validates per-object for
    correctness.
