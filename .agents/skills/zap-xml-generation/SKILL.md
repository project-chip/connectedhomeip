---
name: zap-xml-generation
description: >-
    Generates and updates ZAP template XML files (.xml) in the Matter SDK from
    Matter specification AsciiDoc documents (.adoc) using the Alchemy CLI tool.
    Use when generating ZAP XMLs from Spec to SDK, updating cluster definitions
    (src/app/zap-templates/zcl/data-model/chip/*-cluster.xml), adding
    provisional clusters, updating global structs/enums
    (src/app/zap-templates/zcl/data-model/chip/global-structs.xml,
    src/app/zap-templates/zcl/data-model/chip/global-enums.xml), updating
    namespaces (src/app/zap-templates/zcl/data-model/chip/namespaces.xml) or
    device types (src/app/zap-templates/zcl/data-model/chip/device-types.xml -
    partial support), generating XMLs behind #ifdef tags (--attribute), or
    applying provisional policies (--provisional-policy). Don't use for
    analyzing .zap/.matter files, formatting AsciiDoc docs, or C++/Python
    changes.
---

# ZAP XML Generation from Spec to SDK (Alchemy)

This skill provides comprehensive instructions, scenarios, and commands for
generating and amending ZAP template XML files (`.xml`) in the Matter SDK
repository (`connectedhomeip`) from Matter specification AsciiDoc documents
(`.adoc`) using **Alchemy** (`alchemy zap`).

---

## 1. Overview & Tool Setup

Alchemy is the canonical command-line tool for transforming Matter specification
documents into code-generation artifacts. When invoked with the `zap`
subcommand, Alchemy parses the AsciiDoc data model definitions and translates
them into the ZAP XML files located in the Matter SDK under
`src/app/zap-templates/zcl/data-model/chip/`.

### 1.1 Obtaining & Building Alchemy in `alchemy` Folder

Always clone and build Alchemy inside its own dedicated `alchemy` folder (for
example, as a sibling directory `../alchemy`), rather than inside the
`connectedhomeip` repository:

```bash
# Clone and build inside a dedicated alchemy folder alongside your SDK/Spec clones
git clone https://github.com/project-chip/alchemy.git ../alchemy
cd ../alchemy
go build -o alchemy .
```

Alternatively, prebuilt binaries can be downloaded directly from the
[Alchemy GitHub releases page](https://github.com/project-chip/alchemy/releases).

### 1.2 Standard Repository Paths & Execution Context

When executing commands, assume you are running from inside the root directory
of the Matter SDK repository (`connectedhomeip`) and invoking the binary built
in the `alchemy` folder (`../alchemy/alchemy`, or `alchemy` if installed to your
`$PATH`):

```bash
cd <path_to_connectedhomeip>
../alchemy/alchemy zap --sdk-root=. --spec-root=../connectedhomeip-spec [options] [spec_doc_paths...]
```

-   **Current Working Directory (`CWD`)**: `connectedhomeip` (`.`)
-   **Alchemy Binary Location**: `../alchemy/alchemy` (built in `alchemy`
    folder)
-   **SDK Root (`--sdk-root`)**: `.` (the current working directory)
-   **Specification Root (`--spec-root`)**: `../connectedhomeip-spec` (assuming
    a sibling clone of `connectedhomeip-spec`)
-   **Output Location in SDK**: `src/app/zap-templates/zcl/data-model/chip/`

---

## 2. Core Generation Scenarios

### 2.1 Scenario 1: Amending an Existing Cluster (`-cluster.xml`)

**Use Case**: You modified, added, or removed attributes, commands, events, or
data types in an existing cluster AsciiDoc file
(`connectedhomeip-spec/src/app_clusters/<ClusterName>.adoc`) and need to update
its corresponding XML in the SDK (`<cluster-name>-cluster.xml`).

```bash
alchemy zap --sdk-root=. --spec-root=../connectedhomeip-spec ../connectedhomeip-spec/src/app_clusters/Thermostat.adoc
```

**Key Behaviors & Gotchas**:

-   **Amending by Default**: Alchemy reads the existing XML file and amends it.
    It preserves element order, existing comments, and unrecognized XML
    attributes while injecting or updating spec entities.
-   **Automatic Dependency Tracking**: If the target cluster relies on data
    types (structs, bitmaps, enums) defined in other cluster docs or global data
    types (`global-structs.xml`, `global-enums.xml`), Alchemy automatically
    generates/amends those dependency XMLs as well.
-   **Index & List Updates**: Alchemy automatically registers any updates in
    `src/app/zap-templates/zcl/data-model/chip/clusters.xml` and
    `src/app/zap-templates/zcl/zcl.json`.

---

### 2.2 Scenario 2: Generating XML for a New / Provisional Cluster

**Use Case**: A new cluster AsciiDoc document was introduced in
`connectedhomeip-spec` and does not yet have an XML template in the SDK.

```bash
alchemy zap --sdk-root=. --spec-root=../connectedhomeip-spec ../connectedhomeip-spec/src/app_clusters/NewAwesomeCluster.adoc
```

**Key Behaviors**:

-   Alchemy detects that `<cluster-name>-cluster.xml` does not exist and creates
    a new XML file from scratch with standard XML headers and copyright/license
    comments.
-   The new cluster is automatically added to `clusters.xml`, `zcl.json`, and
    provisional index files (`provisional.xml` / `index.xml` as applicable).

---

### 2.3 Scenario 3: Generating XML for Features Behind Conditional `#ifdef` Attributes

**Use Case**: A cluster, attribute, or feature in the specification is wrapped
in an AsciiDoc `ifdef` block (e.g., guarded by `in-progress` or a specific
feature flag like `my-cool-feature`).

```asciidoc
ifdef::in-progress,my-cool-feature[]
include::./MyAwesomeCluster.adoc[]
endif::[]
```

**Command**:

```bash
alchemy zap --attribute="in-progress" --attribute="my-cool-feature" --sdk-root=. --spec-root=../connectedhomeip-spec ../connectedhomeip-spec/src/app_clusters/MyAwesomeCluster.adoc
```

> [!IMPORTANT] > **Why `--attribute` is Mandatory**: Alchemy generates ZAP XML
> strictly based on what renders in the AsciiDoc document during processing. If
> a feature or cluster is hidden by an `ifdef::in-progress[]` condition and you
> do not pass `--attribute="in-progress"`, Alchemy will treat those entities as
> non-existent and will omit or delete them from the generated XML!

---

### 2.4 Scenario 4: Applying Provisional API Maturity Policies (`--provisional-policy`)

**Use Case**: Controlling how `apiMaturity="provisional"` XML attributes are
assigned to generated data types (Clusters, Attributes, Commands, Enums,
Structs, Bitmaps, Events) depending on their draft status in the specification.

```bash
alchemy zap --provisional-policy=loose --sdk-root=. --spec-root=../connectedhomeip-spec ../connectedhomeip-spec/src/app_clusters/Thermostat.adoc
```

**Available Policies**:

-   **`none` (Default)**:
    -   Clusters and attributes with provisional conformance in the spec are
        written with `apiMaturity="provisional"`.
    -   If they do not have provisional conformance, `apiMaturity` is removed if
        it previously existed.
    -   Structs that do not currently exist in the ZAP XML are written with
        `apiMaturity="provisional"`.
-   **`loose`**:
    -   Writes or clears `apiMaturity="provisional"` on **all** entity types
        (Attributes, Bitmaps, Clusters, Commands, Enums, Events, Features,
        Structs, and Fields).
    -   Entities are considered provisional unless they have a non-provisional
        conformance column OR are referenced by a non-provisional data type.
-   **`strict`**:
    -   Follows the same rules as `loose`, but **refuses to add new data types**
        to the generated XML if they are not provisional in the spec. Use this
        policy when enforcing strict API stability gates.

---

### 2.5 Scenario 5: Updating Device Types and Namespaces

**Use Case**: You modified Device Type definitions
(`connectedhomeip-spec/src/device_types/*.adoc`) or Namespace definitions
(`connectedhomeip-spec/src/namespaces/*.adoc`).

> [!WARNING] > **Device Types Support Limitation**: Note that automatic
> generation and patching of `device-types.xml` from AsciiDoc documents is
> currently **not fully supported** in Alchemy. While the command exists and
> attempts basic patching, output may be incomplete or experimental. Always
> manually inspect and verify any changes to `device-types.xml`.

```bash
# Update Device Types (experimental / not fully supported - modifies device-types.xml)
alchemy zap --sdk-root=. --spec-root=../connectedhomeip-spec ../connectedhomeip-spec/src/device_types/*.adoc

# Update Namespaces (modifies src/app/zap-templates/zcl/data-model/chip/namespaces.xml)
alchemy zap --sdk-root=. --spec-root=../connectedhomeip-spec ../connectedhomeip-spec/src/namespaces/*.adoc
```

**Key Behaviors**:

-   Namespace `.adoc` files map collectively to `namespaces.xml`.
-   Device type `.adoc` files map collectively to `device-types.xml`
    (`configurator/deviceType` elements), but note the partial support
    limitation above.

---

### 2.6 Scenario 6: Whole-Spec and Batch Processing

**Use Case**: Regenerating or updating ZAP XMLs across the entire specification
(e.g., during major SDK-Spec synchronizations).

```bash
alchemy zap --sdk-root=. --spec-root=../connectedhomeip-spec
```

_(When no specific `.adoc` files are passed, Alchemy parses all documents
discovered in `--spec-root` and updates all corresponding SDK XML files)._

---

## 3. Error Mitigation & Validation Scenarios

If the specification has syntax errors or missing dependencies, Alchemy will
halt with a fatal error:
`Alchemy was unable to proceed due to the following fatal errors in parsing the spec`

Use the following strategies based on your workflow needs:

### 3.1 Exclude Specific Errored Files (`--exclude`)

If a few unrelated spec documents fail to parse while you are generating XMLs
for a different cluster, exclude the broken paths:

```bash
alchemy zap --sdk-root=. --spec-root=../connectedhomeip-spec --exclude="*/BrokenCluster.adoc" ../connectedhomeip-spec/src/app_clusters/*.adoc
```

_(The `--exclude` flag supports glob patterns and can be passed multiple
times)._

### 3.2 Automatically Ignore Errored Files (`--ignore-errored`)

To automatically skip any spec file that encounters parsing errors while
generating XML for all valid files:

```bash
alchemy zap --ignore-errored --sdk-root=. --spec-root=../connectedhomeip-spec ../connectedhomeip-spec/src/app_clusters/*.adoc
```

### 3.3 Force Generation Despite Errors (`--force`)

Forces Alchemy to generate XML files on a best-effort basis even when parsing
errors occur:

```bash
alchemy zap --force --sdk-root=. --spec-root=../connectedhomeip-spec ../connectedhomeip-spec/src/app_clusters/*.adoc
```

> [!WARNING] Use `--force` only as a last resort for local debugging.
> Best-effort generation from partially parsed AsciiDoc files can produce
> incomplete or corrupted XML structures. Always prefer fixing the underlying
> specification error or using `--exclude`.

---

## 4. Preview & Verification Scenarios

### 4.1 Dry Run (`--dry-run` / `-d`)

Run all AsciiDoc parsing, dependency resolution, and XML rendering logic without
writing any files to disk. Useful for verifying that your spec changes parse
cleanly:

```bash
alchemy zap --dry-run --sdk-root=. --spec-root=../connectedhomeip-spec ../connectedhomeip-spec/src/app_clusters/Thermostat.adoc
```

### 4.2 Patch Output (`--patch` / `-p`)

Generate a unified diff (`.patch`) to `stdout` showing the exact XML additions,
modifications, and deletions without modifying the SDK directly:

```bash
alchemy zap --patch --sdk-root=. --spec-root=../connectedhomeip-spec ../connectedhomeip-spec/src/app_clusters/Thermostat.adoc > zap_changes.patch
```

---

## 5. Summary Cheatsheet

| Flag                   |          Default          | Description                                                                                           |
| :--------------------- | :-----------------------: | :---------------------------------------------------------------------------------------------------- |
| `--spec-root`          | `../connectedhomeip-spec` | Root path to the clone of the Matter Specification repository (`../connectedhomeip-spec` when in SDK) |
| `--sdk-root`           |            `.`            | Root path to the clone of the Matter SDK repository (`.` when inside `connectedhomeip`)               |
| `--attribute="<name>"` |           `""`            | Sets an AsciiDoc attribute (`#ifdef`) so guarded sections render (e.g., `"in-progress"`)              |
| `--provisional-policy` |          `none`           | Policy (`none`, `loose`, `strict`) for assigning `apiMaturity="provisional"` attributes               |
| `--exclude=<pattern>`  |           `""`            | Glob pattern of spec files to ignore during processing (can be repeated)                              |
| `--ignore-errored`     |          `false`          | Automatically skip spec documents that encounter parsing errors                                       |
| `--force`              |          `false`          | Force XML generation even if spec parsing errors occur                                                |
| `--dry-run` / `-d`     |          `false`          | Execute all generation logic without saving changes to disk                                           |
| `--patch` / `-p`       |          `false`          | Write unified diffs of XML changes to `stdout` instead of modifying files                             |
