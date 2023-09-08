/**
 * @file
 * @brief Matter abstraction layer for Direct Internet Connectivity.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc.
 *www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon
 *Laboratories Inc. Your use of this software is
 *governed by the terms of Silicon Labs Master
 *Software License Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 *This software is distributed to you in Source Code
 *format and is governed by the sections of the MSLA
 *applicable to Source Code.
 *
 ******************************************************************************/

#include "silabs_utils.h"
#include "dic_control.h"
#include "dic.h"
#include <platform/CHIPDeviceLayer.h>

#ifdef ENABLE_AWS_OTA_FEAT
extern "C"{
   extern void aws_ota_init(void);
}
#endif

#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
#include  <app/clusters/on-off-server/on-off-server.h>
#endif //ZCL_USING_ON_OFF_CLUSTER_SERVER

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
#include <app/clusters/door-lock-server/door-lock-server.h>
#endif // ZCL_USING_DOOR_LOCK_CLUSTER_SERVER

#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
#include <app/clusters/window-covering-server/window-covering-server.h>
#endif //ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER

#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
#include "TemperatureManager.h"
#endif //ZCL_USING_THERMOSTAT_CLUSTER_SERVER

#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
#define DECIMAL 10
#define MSG_SIZE 5
#include "dic.h"

namespace ThermAttr = chip::app::Clusters::Thermostat::Attributes;
#endif //ZCL_USING_THERMOSTAT_CLUSTER_SERVER

using namespace chip;
using namespace chip::Platform;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters;


constexpr EndpointId kEndpointId = 1; // the clusters server could be configured on any endpoint
constexpr uint8_t kStringNotFound = 0xFF;
constexpr uint8_t kMaxCommandStrLength = 30;

struct mqttControlCommand {
    const char* cmdString;
    union{

#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
        CommandId cmdId;
#endif //ZCL_USING_ON_OFF_CLUSTER_SERVER

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
        DlLockState lockState;
#endif //ZCL_USING_DOOR_LOCK_CLUSTER_SERVER

#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
     void (*setPosition)(EndpointId, chip::app::Clusters::WindowCovering::NPercent100ths); // Function pointer to set the position
#endif //ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER

#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
        void (*setAttribute)(EndpointId, int32_t); // Function pointer to set the attribute
#endif //ZCL_USING_THERMOSTAT_CLUSTER_SERVER

    }action;
};

typedef struct mqttControlCommand mqttControlCommand;

#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
using namespace chip::app::Clusters::OnOff;

// OnOff cluster exposed commands to mqtt
static constexpr mqttControlCommand OnOffMqttControlCmd[] = {
    { .cmdString = "off", .action = { .cmdId = OnOff::Commands::Off::Id } },
    { .cmdString = "on", .action = { .cmdId = OnOff::Commands::On::Id } },
    { .cmdString = "toggle", .action = { .cmdId = OnOff::Commands::Toggle::Id } },
};
#endif //ZCL_USING_ON_OFF_CLUSTER_SERVER

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
using namespace chip::app::Clusters::DoorLock;
// Doorlock cluster exposed commands to mqtt
static constexpr mqttControlCommand DlMqttControlCmd[] = {
    { .cmdString = "unlock",  .action = { .lockState = DlLockState::kUnlocked } },
    { .cmdString = "lock", .action = { .lockState = DlLockState::kLocked } },
};
#endif // ZCL_USING_DOOR_LOCK_CLUSTER_SERVER

#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
using namespace chip::app::Clusters::WindowCovering;
static mqttControlCommand WindowMqttControlCmd[] = {
   { .cmdString = "Lift", .action = {
     .setPosition = [](EndpointId endpoint, chip::app::Clusters::WindowCovering::NPercent100ths value) {
     LiftPositionSet(endpoint, value);
   }
   }},
   { .cmdString = "Tilt", .action = {
     .setPosition = [](EndpointId endpoint, chip::app::Clusters::WindowCovering::NPercent100ths value) {
      TiltPositionSet(endpoint, value);
   }
   }},
};
#endif //ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER

