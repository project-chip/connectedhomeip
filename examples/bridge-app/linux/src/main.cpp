/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <AppMain.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Events.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventLogging.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/reporting/reporting.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/ZclString.h>
#include <platform/CommissionableDataProvider.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include "CommissionableInit.h"
#include "main.h"
#include <app/server/Server.h>

/*----------------------------------------------------------------
------------------------------- Dev-----------------------------
----------------------------------------------------------------*/


#include <app/clusters/switch-server/switch-server.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <cassert>
#include <iostream>
#include <vector>
#include "LightSwitchMgr.h"
#include <app/AttributeAccessInterface.h>
#include <app/util/binding-table.h>
#include <cstring>
#include <csignal>
#include "OTA.h"
#include "DeviceMatter.h"
#include "HandleAttributeSensor.h"
#include "HandleAttributeLight.h"
#include "IoTDeviceManager.h"
#include "MQTTHandler.h"


using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;
using namespace MatterDevice;



// REVISION DEFINITIONS:
// =================================================================================

#define ZCL_DESCRIPTOR_CLUSTER_REVISION (1u)
#define ZCL_IDENTIFY_CLUSTER_REVISION (4u)
#define ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_CLUSTER_REVISION (1u)
#define ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_FEATURE_MAP (0u)
#define ZCL_FIXED_LABEL_CLUSTER_REVISION (1u)

#define ZCL_ON_OFF_CLUSTER_REVISION (4u)
#define ZCL_ON_OFF_CLUSTER_FEATURE_MAP (0u)


#define ZCL_SWITCH_CLUSTER_REVISION (1u)
#define ZCL_BINDING_CLUSTER_REVISION (1u)
#define ZCL_COLOR_CLUSTER_REVISION (5u)
#define ZCL_GROUPS_CLUSTER_REVISION (4u)
#define ZCL_SCENES_CLUSTER_REVISION (4u)
// ---------------------------------------------------------------------------

IoTDeviceManager manager("/var/tmp/data/devices.json");
MQTTHandler mqttClient(manager, "/var/tmp/data/mqtt_config.json");

/* ------------------------------------------------------------------------ */

/*
    Add Endpoints to the Bridge
*/
int AddDeviceEndpoint(EmberAfEndpointType * ep, const Span<const EmberAfDeviceType> & deviceTypeList,
                      const Span<DataVersion> & dataVersionStorage, chip::EndpointId parentEndpointId = chip::kInvalidEndpointId, 
                      bool autoFind = true, unsigned int FixEndpoint = 0)
{

    EndpointId endpoint = 0;
    CHIP_ERROR ret;

    if(autoFind == true){
        for(endpoint = 0; endpoint < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; endpoint++){
            if(emberAfEndpointIndexIsEnabled(endpoint) == false && endpoint >= FIXED_ENDPOINT_COUNT){
                break;
            }
        }
    }
    else{

        endpoint = (EndpointId) FixEndpoint;
    }

    if(endpoint == CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT){
        ChipLogProgress(DeviceLayer, "Failed to add dynamic endpoint %d == %d", endpoint, CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT);
        return 0;
    }

    DeviceLayer::StackLock lock;

    ret = emberAfSetDynamicEndpoint((uint16_t)(endpoint - FIXED_ENDPOINT_COUNT), endpoint, ep, dataVersionStorage, deviceTypeList, parentEndpointId);

    if (ret == CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Added dynamic endpoint %d --- index = %d", endpoint, (uint16_t)(endpoint - FIXED_ENDPOINT_COUNT));
        return (int)endpoint;
    }
    ChipLogProgress(DeviceLayer, "Failed to add dynamic endpoint %d", ret.AsInteger());
    return -1;
}

/*
    Remove Endpoint from Bridge
*/
int RemoveDeviceEndpoint(EndpointId endpoint)
{
    if(emberAfEndpointIndexIsEnabled(endpoint) == false){
        ChipLogProgress(DeviceLayer, "Removed dynamic endpoint %d failed --- Endpoint is not enable ", endpoint);
    }
    DeviceLayer::StackLock lock;
    ChipLogProgress(DeviceLayer, "Removed dynamic endpoint %d", endpoint);
    emberAfClearDynamicEndpoint((uint16_t)(endpoint - FIXED_ENDPOINT_COUNT));
    return -1;
}

