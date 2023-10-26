## Generator description

Generates a structured `Idl` data type into a human-readable text format
(`.matter` file).

It is useful for tools that ingest non-idl data but convert into idl data (e.g.
`zapxml` or CSA data model XML data.)

### Usage

A no-op usage can be:

```
./scripts/codegen.py -g idl --output-dir out/idlgen examples/all-clusters-app/all-clusters-common/all-clusters-app.matter
```

which would re-generate the entire `all-clusters-app.matter` into
`out/idlgen/idl.matter`

This generation is useful for testing/validating that both parsing and
generation works. Actual usage of this generator would be inside XML tools.

### Within XML parsing

A XML parser will use this code generator to output a human readable view of the
parsed data:

```
./scripts/py_matter_idl/matter_idl/zapxml_parser.py \
    ./src/app/zap-templates/zcl/data-model/chip/onoff-cluster.xml \
    ./src/app/zap-templates/zcl/data-model/chip/global-attributes.xm
```
