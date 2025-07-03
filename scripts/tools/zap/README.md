---
orphan: true
---

# ZAP

This directory contains various tools related to ZAP.

# ZAP Cluster Revision Update Tool (update_cluster_revisions.py)

This tool parses all ZAP files in the codebase and updates cluster revisions.

**WARNING**: This tool only updates the revision number. Please ensure any new
attributes, events or commands are implemented accordingly.

## Prerequisites

This tool may require the python environment to parse the latest version of the
cluster revisions (i.e. if you don't provide a '--new-revision' argument). The
python environment can be built and activated using:

```
# Build
./scripts/build_python.sh -i out/python_env

# Activate
source out/python_env/bin/activate
```

## How to run

Usage:

```
usage: update_cluster_revisions.py [-h] [--cluster-id CLUSTER_ID] [--new-revision NEW_REVISION] [--old-revision OLD_REVISION] [--dry-run] [--parallel] [--no-parallel]

Update the ClusterRevision for a chosen cluster in all .zap files

options:
  -h, --help            show this help message and exit
  --cluster-id CLUSTER_ID
                        The id of the cluster, as hex, for which the cluster revision should be updated. If omitted, all outdated clusters are updated.
  --new-revision NEW_REVISION
                        The new cluster revision as a decimal integer. If omitted, the cluster revision will be updated to the latest according to the specification
  --old-revision OLD_REVISION
                        If set, only clusters with this old revision will be updated. This is a decimal integer.
  --dry-run             Don't do any generation, just log what .zap files would be updated (default: False)
  --parallel
  --no-parallel
```

[Note]

-   Use `--dry-run` to print only, don't update ZAP files
-   Omit `--cluster-id` to search for all clusters
-   Omit `--new-revision` to update to the latest revision according to the
    specification
-   Optionally provide `--old-revision`, `--cluster-id` and `--new-revision` to
    update only clusters that match `old-revision`.

Example #1: Check all outdated cluster revisions and print only (do not modify
the ZAP file `--dry-run`):

```
./scripts/tools/zap/update_cluster_revisions.py --dry-run --no-parallel
```

Example output if outdated clusters are found:

```
...
Checking for outdated cluster revisions on: examples/light-switch-app/light-switch-common/icd-lit-light-switch-app.zap
6 found!
Endpoint: 0 cluster_code: 40 cluster_revision: 3 cluster_spec_revision: 4 name: Basic Information
Endpoint: 0 cluster_code: 48 cluster_revision: 1 cluster_spec_revision: 2 name: General Commissioning
Endpoint: 0 cluster_code: 49 cluster_revision: 1 cluster_spec_revision: 2 name: Network Commissioning
Endpoint: 0 cluster_code: 53 cluster_revision: 2 cluster_spec_revision: 3 name: Thread Network Diagnostics
Endpoint: 1 cluster_code: 3 cluster_revision: 4 cluster_spec_revision: 5 name: Identify
Endpoint: 2 cluster_code: 3 cluster_revision: 2 cluster_spec_revision: 5 name: Identify
Checking for outdated cluster revisions on: examples/light-switch-app/light-switch-common/light-switch-app.zap
6 found!
Endpoint: 0 cluster_code: 40 cluster_revision: 3 cluster_spec_revision: 4 name: Basic Information
Endpoint: 0 cluster_code: 48 cluster_revision: 1 cluster_spec_revision: 2 name: General Commissioning
Endpoint: 0 cluster_code: 49 cluster_revision: 1 cluster_spec_revision: 2 name: Network Commissioning
Endpoint: 0 cluster_code: 53 cluster_revision: 2 cluster_spec_revision: 3 name: Thread Network Diagnostics
Endpoint: 1 cluster_code: 3 cluster_revision: 4 cluster_spec_revision: 5 name: Identify
Endpoint: 2 cluster_code: 3 cluster_revision: 2 cluster_spec_revision: 5 name: Identify
Checking for outdated cluster revisions on: examples/light-switch-app/qpg/zap/switch.zap
7 found!
Endpoint: 0 cluster_code: 40 cluster_revision: 3 cluster_spec_revision: 4 name: Basic Information
Endpoint: 0 cluster_code: 47 cluster_revision: 1 cluster_spec_revision: 3 name: Power Source
Endpoint: 0 cluster_code: 48 cluster_revision: 1 cluster_spec_revision: 2 name: General Commissioning
Endpoint: 0 cluster_code: 49 cluster_revision: 1 cluster_spec_revision: 2 name: Network Commissioning
Endpoint: 0 cluster_code: 53 cluster_revision: 2 cluster_spec_revision: 3 name: Thread Network Diagnostics
Endpoint: 1 cluster_code: 3 cluster_revision: 4 cluster_spec_revision: 5 name: Identify
Endpoint: 2 cluster_code: 3 cluster_revision: 4 cluster_spec_revision: 5 name: Identify
Checking for outdated cluster revisions on: src/controller/data_model/controller-clusters.zap
0 found!

Checking for outdated cluster revisions on: scripts/tools/zap/tests/inputs/lighting-app.zap
8 found!
Endpoint: 0 cluster_code: 40 cluster_revision: 3 cluster_spec_revision: 4 name: Basic Information
Endpoint: 0 cluster_code: 48 cluster_revision: 1 cluster_spec_revision: 2 name: General Commissioning
Endpoint: 0 cluster_code: 49 cluster_revision: 1 cluster_spec_revision: 2 name: Network Commissioning
Endpoint: 0 cluster_code: 53 cluster_revision: 2 cluster_spec_revision: 3 name: Thread Network Diagnostics
Endpoint: 0 cluster_code: 59 cluster_revision: 1 cluster_spec_revision: 2 name: Switch
Endpoint: 1 cluster_code: 3 cluster_revision: 4 cluster_spec_revision: 5 name: Identify
Endpoint: 1 cluster_code: 6 cluster_revision: 5 cluster_spec_revision: 6 name: On/Off
Endpoint: 1 cluster_code: 1030 cluster_revision: 4 cluster_spec_revision: 5 name: Occupancy Sensing
...
```