#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
namespace ThermAttr = chip::app::Clusters::Thermostat::Attributes;
static mqttControlCommand thermostatMqttControlCmd[] = {
   { .cmdString = "SetMode", .action = {
     .setAttribute = [](EndpointId endpoint, int32_t value) {
     Thermostat::Attributes::SystemMode::Set(endpoint, value);
   }
   }},
   { .cmdString = "Heating", .action = {
     .setAttribute = [](EndpointId endpoint, int32_t value) {
     Thermostat::Attributes::OccupiedHeatingSetpoint::Set(endpoint, value);
   }
   }},
   { .cmdString = "Cooling", .action = {
     .setAttribute = [](EndpointId endpoint, int32_t value) {
     Thermostat::Attributes::OccupiedCoolingSetpoint::Set(endpoint, value);
   }
   }},
};
#endif // ZCL_USING_THERMOSTAT_CLUSTER_SERVER

// Macro to get the number of elemet of a given array
#define COUNT_OF(A) (sizeof(A) / sizeof((A)[0]))

/*
 * This function returns the index where a exposed command is in a given
 * mqttControlCommand can be found.
 * If the command string is not found/not exposed. This function returns kStringNotFound
 */
uint8_t GetCommandStringIndex(const mqttControlCommand* ctrlCmd,uint8_t size, char* stringToMatch)
{
    for (uint8_t index = 0; index < size; index++) {
        if (strcmp(stringToMatch, ctrlCmd[index].cmdString) == 0) {
            return index;
        }
    }

    return kStringNotFound; // string not found
}

