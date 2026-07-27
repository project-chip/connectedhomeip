# Matter (CHIP) Agent Skills

This directory contains skills and instructions for AI agents (Gemini, Co-pilot,
etc.) to assist with development and code review in the Matter repository.

## Available Skills

### Binary Size Comparison

-   **Location**: `.agents/skills/binary-size-comparison/`
-   **Purpose**: Guidelines for comparing binary (ELF) sizes using Rust-based
    tooling (`elf-bloat` and `asm-annotate`) across various platforms to analyze
    the footprint impact of changes.
-   **Triggers**: Use when analyzing the impact of code changes on binary
    footprint (size) or comparing binary sizes across platforms.

### Chip-Tool Testing

-   **Location**: `.agents/skills/chip-tool-testing/`
-   **Purpose**: Guidelines for building Matter example applications and
    `chip-tool`, and using `chip-tool` to commission and interact with the
    examples.
-   **Triggers**: Use when building and testing Matter example applications and
    using `chip-tool` for local commissioning and cluster interaction.

### Code-Driven Cluster Development

-   **Location**: `.agents/skills/code-driven-cluster-development/`
-   **Purpose**: Guidelines for implementing or migrating Matter server clusters
    using the `DefaultServerCluster` base class (code-driven data model
    approach).
-   **Triggers**: Automatically activates when developing, implementing, or
    migrating Matter server clusters using the code-driven data model approach.

### Code-Driven Cluster Migration

-   **Location**: `.agents/skills/code-driven-cluster-migration/`
-   **Purpose**: Step-by-step procedure for migrating a legacy Ember-based
    Matter server cluster in `src/app/clusters/` to the code-driven pattern.
-   **Triggers**: Use when converting an existing legacy Ember-based Matter
    server cluster to the code-driven pattern.

### Code-Driven Cluster TDD Implementation

-   **Location**: `.agents/skills/code-driven-cluster-tdd-implementation/`
-   **Purpose**: Guidelines for implementing or migrating Matter server clusters
    to the code-driven pattern using Test-Driven Development (TDD).
-   **Triggers**: Use when implementing or migrating Matter server clusters to
    the code-driven pattern using Test-Driven Development (TDD).

### Documentation Discovery

-   **Location**: `.agents/skills/documentation-discovery/`
-   **Purpose**: Essential guidelines instructing AI agents on how to
    incrementally discover, read, and consult top-level monorepo documentation
    and example-specific reference guides before altering code or executing test
    harnesses.
-   **Triggers**: Use when starting a new task, especially before altering code
    or executing test harnesses.

### GitHub Actions Failure Investigation

-   **Location**: `.agents/skills/github-action-failure-investigation/`
-   **Purpose**: Guidelines for fetching, downloading, and analyzing failed
    GitHub Actions runs, including raw logs, packet captures, and detecting
    runner freezes.
-   **Triggers**: Use when investigating a failed GitHub Actions run or job.

### GitHub PR Description Writer

-   **Location**: `.agents/skills/github-pr-description-writer/`
-   **Purpose**: Guidelines for generating clear, concise, and objective GitHub
    Pull Request descriptions.
-   **Triggers**: Use when preparing or writing descriptions for pull requests.

### Matter Specification Access

-   **Location**: `.agents/skills/matter-specification-access/`
-   **Purpose**: Guidelines for accessing and reading the private Matter
    specification and test plans (cloning spec and test plan repos, converting
    spec to markdown).
-   **Triggers**: Use when needing to access or read the Matter specification or
    test plans for verifying device semantics or mirroring test plans.

### Podman VSCode Build

-   **Location**: `.agents/skills/podman-vscode-build/`
-   **Purpose**: Guidelines for building and testing Matter examples using
    Podman with the official vscode build image in a non-interactive
    environment.
-   **Triggers**: Use when running builds or tests inside a Podman container,
    especially when targeting embedded or cross-compiled platforms.

### Python Test Reviewer

-   **Location**: `.agents/skills/python-test-reviewer/`
-   **Purpose**: Specialized guidance for reviewing Python tests.
-   **Triggers**: Automatically activates when reviewing files ending in
    `_test.py` or located in `src/python_testing`.

### Python Test Runner

-   **Location**: `.agents/skills/python-test-runner/`
-   **Purpose**: Guidelines and instructions for building prerequisite example
    applications and running python-based integration and certification tests
    located in `src/python_testing/`.
-   **Triggers**: Use when running python-based integration and certification
    tests, executing python test scripts, or running regression tests.

### ZAP and Matter File Analysis

-   **Location**: `.agents/skills/zap-matter-analysis/`
-   **Purpose**: Guidelines and common jq/grep/awk queries for investigating ZAP
    (`.zap`) and Matter (`.matter`) files to understand endpoints, clusters,
    attributes, and commands.
-   **Triggers**: Use when analyzing ZAP (`.zap`) and Matter (`.matter`) data
    model configuration files.

### ZAP XML Generation (Alchemy)

-   **Location**: `.agents/skills/zap-xml-generation/`
-   **Purpose**: Guidance and command patterns for generating and updating ZAP
    template XMLs from Matter specification AsciiDoc files
    (`connectedhomeip-spec`) to the Matter SDK (`connectedhomeip`) using the
    Alchemy CLI (`alchemy zap`).
-   **Triggers**: Automatically activates when modifying or generating ZAP XML
    artifacts (`src/app/zap-templates/zcl/data-model/chip/*-cluster.xml`,
    `src/app/zap-templates/zcl/data-model/chip/device-types.xml`,
    `src/app/zap-templates/zcl/data-model/chip/namespaces.xml`,
    `src/app/zap-templates/zcl/data-model/chip/global-structs.xml`,
    `src/app/zap-templates/zcl/data-model/chip/global-enums.xml`), handling
    `#ifdef` guarded spec attributes, or synchronizing spec-to-SDK data models.

## Using Skills

AI agents will automatically discover and load these skills when they are
relevant to your task. You can also explicitly ask the agent to "Use the
python-test-reviewer skill" if you want it to perform a focused review of your
test code.

---

_Note: This structure is designed to support both Gemini and Co-pilot agents.
Core guidelines are being migrated here from `.gemini/styleguide.md`._