Protocols::InteractionModel::Status HandleReadBridgedDeviceBasicAttribute(chip::AttributeId attributeId, uint8_t * buffer,  uint16_t maxReadLength, EndpointId endpoint){
    using namespace BridgedDeviceBasicInformation::Attributes;

    ChipLogProgress(DeviceLayer, "HandleReadBridgedDeviceBasicAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    IoTDevice device = manager.loadDeviceByEndpoint(endpoint);

    // std::cout << device.toJson().toStyledString() << std::endl;

    // if(device == IoTDevice()){
    //     return Protocols::InteractionModel::Status::Failure;
    // }

    if ((attributeId == Reachable::Id))
    {
        *buffer = (uint8_t) 1;
    }
    else if ((attributeId == NodeLabel::Id))
    {        
        MutableByteSpan zclNameSpan(buffer, 20);
        MakeZclCharString(zclNameSpan, device.product_name.c_str());      
    }
    else if ((attributeId == VendorName::Id)) {
        MutableByteSpan zclNameSpan(buffer, 20);
        MakeZclCharString(zclNameSpan, device.vendor_name.c_str()); 
    }
    else if ((attributeId == VendorID::Id)) {
        memcpy(buffer , &(device.vendor_id), sizeof(device.vendor_id));
    }
    else if ((attributeId == ProductID::Id)) {
        memcpy(buffer , &(device.product_id), sizeof(device.product_id));
    }
    else if ((attributeId == ProductName::Id)) {
        MutableByteSpan zclNameSpan(buffer, 20);
        MakeZclCharString(zclNameSpan, device.product_name.c_str()); 
    }
    else if ((attributeId == SerialNumber::Id)) {
        MutableByteSpan zclNameSpan(buffer, 20);
        MakeZclCharString(zclNameSpan, device.serial_number.c_str());
    }
    else if((attributeId == SoftwareVersion::Id)){
        memcpy(buffer , &(device.software_version), sizeof(device.software_version));
    }
    else if((attributeId == HardwareVersion::Id)){
        memcpy(buffer , &(device.hardware_version), sizeof(device.hardware_version));
    }
    else if((attributeId == SoftwareVersionString::Id)){
        MutableByteSpan zclNameSpan(buffer, 10);
        MakeZclCharString(zclNameSpan, device.software_version_string.c_str());
    }
    else if((attributeId == HardwareVersionString::Id)){
        MutableByteSpan zclNameSpan(buffer, 10);
        MakeZclCharString(zclNameSpan, device.hardware_version_string.c_str());
    }
    else if ((attributeId == ClusterRevision::Id))
    {
        *buffer = (uint16_t) ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_CLUSTER_REVISION;
    }
    else if ((attributeId == FeatureMap::Id))
    {
        *buffer = (uint32_t) ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_FEATURE_MAP;
    }
    else
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    return Protocols::InteractionModel::Status::Success;
}


Protocols::InteractionModel::Status HandleReadBindingAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{

    if ((attributeId == Binding::Attributes::ClusterRevision::Id) && (maxReadLength == 2))
    {
        *buffer = (uint16_t)ZCL_BINDING_CLUSTER_REVISION;
      
    }  
    else if ((attributeId == Binding::Attributes::FeatureMap::Id))
    {
        uint16_t featureMap = 0;
        memcpy(buffer, &featureMap, sizeof(featureMap));
    }
    else
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status HandleReadIdentifyAttribute(chip::AttributeId attributeId, uint8_t * buffer,
                                                 uint16_t maxReadLength)
{
    // printf("HandleReadIdentifyAttribute %d\n", attributeId);
    if ((attributeId == Identify::Attributes::ClusterRevision::Id))
    {
       *buffer = (uint16_t)ZCL_IDENTIFY_CLUSTER_REVISION;
    }  
    else if ((attributeId == Identify::Attributes::FeatureMap::Id))
    {
        *buffer = 0;
    }
    else if ((attributeId == Identify::Attributes::IdentifyTime::Id)){
        *buffer = 0;
    }
    else
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    return Protocols::InteractionModel::Status::Success;
}


Protocols::InteractionModel::Status emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                   const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                                   uint16_t maxReadLength)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    ChipLogProgress(DeviceLayer, "emberAfExternalAttributeReadCallback: ep=%d -- clusterId = %d --- endpointIndex=%d", endpoint, clusterId, endpointIndex);

    Protocols::InteractionModel::Status ret = Protocols::InteractionModel::Status::Failure;

    if ((endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT))
    {

        if (clusterId == BridgedDeviceBasicInformation::Id)
        {
            ret = HandleReadBridgedDeviceBasicAttribute(attributeMetadata->attributeId, buffer, maxReadLength, endpoint);
        }
        else if (clusterId == OnOff::Id)
        {
            ret = HandleReadOnOffAttribute(attributeMetadata->attributeId, buffer, maxReadLength, endpoint);
        }
        else if (clusterId == Binding::Id)
        {
            ret  = HandleReadBindingAttribute(attributeMetadata->attributeId, buffer, maxReadLength);
        }
        else if(clusterId == LevelControl::Id){
            ret  = HandleReadLevelControlAttribute(attributeMetadata->attributeId, buffer, maxReadLength, endpoint);
        }
        else if(clusterId == ColorControl::Id){            
            ret = HandleReadColorControlAttribute(attributeMetadata->attributeId, buffer, maxReadLength, endpoint);
        }
        else if(clusterId == Identify::Id){
            ret = HandleReadIdentifyAttribute(attributeMetadata->attributeId, buffer, maxReadLength);
        }
        else if(clusterId == Switch::Id){
            ret = HandleReadSwitchAttribute(attributeMetadata->attributeId, buffer, maxReadLength, endpoint);
        }
        else if(clusterId == BooleanState::Id){
            ret = HandleReadBooleanStateAttribute(attributeMetadata->attributeId, buffer, maxReadLength, endpoint);
        }
        else if(clusterId == TemperatureMeasurement::Id){
            ret = HandleReadTemperatureMeasurementAttribute(attributeMetadata->attributeId, buffer, maxReadLength, endpoint);
        }
        else if(clusterId == RelativeHumidityMeasurement::Id){
            ret = HandleReadRelativeHumidityMeasurementAttribute(attributeMetadata->attributeId, buffer, maxReadLength, endpoint);
        }
        else if(clusterId == IlluminanceMeasurement::Id){
            ret = HandleReadIlluminanceMeasurementAttribute(attributeMetadata->attributeId, buffer, maxReadLength, endpoint);
        }

    }

    return ret;
}