namespace dic {
    namespace control {
/*
 * This function received data from dic mqtt. The data received is expected to be a
 * string matching exposed cluster commands.
 *
 * *** This function has no return for proof of concept sake,
 *     but error codes should be returned as many thing could happen here ***
 */

#ifdef ENABLE_AWS_OTA_FEAT
void aws_ota_init_task(void* parameters)
{
    ( void ) parameters;
    aws_ota_init();
}
#endif

void dic_incoming_data_cb(void* arg, const char* topic, uint16_t topic_len, const uint8_t* data, uint16_t len, uint8_t flags)
{
    const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;
    (void)client_info;

    if (data == nullptr) {
        return;
    }

    uint8_t cmdIndex = kStringNotFound;
    char cmd[kMaxCommandStrLength] = { 0 };
    VerifyOrReturn(kMaxCommandStrLength > len);
        CopyString(cmd, len+1,(const char*) data);
    std::string cmdString(cmd);
    char *_cmd = strtok(const_cast<char *>(cmdString.c_str()), "/");
    char *value = strtok(NULL, "/");
    int32_t _value = 0;
    if (value != NULL)
    {
    _value = std::stoi(value);
    }
    SILABS_LOG("_value=%d",_value);

#ifdef ENABLE_AWS_OTA_FEAT
    if(strcmp(cmd, "ota")==0)
    {
        static TaskHandle_t AWS_OTA = NULL;
        if(AWS_OTA == NULL)
        {   
            if ((pdPASS != xTaskCreate(aws_ota_init_task, "AWS_OTA", AWS_OTA_TASK_STACK_SIZE, NULL, AWS_OTA_TASK_PRIORITY, &AWS_OTA)) ||
                !AWS_OTA)
            {   
                SILABS_LOG("Failed to create AWS OTA Task");
                return;
            }
            SILABS_LOG("Task creation successfull for AWS OTA thread");
        }
        return;
    }
#endif

#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
    cmdIndex = GetCommandStringIndex(OnOffMqttControlCmd,COUNT_OF(OnOffMqttControlCmd),_cmd);
    if (cmdIndex != kStringNotFound) {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        OnOffServer::Instance().setOnOffValue(kEndpointId, OnOffMqttControlCmd[cmdIndex].action.cmdId, false);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        return;
    }
#endif // ZCL_USING_ON_OFF_CLUSTER_SERVER

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
    cmdIndex = GetCommandStringIndex(DlMqttControlCmd, COUNT_OF(DlMqttControlCmd), _cmd);
    if (cmdIndex != kStringNotFound) {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        DoorLockServer::Instance().SetLockState(kEndpointId, DlMqttControlCmd[cmdIndex].action.lockState, OperationSourceEnum::kUnspecified);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        return;
    }
#endif // ZCL_USING_DOOR_LOCK_CLUSTER_SERVER

#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
    cmdIndex = GetCommandStringIndex(WindowMqttControlCmd, COUNT_OF(WindowMqttControlCmd), _cmd);
    if (cmdIndex != kStringNotFound) {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        WindowMqttControlCmd[cmdIndex].action.setPosition(kEndpointId,(chip::app::Clusters::WindowCovering::NPercent100ths) _value);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        return;
    }
#endif //ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER

#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
cmdIndex = GetCommandStringIndex(thermostatMqttControlCmd, COUNT_OF(thermostatMqttControlCmd), _cmd);
if (cmdIndex != kStringNotFound) {
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    thermostatMqttControlCmd[cmdIndex].action.setAttribute(kEndpointId, _value);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    return;
    }

#endif // ZCL_USING_THERMOSTAT_CLUSTER_SERVER

    if (cmdIndex == kStringNotFound) {
        SILABS_LOG("Command %s not found", cmd);
    }
}

void SubscribeMQTT(intptr_t context){
    dic_mqtt_subscribe(NULL, NULL, dic_incoming_data_cb, MQTT_SUBSCRIBE_TOPIC, MQTT_QOS_0);
}

void subscribeCB(void){
     chip::DeviceLayer::PlatformMgr().ScheduleWork(SubscribeMQTT, reinterpret_cast<intptr_t>(nullptr));
}

#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
void AttributeHandler(EndpointId endpointId,AttributeId attributeId){
  switch (attributeId)
  {
    case ThermAttr::LocalTemperature::Id: {
      int8_t CurrentTemp = TempMgr().GetCurrentTemp();
      char buffer[MSG_SIZE];
      itoa(CurrentTemp, buffer, DECIMAL);
      dic_sendmsg("LocalTemperature/Temp",(const char *)(buffer));    
    }
    break;

    case ThermAttr::OccupiedCoolingSetpoint::Id: {
      int8_t coolingTemp =  TempMgr().GetCoolingSetPoint();
      char buffer[MSG_SIZE];
      itoa(coolingTemp, buffer, DECIMAL);
      dic_sendmsg("OccupiedCoolingSetpoint/coolingTemp",(const char *)(buffer));     
    }
    break;

    case ThermAttr::OccupiedHeatingSetpoint::Id: {
      int8_t heatingTemp = TempMgr().GetHeatingSetPoint();
      char buffer[MSG_SIZE];
      itoa(heatingTemp, buffer, DECIMAL);
      dic_sendmsg("OccupiedHeatingSetpoint/heatingTemp", (const char *)(buffer));
    }
    break;

    case ThermAttr::SystemMode::Id: {
      int8_t mode = TempMgr().GetMode();
      char buffer[MSG_SIZE];
      const char* Mode;
      switch (mode){
        case 0:
            Mode = "OFF";
            break;
        case 1:
            Mode = "HEAT&COOL";
            break;
        case 3:
            Mode ="COOL";
            break;
        case 4:
            Mode ="HEAT";
            break;
        default:
            Mode = "INVALID MODE";
            break;
      }
      uint16_t current_temp = TempMgr().GetCurrentTemp();
      itoa(current_temp, buffer, DECIMAL);
      dic_sendmsg("thermostat/systemMode", Mode);
      dic_sendmsg("thermostat/currentTemp", (const char *)(buffer));
    }
    break;

    default: {
      SILABS_LOG("Unhandled thermostat attribute %x", attributeId);
      return;
    }
    break;
  }
}
#endif //ZCL_USING_THERMOSTAT_CLUSTER_SERVER
} //control
} //dic