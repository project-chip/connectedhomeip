/***************************************************************************/ /**
                                                                               *
                                                                               *    <COPYRIGHT>
                                                                               *
                                                                               *    Licensed under the Apache License, Version 2.0
                                                                               *(the "License"); you may not use this file except in
                                                                               *compliance with the License. You may obtain a copy
                                                                               *of the License at
                                                                               *
                                                                               *        http://www.apache.org/licenses/LICENSE-2.0
                                                                               *
                                                                               *    Unless required by applicable law or agreed to
                                                                               *in writing, software distributed under the License
                                                                               *is distributed on an "AS IS" BASIS, WITHOUT
                                                                               *WARRANTIES OR CONDITIONS OF ANY KIND, either express
                                                                               *or implied. See the License for the specific
                                                                               *language governing permissions and limitations under
                                                                               *the License.
                                                                               *
                                                                               ******************************************************************************/

// Cluster ids.
enum
{
    ZAP_CLUSTER_BASIC         = 0x0000,
    ZAP_CLUSTER_IDENTIFY      = 0x0003,
    ZAP_CLUSTER_GROUPS        = 0x0004,
    ZAP_CLUSTER_SCENES        = 0x0005,
    ZAP_CLUSTER_ON_OFF        = 0x0006,
    ZAP_CLUSTER_LEVEL_CONTROL = 0x0008,
    ZAP_CLUSTER_OTA_BOOTLOAD  = 0x2000,
};

// Basic cluster server (0x0000) - ResetToFactoryDefaults (0x00) command.
typedef struct
{
    bool dummy;
} ZapClusterBasicServerCommandResetToFactoryDefaultsRequest_t;
typedef struct
{
    enum8_t status;
} ZapClusterBasicServerCommandResetToFactoryDefaultsResponse_t;

ZapStatus_t zapSendClusterBasicServerCommandResetToFactoryDefaultsResponse(
    const ZapCommandContext_t * context, const ZapClusterBasicServerCommandResetToFactoryDefaultsResponse_t * response);

// -----------------------------------------------------------------------------
// On/off cluster (0x0006).

// On/off cluster client (0x0006).
extern const ZapClusterSpec_t zapClusterOnOffClientSpec;

// On/off cluster client (0x0006) - attribute ids.
enum
{
    ZAP_CLUSTER_ON_OFF_CLIENT_ATTRIBUTE_ON_OFF_CLUSTER_CLUSTER_REVISION_CLIENT = 0xFFFD,
    ZAP_CLUSTER_ON_OFF_CLIENT_ATTRIBUTE_ON_OFF_CLUSTER_REPORTING_STATUS_CLIENT = 0xFFFE,
};

// On/off cluster server (0x0006).
extern const ZapClusterSpec_t zapClusterOnOffServerSpec;

// On/off cluster server (0x0006) - attribute ids.
enum
{
    ZAP_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF                                 = 0x0000,
    ZAP_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_GLOBAL_SCENE_CONTROL                   = 0x4000,
    ZAP_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_TIME                                = 0x4001,
    ZAP_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_OFF_WAIT_TIME                          = 0x4002,
    ZAP_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_START_UP_ON_OFF                        = 0x4003,
    ZAP_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF_CLUSTER_CLUSTER_REVISION_SERVER = 0xFFFD,
    ZAP_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF_CLUSTER_REPORTING_STATUS_SERVER = 0xFFFE,
};

// On/off cluster server (0x0006) - command ids.
enum
{
    ZAP_CLUSTER_ON_OFF_SERVER_COMMAND_OFF                         = 0x00,
    ZAP_CLUSTER_ON_OFF_SERVER_COMMAND_ON                          = 0x01,
    ZAP_CLUSTER_ON_OFF_SERVER_COMMAND_TOGGLE                      = 0x02,
    ZAP_CLUSTER_ON_OFF_SERVER_COMMAND_OFF_WITH_EFFECT             = 0x40,
    ZAP_CLUSTER_ON_OFF_SERVER_COMMAND_ON_WITH_RECALL_GLOBAL_SCENE = 0x41,
    ZAP_CLUSTER_ON_OFF_SERVER_COMMAND_ON_WITH_TIMED_OFF           = 0x42,
};