Protocols::InteractionModel::Status emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                    const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    Protocols::InteractionModel::Status ret = Protocols::InteractionModel::Status::Failure;

    ChipLogProgress(DeviceLayer, "emberAfExternalAttributeWriteCallback: ep=%d", endpoint);

    if (endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (clusterId == OnOff::Id)
        {
            ret = HandleWriteOnOffAttribute(attributeMetadata->attributeId, buffer, endpoint);
        }
        else if(clusterId == ColorControl::Id){
            ret = HandleWriteColorControlAttribute(attributeMetadata->attributeId, buffer, endpoint);
        }
        else if(clusterId == LevelControl::Id){
            ret = HandleWriteLevelControlAttribute( attributeMetadata->attributeId, buffer, endpoint);
        }
    }

    return ret;
}


const EmberAfDeviceType gBridgedDimmerSwitchDeviceTypes[] = {{ DEVICE_TYPE_DIMMER_SWITCH, DEVICE_VERSION_DEFAULT},
                                                              { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT }};

const EmberAfDeviceType gBridgedContactDeviceTypes[] = {{ DEVICE_TYPE_CONTACT_SENSOR, DEVICE_VERSION_DEFAULT},
                                                         { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT }};

const EmberAfDeviceType gBridgedLightSensorDeviceTypes[] = {{ DEVICE_TYPE_LIGHT_SENSOR, DEVICE_VERSION_DEFAULT},
                                                             { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT }};

const EmberAfDeviceType gBridgedTemperatureSensorDeviceTypes[] = {{ DEVICE_TYPE_TEMPERATURE_SENSOR, DEVICE_VERSION_DEFAULT},
                                                                   { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT }};
                                                                   
const EmberAfDeviceType gBridgedHumiditySensorDeviceTypes[] = {{ DEVICE_TYPE_HUMIDITY_SENSOR, DEVICE_VERSION_DEFAULT},
                                                                { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT }};

const EmberAfDeviceType gBridgedOnOffLightDeviceTypes[] = {{ DEVICE_TYPE_LO_ON_OFF_LIGHT, DEVICE_VERSION_DEFAULT},
                                                            { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT }};

const EmberAfDeviceType gBridgedDimLightDeviceTypes[] = {{ DEVICE_TYPE_DIM_LIGHT, DEVICE_VERSION_DEFAULT},
                                                          { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT }};

const EmberAfDeviceType gBridgedColorTemperatureLightDeviceTypes[] = {{ DEVICE_TYPE_COLOR_TEMPERATURE_LIGHT, DEVICE_VERSION_DEFAULT},
                                                                       { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT }};

const EmberAfDeviceType gBridgedExtendedLightDeviceTypes[] = {{ DEVICE_TYPE_EXTENAL_LIGHT, DEVICE_VERSION_DEFAULT},
                                                               { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT }};

const EmberAfDeviceType gBridgedOnOffSwitchDeviceTypes[] = {{ DEVICE_TYPE_LO_SWITCH_LIGHT, DEVICE_VERSION_DEFAULT},
                                                             { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT }};


