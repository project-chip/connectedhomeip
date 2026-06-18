# Vero AI Code Review Instructions — NXP connectedhomeip

> This file configures Vero AI reviewer behaviour for Pull Requests in the NXP
> fork of the Matter SDK.

---

## General Review Principles

- Be concise. Do not over-explain what you see in the code; only describe code
  when there is a specific concern or a question is asked.
- Do not repeat yourself across comments.
- Do not comment unless a change is probably desirable or clearly wrong.
- Ensure extensions or fixes to existing code match the prevailing style of the
  surrounding code.
- Look for common typos and suggest fixes.
- The SDK uses automated code formatting (clang-format, gn format, etc.). Do
  **not** comment on whitespace, line length, or other pure-formatting issues —
  the formatter handles these.
- Do not comment on content inside XML files or `.matter` cluster definition
  files.
- The SDK implements an in-progress Matter specification that may not be
  publicly available. Do **not** make assumptions about the Matter
  specification or flag code as wrong based on spec assumptions. Treat the
  specification as out of scope unless the PR author explicitly references it.

---

## NXP Commit Format — Mandatory Check

Every commit touching NXP-owned files **must** carry a tag prefix in its
commit title. Flag any commit that is missing a tag.

### Required tag format

```
[TAG][MODULE][TARGET] short description
```

| Field    | Allowed values                                                                 | Required for              |
|----------|--------------------------------------------------------------------------------|---------------------------|
| `TAG`    | `[nxp fromlist]`, `[nxp fromtree]`, `[nxp noup]`, `[nxp toup]`, `[nxp mergeup]` | **All NXP commits**       |
| `MODULE` | `platform`, `example`, `config`, `tools`, `doc`                                | Mandatory for `toup`/`noup` |
| `TARGET` | `rw61x`, `rt1170`, `rt1060`, `mcxw72`, `se05x`, `common`, `zephyr`            | Mandatory for `toup`/`noup` |

### Tag semantics — flag violations

| Tag              | Meaning & review check |
|------------------|------------------------|
| `[nxp fromlist]` | Patch has an upstream PR. Verify the commit message references the upstream PR URL. |
| `[nxp fromtree]` | Cherry-picked from upstream. Verify the upstream commit SHA is mentioned. **Always create a PR task** asking the author to confirm: *"No conflicts were encountered during the cherry-pick of this fromtree commit."* If there were conflicts, the following procedure must have been followed — resolving conflicts directly on the fromtree cherry-pick is **not** allowed: (1) Abort the cherry-pick. (2) Identify the older NXP commit causing the conflict (commit B — must carry `[nxp noup]` or `[nxp toup]`). (3) `git revert <commit-B-sha>`. (4) Re-apply commit A (the fromtree) — no conflict expected. (5) `git cherry-pick <commit-B-sha>` and resolve the conflict at this step only. This ensures the fromtree commit is applied cleanly and the NXP delta is rebased on top of it. |
| `[nxp noup]`     | NXP-specific, will **never** be upstreamed. A justification comment **must** be present in the commit message or in the code. Flag if missing. |
| `[nxp toup]`     | Targeted for upstream but blocked. A reason for the block **must** be stated. Flag if missing. |
| `[nxp mergeup]`  | Periodic upstream merge. Should not contain functional NXP changes — flag if it does. |

### Jira references — forbidden in commit titles and descriptions

**No Jira ticket reference is allowed in commit titles or commit descriptions.**
Jira tickets may only appear in the PR title or branch name.
Flag any commit message that contains a Jira key (e.g. `MATTER-1234`,
`CONNINT-567`, `WSW-890`).

---

## NXP File Ownership — Where NXP Changes Belong

NXP is responsible for maintaining files that contain `nxp` in their path.
Use this to guide review focus:

