### ccache troubleshooting and controls

This repo uses ccache in CI to accelerate C/C++ builds. The cache is stored via GitHub Actions Cache with weekly rotation and validated by ccache on every object retrieval.

Common tasks
- Disable ccache for a run:
  - Add `[no-ccache]` in the commit message, or
  - Manually dispatch the `Tests` workflow with `disable_ccache=true`.
- Force cache rotation for a run:
  - Manually dispatch `Tests` with `cache_suffix` set to any string (e.g. a short hash). This appends to the cache key so a new cache archive is created.
- Clear/remove cache for local runs:
  - Delete the `.ccache` directory in the workspace.

On master failures
- CI will automatically retry the Linux and Darwin builds without ccache if the first attempt fails on master. This guards against rare cache/tooling interactions.

Debugging tips
- Print stats: add a step `ccache -s` (already present in workflows) to inspect hit/miss rates.
- Compiler/version changes: protected by `CCACHE_COMPILERCHECK=content` so caches are automatically missed.
- Time macros: we use `CCACHE_SLOPPINESS=time_macros`. If your build embeds `__DATE__`/`__TIME__`, consider disabling sloppiness locally to verify.

Notes
- External cache keys rotate weekly; ccache still validates per-object for correctness.
- You can also export `CCACHE_DISABLE=1` locally to bypass ccache entirely.
