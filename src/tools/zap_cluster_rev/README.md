---
orphan: true
---

# ZAP Cluster Revision Bump Tool

This tool parses ZAP files and updates outdated cluster revisions according to
the specification.

# Prerequisites

This tool uses the python environment, which can be built and activated using:

```
# Build
./scripts/build_python.sh -i out/python_env

# Activate
source out/python_env/bin/activate
```

# How to run

Usage:

```
usage: zap_cluster_rev.py [-h] (--print-only | --update) filenames [filenames ...]
```

Example #1: Check cluster revisions and update a ZAP file:

```
python src/tools/zap_cluster_rev/zap_cluster_rev.py --update examples/network-manager-app/network-manager-common/network-manager-app.zap
```

Expected output if outdated clusters are found:

```
python src/tools/zap_cluster_rev/zap_cluster_rev.py --update examples/network-manager-app/network-manager-common/network-manager-app.zap
Checking for outdated cluster revisions on: examples/network-manager-app/network-manager-common/network-manager-app.zap
3 found!
Endpoint: 0 cluster_code: 40 cluster_revision: 3 cluster_spec_revision: 4 name: Basic Information
Endpoint: 0 cluster_code: 48 cluster_revision: 1 cluster_spec_revision: 2 name: General Commissioning
Endpoint: 0 cluster_code: 53 cluster_revision: 2 cluster_spec_revision: 3 name: Thread Network Diagnostics
Cluster revisions updated successfully!
```

Expected output if no outdated clusters are found:

```
Checking for outdated cluster revisions on: examples/network-manager-app/network-manager-common/network-manager-app.zap
0 found!
```

Example #2: Check the cluster revisions and print only (do not modify the ZAP
file):

```
python src/tools/zap_cluster_rev/zap_cluster_rev.py --print-only examples/network-manager-app/network-manager-common/network-manage
r-app.zap
```

Expected output:

```
Checking for outdated cluster revisions on: examples/network-manager-app/network-manager-common/network-manager-app.zap
3 found!
Endpoint: 0 cluster_code: 40 cluster_revision: 3 cluster_spec_revision: 4 name: Basic Information
Endpoint: 0 cluster_code: 48 cluster_revision: 1 cluster_spec_revision: 2 name: General Commissioning
Endpoint: 0 cluster_code: 53 cluster_revision: 2 cluster_spec_revision: 3 name: Thread Network Diagnostics
```

The option `--print-only` is useful for testing and ensuring the tool is
identifying the right outdated clusters before updating.