// On/off cluster server (0x0006) - Off (0x00) command.
typedef struct
{
    bool dummy;
} ZapClusterOnOffServerCommandOffRequest_t;
typedef struct
{
    enum8_t status;
} ZapClusterOnOffServerCommandOffResponse_t;
ZapStatus_t zapSendClusterOnOffServerCommandOffResponse(const ZapCommandContext_t * context,
                                                        const ZapClusterOnOffServerCommandOffResponse_t * response);

// On/off cluster server (0x0006) - On (0x01) command.
typedef struct
{
    bool dummy;
} ZapClusterOnOffServerCommandOnRequest_t;
typedef struct
{
    enum8_t status;
} ZapClusterOnOffServerCommandOnResponse_t;
ZapStatus_t zapSendClusterOnOffServerCommandOnResponse(const ZapCommandContext_t * context,
                                                       const ZapClusterOnOffServerCommandOnResponse_t * response);

// On/off cluster server (0x0006) - Toggle (0x02) command.
typedef struct
{
    bool dummy;
} ZapClusterOnOffServerCommandToggleRequest_t;
typedef struct
{
    enum8_t status;
} ZapClusterOnOffServerCommandToggleResponse_t;
ZapStatus_t zapSendClusterOnOffServerCommandToggleResponse(const ZapCommandContext_t * context,
                                                           const ZapClusterOnOffServerCommandToggleResponse_t * response);

// -----------------------------------------------------------------------------
// Level Control cluster (0x0008).

// Level Control cluster client (0x0008).
extern const ZapClusterSpec_t zapClusterLevelControlClientSpec;

// Level Control cluster client (0x0008) - attribute ids.
enum
{
    ZAP_CLUSTER_LEVEL_CONTROL_CLIENT_ATTRIBUTE_LEVEL_CONTROL_CLUSTER_CLUSTER_REVISION_CLIENT = 0xFFFD,
    ZAP_CLUSTER_LEVEL_CONTROL_CLIENT_ATTRIBUTE_LEVEL_CONTROL_CLUSTER_REPORTING_STATUS_CLIENT = 0xFFFE,
};

// Level Control cluster server (0x0008).
extern const ZapClusterSpec_t zapClusterLevelControlServerSpec;

// Level Control cluster server (0x0008) - attribute ids.
enum
{
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL                                 = 0x0000,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_LEVEL_CONTROL_REMAINING_TIME                  = 0x0001,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_OPTIONS                                       = 0x000F,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_ON_OFF_TRANSITION_TIME                        = 0x0010,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_ON_LEVEL                                      = 0x0011,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_ON_TRANSITION_TIME                            = 0x0012,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_OFF_TRANSITION_TIME                           = 0x0013,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_DEFAULT_MOVE_RATE                             = 0x0014,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_START_UP_CURRENT_LEVEL                        = 0x4000,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_LEVEL_CONTROL_CLUSTER_CLUSTER_REVISION_SERVER = 0xFFFD,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_LEVEL_CONTROL_CLUSTER_REPORTING_STATUS_SERVER = 0xFFFE,
};

// Level Control cluster server (0x0008) - command ids.
enum
{
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_COMMAND_MOVE_TO_LEVEL             = 0x00,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_COMMAND_MOVE                      = 0x01,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_COMMAND_STEP                      = 0x02,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_COMMAND_STOP                      = 0x03,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_COMMAND_MOVE_TO_LEVEL_WITH_ON_OFF = 0x04,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_COMMAND_MOVE_WITH_ON_OFF          = 0x05,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_COMMAND_STEP_WITH_ON_OFF          = 0x06,
    ZAP_CLUSTER_LEVEL_CONTROL_SERVER_COMMAND_STOP_WITH_ON_OFF          = 0x07,
};

