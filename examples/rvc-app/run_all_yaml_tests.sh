#!/bin/bash

## ---
# This is a helper script that runs all the yaml tests for all the application clusters enabled in the RVC App.
# Run the script from the root dir.
# The script takes the node ID that the device was commissioned with.
## ---

NODEID=$1
RVC_DEVICE_ENDPOINT=1

PICS_RUN="examples/rvc-app/rvc-common/pics/RVC_Clean_Mode_Cluster_Test_Plan.txt"
PICS_CLEAN="examples/rvc-app/rvc-common/pics/RVC_Run_Mode_Cluster_Test_Plan.txt"
PICS_OP_STATE="examples/rvc-app/rvc-common/pics/RVC_Operational_State_Cluster_Test_Plan.txt"

if [ -z "$NODEID" ]; then
    echo "Usage: run_all_yaml_tests [Node ID]"
    exit
fi

# RVC Clean Mode cluster
./scripts/tests/yaml/chiptool.py tests Test_TC_RVCCLEANM_1_1 --PICS "$PICS_CLEAN" --nodeId "$NODEID" --endpoint "$RVC_DEVICE_ENDPOINT" &&
    ./scripts/tests/yaml/chiptool.py tests Test_TC_RVCCLEANM_1_2 --PICS "$PICS_CLEAN" --nodeId "$NODEID" --endpoint "$RVC_DEVICE_ENDPOINT" &&
    ./scripts/tests/yaml/chiptool.py tests Test_TC_RVCCLEANM_2_1 --PICS "$PICS_CLEAN" --nodeId "$NODEID" --endpoint "$RVC_DEVICE_ENDPOINT" &&

    # RVC Operational State cluster
    ./scripts/tests/yaml/chiptool.py tests Test_TC_RVCOPSTATE_1_1 --PICS "$PICS_OP_STATE" --nodeId "$NODEID" --endpoint "$RVC_DEVICE_ENDPOINT" &&
    ./scripts/tests/yaml/chiptool.py tests Test_TC_RVCOPSTATE_2_2 --PICS "$PICS_OP_STATE" --nodeId "$NODEID" --endpoint "$RVC_DEVICE_ENDPOINT" &&

    # RVC Run Mode cluster
    ./scripts/tests/yaml/chiptool.py tests Test_TC_RVCRUNM_1_1 --PICS "$PICS_RUN" --nodeId "$NODEID" --endpoint "$RVC_DEVICE_ENDPOINT" &&
    ./scripts/tests/yaml/chiptool.py tests Test_TC_RVCRUNM_1_2 --PICS "$PICS_RUN" --nodeId "$NODEID" --endpoint "$RVC_DEVICE_ENDPOINT" &&
    ./scripts/tests/yaml/chiptool.py tests Test_TC_RVCRUNM_2_1 --PICS "$PICS_RUN" --nodeId "$NODEID" --endpoint "$RVC_DEVICE_ENDPOINT" &&
    echo done
