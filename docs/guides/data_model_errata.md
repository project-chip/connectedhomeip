# Data Model Errata Engine

The Matter SDK implements an in-progress Matter specification that may be in
flux. Our stable checked-in XML data models (located in `data_model/`) represent
a specific released baseline (e.g., `1.6`).

When Pull Requests develop features for "next" or implement corrections for
specification typos, they can cause Python Interaction Data Model (IDM) tests to
fail against the stable baseline XMLs.

To bridge this gap without making unauthorized manual edits to the
machine-generated XML files, the SDK provides a declarative **Data Model Errata
Engine**.

## Authoritative Errata Overlay

The adaptations live in a single top-level overlay file:
`data_model/errata_future.yaml`.

The YAML format requires an explicit revision compatibility guard and matches
target objects by exact name.

### Schema Example

```yaml
# Authoritative Errata to bridge baseline XMLs with Matter 'next' PR development.

# Guard to guarantee this overlay is only applied to compatible baseline revisions.
compatible_specification_revisions:
    - 1.6

AmbientContextSensing: # Auto-matched Cluster Name (Mandates sanitized PascalCase)
    SimultaneousDetectionLimit: # Auto-matched Attribute or Command Name
        read_access: RV # Mapped to AccessControlEntryPrivilegeEnum.kView
        write_access: none # Redefine or clear access
```

## How It Works Under the Hood

During test execution, the Python testing harness (`basic_composition.py`)
inspects the target device's `SpecificationVersion` attribute (from the
`BasicInformation` cluster on endpoint 0) to determine which baseline XML data
model directory (e.g., `1.6`) to load.

By default, low-level specification parsing (`spec_parsing.py`) applies **no
errata**, ensuring pure, strict certification validation. When executing
Interaction Data Model (IDM) tests or in-progress PR validation suites, the
harness explicitly enables errata loading (via the
`--enable-spec-errata-ci-only-disallowed-for-certification` CLI flag or
`spec_errata_path` configuration).

Right after assembling base and derived clusters, the parser invokes
`apply_errata` (from `matter.testing.data_model_errata`). This verifies that the
active specification revision matches the YAML's compatibility list, resolves
element names via sanitized AST maps (`_sanitize_name()` matches keys to
lowercase structures, ensuring spaces and punctuation like `On/Off` map to clean
CamelCase) while actively rejecting raw XML names with spaces or slashes, and
applies the specified overrides in memory.

### Supported Overrides

-   **Cluster**: `revision` (integer). A reserved cluster-level key that
    overrides the cluster's reference revision (see "The `revision` key" below).
-   **Attributes**: `read_access`, `write_access` (Supports standard access
    privilege codes: `RV`, `RO`, `RM`, `RA`, `none`, or `view`, `operate`,
    `manage`, `administer`).
-   **Commands**: `invoke_access` / `privilege` (Supports standard access
    privilege codes: `RV`, `RO`, `RM`, `RA`, `none`, or `view`, `operate`,
    `manage`, `administer`).

### The `revision` key

`revision` is a reserved key matched at the **cluster** level (a sibling of
attribute/command element names, not an element name itself):

```yaml
GroupKeyManagement:
    revision: 4
```

#### How it works

The conformance check (`device_conformance_tests.py`) compares the device's
reported `ClusterRevision` attribute (`0xFFFD`) against the _reference_ revision
from the `<cluster revision="...">` XML property, parsed into
`XmlCluster.revision`. This revision is parsed as cluster property, not an
attribute, so it can't be targeted by an attribute name (it isn't in
`attribute_map`).

The `revision` keys allows the errata engine to parses the value as an integer
and writes it directly to `XmlCluster.revision`, so the test compares against
the overridden reference instead of the stale baseline.

Use this when a cluster revision is bumped in a "next" spec release (or SDK PR)
ahead of the checked-in baseline XML.

## Extending Engine Capabilities (Supporting New Errata Overrides)

If your development requires overriding XML elements or fields not currently
handled by the engine (such as attribute `conformance` or event privileges),
follow these steps to extend the engine core:

1. **Locate the Engine Core**: Open
   `src/python_testing/matter_testing_infrastructure/matter/testing/data_model_errata.py`.
2. **Modify Element Resolution**:
    - For existing target types (like Attributes or Commands), locate their
      corresponding helper functions (e.g., `_apply_attribute_errata` or
      `_apply_command_errata`).
    - Add a new check for your intended override key (e.g.,
      `if 'conformance' in overrides:`).
    - Parse the YAML value and directly mutate the target object field (e.g.,
      `target_attribute.conformance = ...`).
3. **Support New Target Element Types**:
    - If extending support to target completely new AST structures (such as
      `events` or `structs`), insert an additional lookup branch in
      `_apply_element_errata` (e.g., resolving against a sanitized event map).
    - Retrieve the target object from `target_cluster.events` and apply the
      intended alterations.
4. **Unit Test Verification**: Always append a formal test method in
   `test_data_model_errata.py` verifying your new override key functions
   correctly and reports invalid inputs.

## Adding a New Errata Rule

1. Open `data_model/errata_future.yaml`.
2. Add your Cluster and Element override.
3. Include a comment referencing the Specification Pull Request, Issue, or SDK
   Pull Request explaining why the adaptation is necessary.
