# The tooling for validating zap files

The SDK contains a copy of CSA XML data definitions in `data_model/clusters`.

The script `./scripts/helpers/check_zap_against_xml.py` can check zap files
against the official XML data definition.

Currently, the only check is that the ClusterRevision in the zap file matches
the latest ClusterRevision in the XML. For example:

```sh
./scripts/helpers/check_zap_against_xml.py --zap examples/lighting-app/lighting-common/lighting-app.zap --matter-root .
```
