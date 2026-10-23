---
name: matter-specification-access
description: >
    Guidelines for accessing and reading the Matter specification and test
    plans. These resources are private and in-progress, so they must be cloned
    or requested.
---

# Matter Specification Access

## Overview

The Matter specification and test plans are private repositories. Agents cannot
assume they know the latest specification content as it is actively developed.
This skill describes how to obtain and read these documents.

## Repositories

-   **Matter Specification**:
    `git@github.com:CHIP-Specifications/connectedhomeip-spec.git`
-   **Test Plans**: `git@github.com:CHIP-Specifications/chip-test-plans.git`

## Cloning Guidelines

-   **Check for existing checkout**: Before cloning, check if the repository is
    already available locally (e.g., in the user's home directory or a known
    location).
-   **Do NOT dirty the source tree**: Always clone into a location that is
    ignored or temporary.
    -   Recommended location: `out/` directory (e.g., `out/spec`,
        `out/test_plans`) as it is automatically ignored by the SDK build system
        and typically ignored by Git.
    -   Alternative: A system temporary directory.
-   **Use Shallow Cloning**: The repositories are very large. Always use
    `--depth 1` when cloning to save time and disk space.
    ```bash
    git clone --depth 1 git@github.com:CHIP-Specifications/connectedhomeip-spec.git out/spec
    git clone --depth 1 git@github.com:CHIP-Specifications/chip-test-plans.git out/test_plans
    ```

## Verifying Access

Since these repositories are private, some agents (e.g., running in CI or
restricted environments) may not have access.

-   **Lightweight Check**: Before attempting to clone the massive repositories,
    verify access using a non-interactive `git ls-remote`. This checks remote
    references without downloading any objects, while ensuring SSH fails fast
    instead of hanging on host-key or passphrase prompts.
    ```bash
    GIT_SSH_COMMAND='ssh -o BatchMode=yes -o ConnectTimeout=5' \
      git ls-remote git@github.com:CHIP-Specifications/connectedhomeip-spec.git
    ```
-   **Handling Failure**: If the command fails or would otherwise require
    interactive credentials or host-key confirmation (which automated agents
    cannot provide), assume access is unavailable. In this case:
    -   Ask the user for help or to provide the necessary files.
    -   Fall back to assuming the specification content is unknown and out of
        scope (as per general principles in `AGENTS.md`).

## Reading the Specification

The specification is written in **`AsciiDoc`** format.

-   **Prerequisites**: Generating Markdown from `AsciiDoc` requires **Docker**.
-   **Context Pollution**: `AsciiDoc` files may contain extensive license blurbs
    that can pollute the LLM context.
-   **Conversion to Markdown**: It is highly recommended to convert the spec to
    Markdown for better readability and reduced noise.
    -   Use the tool `tools/matter-to-markdown.sh` in the spec repository.
    -   **In-Progress Items**: To include in-progress work (which the SDK often
        tracks), pass `--include-in-progress 1` to the script. This enables the
        general `in-progress` flag in `Asciidoctor`. You can also pass specific
        feature flags if known (e.g., `--include-in-progress lsf`).
    -   Example command to build all specs with in-progress items:
        ```bash
        cd out/spec && ./tools/matter-to-markdown.sh --spec all --include-in-progress 1
        ```
-   **Targeted Reading**: The specification is extremely long. Avoid reading
    whole files if possible.

### Finding Information in Generated Markdown

Output is written to `build/markdown/<ref_label>/` (e.g.,
`build/markdown/master/` if on master branch).

-   **Cluster Specification**:
    -   Located in `build/markdown/<ref_label>/appclusters/` subdirectory.
    -   Files are split by chapters (e.g., `03-lighting.md`, `11-cameras.md`).
    -   Each file contains the clusters belonging to that functional domain.
        Look at `_index.md` in that directory for a list of chapters.
-   **Device Type Specification**:
    -   Located in `build/markdown/<ref_label>/device_library/` subdirectory.
    -   Files are split by chapters (e.g., `04-lighting-device-types.md`,
        `16-camera-device-types.md`).
    -   Each file contains the device types for that domain.
    -   Look at `_index.md` in that directory for a list of chapters.

## Reading Test Plans

Test plans are also in **`AsciiDoc`** format.

-   **No Markdown Conversion**: There is currently no official markdown
    conversion flow for test plans. They should be read as `AsciiDoc`.
-   **Location**:
    -   **Cluster Test Plans**: Individual cluster test plans are generally
        located in `src/cluster` within the test plans repository.
    -   **Filenames**: Filenames typically match the cluster name, but
        capitalization varies. Examples: `src/cluster/AccessControl.adoc`,
        `src/cluster/identify.adoc`, `src/cluster/onoff.adoc`.
    -   **System Test Plans**: Test plans for system-level features (like
        Interaction Model, Secure Channel) are often located in `src/` directly
        (e.g., `src/interactiondatamodel.adoc`, `src/securechannel.adoc`).
-   **Finding Information**: Use `grep` or similar tools to find the specific
    cluster name or feature in `out/test_plans/src/cluster` or
    `out/test_plans/src/`.
