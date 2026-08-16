---
name: binary-size-comparison
description:
    Guidelines for comparing binary (ELF) sizes using Rust-based tooling
    (elf-bloat and asm-annotate) across various platforms to analyze the impact
    of code changes on footprint.
---

# Binary Size Comparison Workflow

This skill provides the workflow to compare binary sizes between your current
branch and a baseline (like the `master` fork-point).

## 1. Prerequisites & Tool Installation

### A. Containerized Build Setup

You MUST use the `podman-vscode-build` skill to handle container setup and build
execution.

-   **macOS Note**: As per `podman-vscode-build`, use `docker` instead of
    `podman` on macOS due to stability and proper Rosetta 2 support for the x86
    toolchains.

### B. Rust Tools Installation (macOS & Linux)

These tools must be installed on your **Host machine**. They require the Rust
toolchain.

_If you don't have Rust (`cargo`) installed, install it first:_

```bash
# This official installer works for both macOS and Linux
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source "$HOME/.cargo/env"
```

_Once Rust is available, install the size comparison tools:_

```bash
cargo install elf_bloat asm-annotate
```

## 2. Platform ELF Locations

Identify your target's ELF path relative to the repo root before you start:

-   **Linux / POSIX**: `out/<target>/<app-name>`
-   **ESP32**: `out/<target>/<app-name>.elf`
-   **Silicon Labs EFR32**: `out/<target>/<app-name>.out`
-   **nRF Connect (script)**:
    `examples/<app>/nrfconnect/build/nrfconnect/zephyr/zephyr.elf`
-   **nRF/Telink (build_examples.py)**: `out/<target>/zephyr/zephyr.elf`

## 3. Comparison Workflow

### A. Build the Baseline (Master Fork-Point)

Determine the fork-point to isolate only your changes.

```bash
CURRENT_COMMIT=$(git rev-parse HEAD)
FORK_POINT=$(git merge-base master HEAD)

git checkout $FORK_POINT
scripts/checkout_submodules.py --shallow --force

# BUILD STEP: Use 'podman-vscode-build' skill here
# Example: docker exec -w /workspace bld_vscode bash -c "source scripts/activate.sh && scripts/examples/nrfconnect_example.sh all-clusters-app nrf52840dk/nrf52840"

mkdir -p out/size-compares/master-baseline
cp <PATH_TO_ELF> out/size-compares/master-baseline/baseline.elf

git checkout $CURRENT_COMMIT
scripts/checkout_submodules.py --shallow --force
```

### B. Build the PR Branch

```bash
# BUILD STEP: Use 'podman-vscode-build' skill here
# Example: docker exec -w /workspace bld_vscode bash -c "source scripts/activate.sh && scripts/examples/nrfconnect_example.sh all-clusters-app nrf52840dk/nrf52840"

mkdir -p out/size-compares/pr-branch
cp <PATH_TO_ELF> out/size-compares/pr-branch/updated.elf
```

### C. Run and Analyze Comparison

Run `elf-bloat` on your **Host machine** to generate the report.

```bash
elf-bloat --compare-base out/size-compares/master-baseline/baseline.elf \
  --viewer none \
  out/size-compares/pr-branch/updated.elf \
  > out/size-compares/size_diff_report.csv

# View top 20 growth contributors (skips CSV header)
tail -n +2 out/size-compares/size_diff_report.csv | sort -t',' -k3 -rn | head -n 20
```

### D. Deep Dive Analysis (Optional)

To find the largest functions in a single binary non-interactively (without
TUI), use `elf-bloat` with the custom viewer to dump a CSV:

```bash
elf-bloat out/size-compares/pr-branch/updated.elf --viewer custom:cat > out/size-compares/single_size_report.csv
head -n 20 out/size-compares/single_size_report.csv
```

If a specific function grew significantly and you want to see exactly which C++
lines generated the bloated assembly, use `asm-annotate`. It maps the compiled
assembly instructions back to your original source code.

> **CRITICAL RULE FOR AI AGENTS**: ALWAYS use the `--dump` flag when running
> `asm-annotate`.

```bash
# Dump the assembly to a file for analysis or for pasting into a PR description
asm-annotate out/size-compares/pr-branch/updated.elf "chip::app::CodegenDataModelProvider::InitDataModelForTesting()" --dump > out/size-compares/annotated_function.txt
cat out/size-compares/annotated_function.txt
```