### FreeRTOS platform files (NXP-owned)
- `src/platform/nxp/**` — platform abstraction layer for NXP SoCs
- `examples/platform/nxp/**` — example platform glue (also in
  [`nxp_matter_support`](https://bitbucket.sw.nxp.com/projects/CONNINT/repos/nxp_matter_support) submodule)
- `examples/**/nxp/**` — NXP-specific example files. **Important:** some files
  under `examples/**/nxp/**` are duplicated (not symlinked) in
  [`mcuxsdk-examples-matter`](https://bitbucket.sw.nxp.com/projects/MCUCORE/repos/mcuxsdk-examples-matter).
  When a PR modifies such a file, flag it and ask the author to port the same
  change to the corresponding file in `mcuxsdk-examples-matter`.
- `config/nxp/**` — NXP build configuration
- `docs/platforms/nxp/**` — NXP platform documentation
- `third_party/nxp/**` — NXP third-party integrations
- `build/config/nxp/**` — NXP GN build rules

### Submodule: nxp_matter_support
The [`nxp_matter_support`](https://bitbucket.sw.nxp.com/projects/CONNINT/repos/nxp_matter_support)
submodule contains CMake files and `examples/platform` content specific to NXP.
Submodule pointer updates **must** be tagged `[nxp noup]` and **must be
isolated** in their own commit — never mixed with functional changes.

### Upstream / non-NXP files
Any change to files **outside** the `nxp` paths above touches upstream Matter
SDK code. Such changes:
1. Should ideally be submitted upstream to CSA first.
2. If kept downstream, must be tagged `[nxp toup]` with a justification.
3. Must **not** be tagged `[nxp noup]` unless there is a very strong reason
   (NXP-proprietary IP, NPI, etc.).

---

## Open-Source Centric Development — Key Rules to Enforce

NXP follows an upstream-centric development model. Flag deviations:

1. **Non-NXP code kept downstream without a `toup` tag** — any change to
   upstream Matter files that is not tagged `[nxp toup]` or `[nxp fromlist]`
   is a process violation.
2. **`noup` overuse** — `[nxp noup]` must be rare and justified. Flag if a
   `noup` commit touches generic Matter stack code that could reasonably be
   upstreamed.
3. **Mixed commits** — a single commit must not mix NXP-specific changes with
   upstream-eligible changes. Flag mixed commits and suggest splitting.
4. **Submodule/west manifest changes mixed with functional changes** — these
   must always be isolated commits.
5. **Mergeup commits containing functional NXP changes** — a `[nxp mergeup]`
   commit must only contain the upstream merge, not NXP functional patches.

---

## NXP Supported Targets

The following NXP platforms are supported in this repository. Use this list
when reviewing target-specific code or `[TARGET]` tags:

| Target tag | SoC / Platform |
|------------|----------------|
| `rw61x`    | RW612 / RW610 (Wi-Fi + BLE + 15.4 combo) |
| `rt1170`   | i.MX RT1170 |
| `rt1060`   | i.MX RT1060 |
| `mcxw72`   | MCXW72 |
| `se05x`    | SE05x secure element |
| `common`   | Shared across multiple NXP targets |
| `zephyr`   | NXP platforms built on Zephyr RTOS |

For Zephyr-based platforms, the NXP downstream Zephyr SDK (`nxp-zsdk`) is
required for features not yet in Zephyr upstream. Review Zephyr-related
changes against [`docs/platforms/nxp/nxp_zephyr_guide.md`](docs/platforms/nxp/nxp_zephyr_guide.md).

---

## Repository Architecture — Review Context

### Downstream release (FreeRTOS)
The manifest entry point is
[`mcuxsdk-manifests-connectivity-addons`](https://bitbucket.sw.nxp.com/projects/MCUCORE/repos/mcuxsdk-manifests-connectivity-addons)
(`west_matter_minimal.yml`). Matter examples live in
[`mcuxsdk-examples-matter`](https://bitbucket.sw.nxp.com/projects/MCUCORE/repos/mcuxsdk-examples-matter);
symbolic links are used to avoid duplication with in-tree examples.

### Upstream / master branch
`master` is the single source of truth. Release branches are created from
`master` only when needed (e.g. when non-release commits land after a
mergeup). All NXP-specific commits sit on top of the latest `[nxp mergeup]`
commit.

---

## PR Merge Rules — Flag Violations

The following rules apply to `master` and all release branches:

- **Minimum 2 approvals** from default reviewers are required before merge.
- **Failing builds must not be merged** (CI/CV waiver required for known
  test instability — must be explicitly noted in the PR).
- **Allowed merge methods:** rebase, `--ff-only` merge, squash + rebase or
  `--ff-only`. A regular merge commit (non-fast-forward) is **not** allowed.
- Flag PRs that appear to bypass these rules.

---

## Development Guides

Use the following SDK guides as reference when reviewing code patterns:

- [`docs/guides/writing_clusters.md`](docs/guides/writing_clusters.md) —
  how cluster handling must be implemented
- [`docs/guides/migrating_ember_cluster_to_code_driven.md`](docs/guides/migrating_ember_cluster_to_code_driven.md) —
  migrating ember clusters to code-driven approach
- [`docs/testing/unit_testing.md`](docs/testing/unit_testing.md) —
  unit testing patterns
- [`docs/testing/integration_tests.md`](docs/testing/integration_tests.md) —
  integration test patterns
- [`docs/platforms/nxp/`](docs/platforms/nxp/) —
  NXP platform-specific guides

---

## Integration Tests — Expected Failures

Tests in `src/python_testing` and `src/app/tests/suites` that verify expected
failures must clearly indicate — either in the test name/label or in a failure
output message — **why** the failure is expected. If a specific spec section
requires the failure, a summary of that requirement should be included.

---

## What NOT to Flag

- Whitespace, indentation, line length — handled by the auto-formatter.
- Copyright year values — humans will catch these.
- `.matter` file content or XML cluster definitions.
- Matter specification compliance — the spec may be in-progress and not
  publicly available; do not flag code as non-compliant based on spec
  assumptions.
- Upstream code style in files NXP does not own — defer to upstream
  maintainers.