// Level Control cluster server (0x0008) - MoveToLevel (0x00) command.
typedef struct
{
    int8u level;
    int16u transitionTime;
} ZapClusterLevelControlServerCommandMoveToLevelRequest_t;
typedef struct
{
    enum8_t status;
} ZapClusterLevelControlServerCommandMoveToLevelResponse_t;
ZapStatus_t zapSendClusterLevelControlServerCommandMoveToLevelResponse(
    const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandMoveToLevelResponse_t * response);

// Level Control cluster server (0x0008) - Move (0x01) command.
typedef struct
{
    int8u moveMode;
    int8u rate;
} ZapClusterLevelControlServerCommandMoveRequest_t;
typedef struct
{
    enum8_t status;
} ZapClusterLevelControlServerCommandMoveResponse_t;
ZapStatus_t zapSendClusterLevelControlServerCommandMoveResponse(const ZapCommandContext_t * context,
                                                                const ZapClusterLevelControlServerCommandMoveResponse_t * response);

// Level Control cluster server (0x0008) - Step (0x02) command.
typedef struct
{
    int8u stepMode;
    int8u stepSize;
    int16u transitionTime;
} ZapClusterLevelControlServerCommandStepRequest_t;
typedef struct
{
    enum8_t status;
} ZapClusterLevelControlServerCommandStepResponse_t;
ZapStatus_t zapSendClusterLevelControlServerCommandStepResponse(const ZapCommandContext_t * context,
                                                                const ZapClusterLevelControlServerCommandStepResponse_t * response);

// Level Control cluster server (0x0008) - Stop (0x03) command.
typedef struct
{
    bool dummy;
} ZapClusterLevelControlServerCommandStopRequest_t;
typedef struct
{
    enum8_t status;
} ZapClusterLevelControlServerCommandStopResponse_t;
ZapStatus_t zapSendClusterLevelControlServerCommandStopResponse(const ZapCommandContext_t * context,
                                                                const ZapClusterLevelControlServerCommandStopResponse_t * response);

// Level Control cluster server (0x0008) - MoveToLevelWithOnOff (0x04) command.
typedef struct
{
    int8u level;
    int16u transitionTime;
} ZapClusterLevelControlServerCommandMoveToLevelWithOnOffRequest_t;
typedef struct
{
    enum8_t status;
} ZapClusterLevelControlServerCommandMoveToLevelWithOnOffResponse_t;
ZapStatus_t zapSendClusterLevelControlServerCommandMoveToLevelWithOnOffResponse(
    const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandMoveToLevelWithOnOffResponse_t * response);

// Level Control cluster server (0x0008) - MoveWithOnOff (0x05) command.
typedef struct
{
    int8u moveMode;
    int8u rate;
} ZapClusterLevelControlServerCommandMoveWithOnOffRequest_t;
typedef struct
{
    enum8_t status;
} ZapClusterLevelControlServerCommandMoveWithOnOffResponse_t;
ZapStatus_t zapSendClusterLevelControlServerCommandMoveWithOnOffResponse(
    const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandMoveWithOnOffResponse_t * response);

// Level Control cluster server (0x0008) - StepWithOnOff (0x06) command.
typedef struct
{
    int8u stepMode;
    int8u stepSize;
    int16u transitionTime;
} ZapClusterLevelControlServerCommandStepWithOnOffRequest_t;
typedef struct
{
    enum8_t status;
} ZapClusterLevelControlServerCommandStepWithOnOffResponse_t;
ZapStatus_t zapSendClusterLevelControlServerCommandStepWithOnOffResponse(
    const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandStepWithOnOffResponse_t * response);

// Level Control cluster server (0x0008) - StopWithOnOff (0x07) command.
typedef struct
{
    bool dummy;
} ZapClusterLevelControlServerCommandStopWithOnOffRequest_t;
typedef struct
{
    enum8_t status;
} ZapClusterLevelControlServerCommandStopWithOnOffResponse_t;
ZapStatus_t zapSendClusterLevelControlServerCommandStopWithOnOffResponse(
    const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandStopWithOnOffResponse_t * response);
