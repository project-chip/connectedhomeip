# The `.matter` IDL file format

The matter IDL file format is designed to be a human-readable representation of
data structures, cluster definitions and endpoint composition.

Since it is designed to be easy for both machine and humans to read, it is the
basis of some tools to make validating zap-based cluster definitions easier.

More details on the format in
[matter_idl/README.md](../../scripts/py_matter_idl/matter_idl/README.md).

## Parsing CSA XML Data definitions

The SDK contains a copy of CSA XML data definitions in `data_model/clusters`.

The files there are updated by running a scraper against the official matter
specification and are a good source of truth for actual definitions. Update
information available in [data_model/README.md](../../data_model/README.md).

NOTE: scraper is a work in progress, XML data may be incomplete or have errors
still.

The script `./scripts/py_matter_idl/matter_idl/data_model_xml_parser.py` has the
ability to parse one or more CSA data model XML files and output their content
in `.matter` format. For example:

```sh
./scripts/py_matter_idl/matter_idl/data_model_xml_parser.py data_model/clusters/BooleanState.xml
```

The tool supports several options that are useful for development:

| Argument(s)             | Description                                                                         |
| ----------------------- | ----------------------------------------------------------------------------------- |
| `-o/--output PATH`      | Output the matter file into a path instead of STDOUT.                               |
| `--compare PATH`        | Also read another `.matter` file for compare. MUST be used with `--compare-output`. |
| `--compare-output PATH` | Output the subset of `--compare` clusters that matches XML into PATH.               |

Using `--compare` AND `--compare-output` produce output that is easier to
compare as opposed to using existing zap-generated matter files because it
strips out comments and it also alpha-sorts elements so that diffs are
human-readable.

### Comparing a `.matter` file against the spec

Combining arguments to the tool allows getting a diff between SDK and
specification:

-   `data_model/clusters/*.xml` are assumed to be the official specification
    definitions
-   `src/controller/data_model/controller-clusters.matter` contains _all_
    clusters defined in the SDK

As such one can run compares such as:

```sh
./scripts/py_matter_idl/matter_idl/data_model_xml_parser.py         \
     -o out/spec.matter                                             \
     --compare-output out/sdk.matter                                \
     --compare src/controller/data_model/controller-clusters.matter \
     data_model/clusters/DoorLock.xml                               \
  && diff out/{spec,sdk}.matter
```

NOTE: due to specification data scraper still being in development, the diff
should be human-validated (e.g. for tool errors or Zigbee-only markers in the
spec).

## Linting `.matter` files for devices

For device validation, `./scripts/idl_lint.py` provides the ability to validate
a matter file for some basic conformance logic. These rules are expressed in
`scripts/rules.matterlint`.

The rules generally are:

-   pre-loaded from silabs XML files (validates that mandatory attributes are
    present)
-   Hard-coded rules (e.g. mandatory clusters and attributes on specific
    endpoints)

Usage:

```sh
./scripts/idl_lint.py ./examples/window-app/common/window-app.matter
```
