#include "HandleAttributeLight.h"

#define ZCL_DIM_CLUSTER_REVISION (5u)

static IoTDeviceManager *deviceManager = nullptr;
static MQTTHandler *mqtt = nullptr;

uint8_t dim = 20;

void HandleLightInit(IoTDeviceManager &manager, MQTTHandler &mqttHandler){
    deviceManager = &manager;
    mqtt = &mqttHandler;
}

Protocols::InteractionModel::Status HandleReadSwitchAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint){
    // ChipLogProgress(DeviceLayer, "HandleReadOnOffAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);
    
    if (attributeId == Switch::Attributes::CurrentPosition::Id && maxReadLength == 1){
        if(deviceManager->getDeviceStateByEndpoint(endpoint)["onoff"].isNull()){
            *buffer = 0;
        }
        else
            *buffer = (uint8_t)deviceManager->getDeviceStateByEndpoint(endpoint)["onoff"].asUInt();
    }
    else if(attributeId == Switch::Attributes::NumberOfPositions::Id && maxReadLength == 1){
        *buffer = 2;
    }
    else if (attributeId == OnOff::Attributes::ClusterRevision::Id)
    {
        // *buffer = (uint16_t) ZCL_SWITCH_CLUSTER_REVISION;
    }
    else {
        return Protocols::InteractionModel::Status::Failure;
    }

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status HandleReadOnOffAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint)
{
    ChipLogProgress(DeviceLayer, "HandleReadOnOffAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    if ( attributeId == OnOff::Attributes::OnOff::Id && maxReadLength == 1)
    {
        // std::cout << deviceManager->getDeviceStateByEndpoint(endpoint) << std::endl;
        if(deviceManager->getDeviceStateByEndpoint(endpoint)["onoff"].isNull()){
            *buffer = 0;
        }
        else
            *buffer = (uint8_t)deviceManager->getDeviceStateByEndpoint(endpoint)["onoff"].asInt();
        std::cout << "Read onoff: " << (int)*buffer << std::endl;
    }
    else if (attributeId == OnOff::Attributes::ClusterRevision::Id)
    {
        // *buffer = (uint16_t) ZCL_ON_OFF_CLUSTER_REVISION;
    }
    else if (attributeId == OnOff::Attributes::FeatureMap::Id){
        // *buffer = ZCL_ON_OFF_CLUSTER_FEATURE_MAP;
    }
    else if (attributeId == OnOff::Attributes::GeneratedCommandList::Id){

    }
    else if (attributeId == OnOff::Attributes::AcceptedCommandList::Id){

    }
    else if (attributeId == OnOff::Attributes::AttributeList::Id){
        
    }
    else if (attributeId == OnOff::Attributes::EventList::Id){
        
    }
    else
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status HandleWriteOnOffAttribute(chip::AttributeId attributeId, uint8_t * buffer, EndpointId endpoint)
{
    ChipLogProgress(DeviceLayer, "HandleWriteOnOffAttribute: attrId=%d --- buffer=%d", attributeId, * buffer);

    if (attributeId == OnOff::Attributes::OnOff::Id)
    {
        uint8_t onoff = *buffer;
        Json::Value root;
        root["onoff"] = Json::Value(onoff);
        deviceManager->updateDeviceStateByEndpoint(endpoint, root);
        IoTDevice device = deviceManager->loadDeviceByEndpoint(endpoint);
        mqtt->sendControl(device.id, &onoff, nullptr, nullptr, nullptr, nullptr, nullptr);
    }
    else
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status HandleReadColorControlAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint)
{
    // ChipLogProgress(DeviceLayer, "HandleReadColorControlAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    if(attributeId == ColorControl::Attributes::ColorTemperatureMireds::Id && maxReadLength == 1){
        if(deviceManager->getDeviceStateByEndpoint(endpoint)["ctt"].isNull()){
            *buffer = 0;
        }
        else
            *buffer = (uint8_t)deviceManager->getDeviceStateByEndpoint(endpoint)["ctt"].asUInt();

    }
    else if(attributeId == ColorControl::Attributes::ColorTempPhysicalMinMireds::Id){
        *buffer = 0;
    }
    else if(attributeId == ColorControl::Attributes::ColorTempPhysicalMaxMireds::Id){
        *buffer = 255;
    }
    else if(attributeId == ColorControl::Attributes::CurrentHue::Id){
        *buffer = (uint8_t)deviceManager->getDeviceStateByEndpoint(endpoint)["h"].asUInt();
    }
    else if(attributeId == ColorControl::Attributes::CurrentSaturation::Id ){
        *buffer = (uint8_t)deviceManager->getDeviceStateByEndpoint(endpoint)["s"].asUInt();
    }
    else if(attributeId == ColorControl::Attributes::ClusterRevision::Id){
        // *buffer = (uint16_t) ZCL_COLOR_CLUSTER_REVISION;
    }
    else if (attributeId == ColorControl::Attributes::GeneratedCommandList::Id){

    }
    else if (attributeId == ColorControl::Attributes::AcceptedCommandList::Id){

    }
    else if (attributeId == ColorControl::Attributes::AttributeList::Id){
        
    }
    else if (attributeId == ColorControl::Attributes::EventList::Id){
        
    }
    else if ((attributeId == ColorControl::Attributes::FeatureMap::Id))
    {
        *buffer = 0;
    }
    else
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status HandleWriteColorControlAttribute(chip::AttributeId attributeId, uint8_t * buffer,EndpointId endpoint)
{
    // ChipLogProgress(DeviceLayer, "HandleWriteControlAttribute: attrId=%d --- buffer=%d", attributeId, * buffer);

    IoTDevice device = deviceManager->loadDeviceByEndpoint(endpoint);
    if (attributeId == ColorControl::Attributes::RemainingTime::Id){

    }
    if (attributeId == ColorControl::Attributes::ColorTemperatureMireds::Id){
        Json::Value root;
        root["ctt"] = *buffer;
        deviceManager->updateDeviceStateByEndpoint(endpoint, root);
        mqtt->sendControl(device.id, nullptr, nullptr, buffer, nullptr, nullptr, nullptr);
    }
    if (attributeId == ColorControl::Attributes::CurrentHue::Id){
        Json::Value root;
        root["h"] = *buffer;
        deviceManager->updateDeviceStateByEndpoint(endpoint, root);
        mqtt->sendControl(device.id, nullptr, nullptr, nullptr, buffer, nullptr, nullptr);
    }
    if (attributeId == ColorControl::Attributes::CurrentSaturation::Id){
        Json::Value root;
        root["s"] = *buffer;
        deviceManager->updateDeviceStateByEndpoint(endpoint, root);
        mqtt->sendControl(device.id, nullptr, nullptr, nullptr, nullptr, nullptr, buffer);
    }
    else
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status HandleReadLevelControlAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint){
    
    ChipLogProgress(DeviceLayer, "HandleReadLevelControlAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    if( attributeId ==LevelControl::Attributes::CurrentLevel::Id && maxReadLength == 1){
        
        if(deviceManager->getDeviceStateByEndpoint(endpoint)["dim"].isNull()){
            *buffer = 0;
        }
        else
            *buffer = (uint8_t)(deviceManager->getDeviceStateByEndpoint(endpoint)["dim"].asUInt());
        
        std::cout << "Read dim: " << *buffer << std::endl;
    }
    else if(attributeId == LevelControl::Attributes::Options::Id && maxReadLength == 1){
        *buffer = 0x01;
    }
    else if(attributeId == LevelControl::Attributes::MinLevel::Id && maxReadLength == 1){
        *buffer = 0;
    }
    else if(attributeId == LevelControl::Attributes::MaxLevel::Id && maxReadLength == 1){
        *buffer = 100;
    }
    else if(attributeId == LevelControl::Attributes::ClusterRevision::Id){
        *buffer = (uint16_t) ZCL_DIM_CLUSTER_REVISION;
    }
    else if ((attributeId == LevelControl::Attributes::FeatureMap::Id))
    {
        *buffer = 0;
    }
    else
    {
        return Protocols::InteractionModel::Status::Failure;
    }
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status HandleWriteLevelControlAttribute(chip::AttributeId attributeId, uint8_t * buffer, EndpointId endpoint){
    
    ChipLogProgress(DeviceLayer, "HandleWriteLevelControlAttribute: attrId=%d --- buffer=%d", attributeId, * buffer);
    IoTDevice device = deviceManager->loadDeviceByEndpoint(endpoint);
    if(attributeId == LevelControl::Attributes::CurrentLevel::Id){
        Json::Value root;
        root["dim"] = *buffer;
        deviceManager->updateDeviceStateByEndpoint(endpoint, root);
        mqtt->sendControl(device.id, nullptr, buffer, nullptr, nullptr, nullptr, nullptr);
    }
    else{
        return Protocols::InteractionModel::Status::Failure;
    }

    return Protocols::InteractionModel::Status::Success;
}
