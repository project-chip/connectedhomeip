#!/bin/bash

## ---
# This is a helper script that runs all the yaml tests for all the application clusters enabled in the RVC App.
# Run the script from the root dir.
# The script takes the node ID that the device was commissioned with.
## ---

NODEID=$1
RVC_DEVICE_ENDPOINT=1
RVC_DEVICE_PICS="examples/rvc-app/rvc-common/pics/rvc-app-pics-values"

if [ -z "$NODEID" ]; then
    echo "Usage: run_all_yaml_tests [Node ID]"
    exit
fi

# RVC Clean Mode cluster
./scripts/tests/yaml/chiptool.py tests Test_TC_RVCCLEANM_1_1 --PICS "$RVC_DEVICE_PICS" --nodeId "$NODEID" --endpoint "$RVC_DEVICE_ENDPOINT" &&

    # RVC Operational State cluster
    ./scripts/tests/yaml/chiptool.py tests Test_TC_RVCOPSTATE_1_1 --PICS "$RVC_DEVICE_PICS" --nodeId "$NODEID" --endpoint "$RVC_DEVICE_ENDPOINT" &&

    # RVC Run Mode cluster
    ./scripts/tests/yaml/chiptool.py tests Test_TC_RVCRUNM_1_1 --PICS "$RVC_DEVICE_PICS" --nodeId "$NODEID" --endpoint "$RVC_DEVICE_ENDPOINT" &&
    echo done
