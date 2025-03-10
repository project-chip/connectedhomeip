#include "HandleAttributeSensor.h"


static IoTDeviceManager *deviceManager = nullptr;

void HandleSensorInit(IoTDeviceManager &manager){
    deviceManager = &manager;
}

Protocols::InteractionModel::Status HandleReadBooleanStateAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint){
    if( attributeId == BooleanState::Attributes::StateValue::Id && maxReadLength == 1){
        if(deviceManager->getDeviceStateByEndpoint(endpoint)["onoff"].asBool()){
            *buffer = 1;
        }
        else {
            *buffer = 0;
        }
    }
    else {
        return Protocols::InteractionModel::Status::Failure;
    }
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status HandleReadTemperatureMeasurementAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint){
    if( attributeId == TemperatureMeasurement::Attributes::MeasuredValue::Id && maxReadLength == 2){
        if(deviceManager->getDeviceStateByEndpoint(endpoint)["temperature"].isNull()){
            // memcpy(buffer, &temperature, sizeof(int16_t));
            *buffer = 0;
            *(buffer + 1) = 0;
        }
        else{
            int16_t temperature = (int16_t)deviceManager->getDeviceStateByEndpoint(endpoint)["temperature"].asInt();
            memcpy(buffer, &temperature, sizeof(int16_t));
        }
    }
    else {
        return Protocols::InteractionModel::Status::Failure;
    }
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status HandleReadRelativeHumidityMeasurementAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint){
    if( attributeId == RelativeHumidityMeasurement::Attributes::MeasuredValue::Id && maxReadLength == 1){
        if(deviceManager->getDeviceStateByEndpoint(endpoint)["humidity"].isNull()){
            *buffer = 0;
        }
        else
            *buffer = (uint8_t)deviceManager->getDeviceStateByEndpoint(endpoint)["humidity"].asInt();

    }
    else {
        return Protocols::InteractionModel::Status::Failure;
    }
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status HandleReadIlluminanceMeasurementAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint){
    if( attributeId == IlluminanceMeasurement::Attributes::MeasuredValue::Id && maxReadLength == 2){
        if(deviceManager->getDeviceStateByEndpoint(endpoint)["illuminance"].isNull()){
            *buffer = 0;
            *(buffer + 1) = 0;
        }
        else{
            uint16_t illuminance = (uint16_t)deviceManager->getDeviceStateByEndpoint(endpoint)["illuminance"].asInt();
            memcpy(buffer, &illuminance, sizeof(uint16_t));
        }
    }
    else {
        return Protocols::InteractionModel::Status::Failure;
    }
    return Protocols::InteractionModel::Status::Success;
}
