#include <stdio.h>
#include "chip-zcl.h"
#include "call-command-handler.h"

int main()
{
    ChipZclCommandContext_t context;
    context.mfgSpecific    = false;
    context.clusterId      = ZCL_ON_OFF_CLUSTER_ID;
    context.commandId      = ZCL_ON_COMMAND_ID;
    context.direction      = ZCL_DIRECTION_CLIENT_TO_SERVER;
    ChipZclStatus_t status = chipZclClusterSpecificCommandParse(&context);
    printf("Success: 0x%X \n", status);
}