void HandleAddDeviceEndpoint(unsigned int endpoint, unsigned int type_id){
    
    // std::cout << "HandleAddDeviceEndpoint: " << endpoint << " " << type_id << std::endl;

    if (type_id == DEVICE_TYPE_LO_ON_OFF_LIGHT)
    {
        /* code */
    }
    else if (type_id == DEVICE_TYPE_DIM_LIGHT)
    {
        /* code */
    }
    else if (type_id == DEVICE_TYPE_COLOR_TEMPERATURE_LIGHT)
    {
        AddDeviceEndpoint(
            &bridgedTempLightEndpoint, 
            Span<const EmberAfDeviceType>(gBridgedColorTemperatureLightDeviceTypes), 
            Span<DataVersion>(gTempLightDataVersions), 
            1, 
            false, 
            endpoint
        );
    }
    else if (type_id == DEVICE_TYPE_LO_SWITCH_LIGHT)
    {
        AddDeviceEndpoint(
            &bridgedOnOffSwitchEndpoint, 
            Span<const EmberAfDeviceType>(gBridgedOnOffSwitchDeviceTypes),
            Span<DataVersion>(gOnOffSwitchDataVersions), 
            1, 
            false, 
            endpoint
        );
    }
    else if (type_id == DEVICE_TYPE_DIMMER_SWITCH)
    {
        /* code */
    }
    else if (type_id == DEVICE_TYPE_COLOR_DIMMER_SWITCH)
    {
        /* code */
    }
    else if (type_id == DEVICE_TYPE_EXTENAL_LIGHT)
    {
        AddDeviceEndpoint(
            &bridgedExtendedLightEndpoint, 
            Span<const EmberAfDeviceType>(gBridgedExtendedLightDeviceTypes), 
            Span<DataVersion>(gExtendedLightDataVersions), 
            1, 
            false, 
            endpoint
        );
        /* code */
    }
    else if (type_id == DEVICE_TYPE_TEMPERATURE_SENSOR)
    {
        AddDeviceEndpoint(
            &bridgedTemperatureEndpoint, 
            Span<const EmberAfDeviceType>(gBridgedTemperatureSensorDeviceTypes),
            Span<DataVersion>(gTemperatureDataVersions), 
            1, 
            false, 
            endpoint
        );
    }
    else if (type_id == DEVICE_TYPE_HUMIDITY_SENSOR)
    {
        /* code */
        AddDeviceEndpoint(
            &bridgedHumidityEndpoint, 
            Span<const EmberAfDeviceType>(gBridgedHumiditySensorDeviceTypes),
            Span<DataVersion>(gHumidityDataVersions), 
            1, 
            false, 
            endpoint
        );
    }
    else if (type_id == DEVICE_TYPE_CONTACT_SENSOR)
    {
        /* code */
        AddDeviceEndpoint(
            &bridgedContactEndpoint, 
            Span<const EmberAfDeviceType>(gBridgedContactDeviceTypes),
            Span<DataVersion>(gContactDataVersions), 
            1, 
            false, 
            endpoint
        );
    }
    else if (type_id == DEVICE_TYPE_LIGHT_SENSOR)
    {
        AddDeviceEndpoint(
            &bridgedLightSensorEndpoint, 
            Span<const EmberAfDeviceType>(gBridgedLightSensorDeviceTypes),
            Span<DataVersion>(gLightSensorDataVersions), 
            1, 
            false, 
            endpoint
        );
    }
}

void signalHandler(int signum)
{
    // std::cout << "[Main] Received signal: " << signum << std::endl;
    std::string deviceID = manager.getNewId();
    manager.setNewId("");
    if (deviceID != "")
    {
        IoTDevice device = manager.loadDeviceById(deviceID);
        std::cout << device.toJson().toStyledString() << std::endl;
        std::cout << device.endpoint << " " << device.type_id << std::endl;
        HandleAddDeviceEndpoint(device.endpoint, device.type_id);
    }
}

int main(int argc, char * argv[])
{
    manager.printAllDevices();
    mqttClient.loop_start();
    HandleSensorInit(manager);
    HandleLightInit(manager, mqttClient);
    signal(SIGUSR1, signalHandler);

    /*--------------------------------------------------------------------- */

    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }
    // Init Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

    initParams.interfaceId = LinuxDeviceOptions::GetInstance().interfaceId;
    chip::Server::GetInstance().Init(initParams);
    RunOTARequestor();

    // gExampleDeviceInfoProvider.SetStorageDelegate(&Server::GetInstance().GetPersistentStorage());
    // chip::DeviceLayer::SetDeviceInstanceInfoProvider(&gExampleDeviceInfoProvider);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
    CHIP_ERROR err = LightSwitchMgr::GetInstance().Init(kLightSwitchEndpoint, kGenericSwitchEndpoint);


    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "LightSwitchMgr Init failed!");
    }

    emberAfEndpointEnableDisable(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1)), false);

    Json::Value devices = manager.getDevices();

    for (const auto &device : devices) {
        HandleAddDeviceEndpoint(device["endpoint"].asUInt(), device["type_id"].asUInt());
    }
  
    sEthernetNetworkCommissioningInstance.Init();
    InitOTARequestor();
    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    return 0;
}
