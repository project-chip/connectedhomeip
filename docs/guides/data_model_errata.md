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

### Provisional Elements

Some specification features are developed ahead of the baseline data model
release. These "in-progress" or "provisional" elements may be implemented in the
SDK but not yet included in the checked-in XML baseline.

To reference a provisional element in the errata overlay without triggering
validation errors, set `is_provisional: true` on that element.

#### Provisional Attributes (Not in Baseline XML)

When a new attribute is added to a cluster in an in-progress specification
release but doesn't exist in the baseline XML, you must provide the attribute ID
in hex format (`0x00`-`0xFFFF`). The errata engine will:

1. Create an `XmlAttribute` object with `PROVISIONAL` conformance
2. Inject it into the cluster's attribute map
3. Allow the device to expose it without failing conformance tests

**Mandatory fields for provisional attributes:**

-   `is_provisional: true`
-   `attribute_id: <hex>` (required; e.g., `0x17`, `0xFF`)

**Optional fields:**

-   `read_access` (e.g., `RV`, `view`)
-   `write_access` (e.g., `administer`, `manage`, `none`)

**Example:**

```yaml
BasicInformation:
    DeviceLocation: # Not in baseline 1.6 XML; planned for 1.7
        is_provisional: true
        attribute_id: 0x17 # REQUIRED: Must be hex format
        write_access: administer
        read_access: RV
```

**Validation rules:**

-   `attribute_id` **must be provided** when `is_provisional` is `true`
-   `attribute_id` must be a **hexadecimal number** with `0x` prefix (e.g.,
    `0x17`, not `23`)
-   `attribute_id` must be in valid range: `0x00` to `0xFFFF`
-   Failure to provide a valid `attribute_id` for a provisional attribute will
    result in an error

#### Provisional Elements (Existing in Baseline)

For provisional elements that already exist in the baseline XML (such as
attributes on provisional clusters or clusters with provisional features), you
can simply set `is_provisional: true` without an `attribute_id`:

```yaml
SomeCluster:
    SomeProvisionalElement:
        is_provisional: true
        read_access: RV
```

In this case, the errata engine will:

1. Locate the existing element in the baseline XML
2. Apply the specified access overrides
3. Mark it as provisional in conformance checking

#### How Provisional Conformance Works

The conformance check (`device_conformance_tests.py`) evaluates each element's
conformance decision. When an element has `PROVISIONAL` conformance:

-   The element is allowed to be present on the device
-   The conformance test will pass even if the element is not in the baseline
    XML
-   The device's actual implementation is validated against the provisional spec
    definition

This enables:

-   Early implementation of "next" specification features
-   Parallel development of SDK and specification
-   Gradual transitions between specification versions

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
4. **Support New Attribute ID Formats**:
    - If adding new element types that require IDs (similar to provisional
      attributes), extend `_parse_and_validate_attribute_id()` to handle the new
      format requirements.
    - Ensure validation includes type checking, format validation, and range
      bounds checking.
5. **Unit Test Verification**: Always append a formal test method in
   `test_data_model_errata.py` verifying your new override key functions
   correctly and reports invalid inputs.

## Adding a New Errata Rule

1. Open `data_model/errata_future.yaml`.
2. Add your Cluster and Element override.
3. Include a comment referencing the Specification Pull Request, Issue, or SDK
   Pull Request explaining why the adaptation is necessary.

### Example: Adding a Provisional Attribute

```yaml
BasicInformation:
    # New DeviceLocation attribute being added in spec version 1.7.
    # SDK PR: https://github.com/project-chip/connectedhomeip/pull/71442
    DeviceLocation:
        is_provisional: true
        attribute_id: 0x17 # Attribute ID from the specification
        write_access: administer
        read_access: RV
```

### Example: Updating an Access Override

```yaml
AmbientContextSensing:
    # Fix spec typo where attribute was specified as RW instead of RV.
    # Reconciles against future spec PR #43327
    SimultaneousDetectionLimit:
        write_access: none
```

### Example: Bumping Cluster Revision

```yaml
GroupKeyManagement:
    # Cluster revision bumped to 4 in Matter 1.6.1 (Add "C" quality to GroupKeyMap attribute).
    # The baseline 1.6 XML lists revision 3, so override the reference revision.
    # TODO: Can be removed once 1.6.1 datamodel is generated and checked in.
    revision: 4
```

## Troubleshooting Errata Errors

If the errata engine reports errors during test execution, check:

1. **Unknown Cluster Name**: Ensure the cluster name uses clean PascalCase
   (e.g., `BasicInformation`, not `Basic Information` or `basic-information`)
2. **Unknown Element**: Verify the attribute/command name matches exactly in the
   specification (case-sensitive after sanitization)
3. **Missing attribute_id for Provisional Attribute**: All provisional
   attributes must include `attribute_id: 0x##` in hex format
4. **Invalid attribute_id Format**: Ensure the attribute ID starts with `0x`
   (e.g., `0x17`, not `23` or `17`)
5. **attribute_id Out of Range**: Ensure the attribute ID is between `0x00` and
   `0xFFFF`
6. **Invalid Access Code**: Use one of: `RV`, `RO`, `RM`, `RA`, `view`,
   `operate`, `manage`, `administer`, or `none`