Example #2: Check for possible outdated revisions of the cluster 0x28 revisions
and print only (do not modify the ZAP file):

```
./scripts/tools/zap/update_cluster_revisions.py --dry-run --no-parallel --cluster-id 0x28
```

Example output:

```
...
Checking for outdated cluster revisions on: examples/bridge-app/bridge-common/bridge-app.zap
1 found!
Endpoint: 0 cluster_code: 0x28 cluster_revision: 3 cluster_spec_revision: 4 name: Basic Information
Checking for outdated cluster revisions on: examples/window-app/common/window-app.zap
1 found!
Endpoint: 0 cluster_code: 0x28 cluster_revision: 3 cluster_spec_revision: 4 name: Basic Information
Checking for outdated cluster revisions on: examples/all-clusters-minimal-app/all-clusters-common/all-clusters-minimal-app.zap
1 found!
Endpoint: 0 cluster_code: 0x28 cluster_revision: 3 cluster_spec_revision: 4 name: Basic Information
Checking for outdated cluster revisions on: examples/smoke-co-alarm-app/smoke-co-alarm-common/smoke-co-alarm-app.zap
1 found!
Endpoint: 0 cluster_code: 0x28 cluster_revision: 3 cluster_spec_revision: 4 name: Basic Information
Checking for outdated cluster revisions on: examples/placeholder/linux/apps/app1/config.zap
2 found!
Endpoint: 0 cluster_code: 0x28 cluster_revision: 2 cluster_spec_revision: 4 name: Basic Information
Endpoint: 1 cluster_code: 0x28 cluster_revision: 2 cluster_spec_revision: 4 name: Basic Information
...
```

Example #3: Update outdated revisions of the cluster 0x28 to the latest in the
specification:

```
./scripts/tools/zap/update_cluster_revisions.py --no-parallel --cluster-id 0x28
```

Example output:

```
Checking for outdated cluster revisions on: examples/window-app/common/window-app.zap
1 found!
Endpoint: 0 cluster_code: 0x28 cluster_revision: 3 cluster_spec_revision: 4 name: Basic Information
Cluster revisions updated successfully!

Searching for zcl file from /usr/local/google/home/sergiosoares/connectedhomeip/examples/window-app/common/window-app.zap
🔧 Using state directory: /usr/local/google/home/sergiosoares/.zap
🤖 Conversion started
    🔍 input files: /usr/local/google/home/sergiosoares/connectedhomeip/examples/window-app/common/window-app.zap
    🔍 output pattern: /usr/local/google/home/sergiosoares/connectedhomeip/examples/window-app/common/window-app.zap
    🐝 database and schema initialized
    🐝 zcl package loaded: /usr/local/google/home/sergiosoares/connectedhomeip/src/app/zap-templates/zcl/zcl.json
    🐝 templates loaded: /usr/local/google/home/sergiosoares/connectedhomeip/src/app/zap-templates/app-templates.json


🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨

Application is failing the Device Type Specification as follows:

  - ⚠ Check Device Type Compliance on endpoint: 0, device type: MA-rootdevice, cluster: Localization Configuration, attribute: ActiveLocale needs to be enabled
  - ⚠ Check Device Type Compliance on endpoint: 0, device type: MA-rootdevice, cluster: Localization Configuration, attribute: SupportedLocales needs to be enabled
  - ⚠ Check Device Type Compliance on endpoint: 2, device type: MA-windowcovering, cluster: Window Covering server needs bit 3 enabled in the Feature Map attribute

Application is failing the Cluster Specification as follows:


🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨


    👈 read in: /usr/local/google/home/sergiosoares/connectedhomeip/examples/window-app/common/window-app.zap
    👉 write out: /usr/local/google/home/sergiosoares/connectedhomeip/examples/window-app/common/window-app.zap
    👉 write out: undefined
😎 Conversion done!
```

Example #4: Update outdated revisions of the cluster 0x28 to version 3:

```
./scripts/tools/zap/update_cluster_revisions.py --no-parallel --cluster-id 0x28 --new-revision 3
```
