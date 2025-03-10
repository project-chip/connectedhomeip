#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Events.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventLogging.h>
#include "IoTDeviceManager.h"
#include "MQTTHandler.h"

using namespace chip;
using namespace chip::app::Clusters;
void HandleLightInit(IoTDeviceManager &manager, MQTTHandler &mqttHandler);

Protocols::InteractionModel::Status HandleReadOnOffAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint);
Protocols::InteractionModel::Status HandleReadLevelControlAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint);
Protocols::InteractionModel::Status HandleReadSwitchAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint);
Protocols::InteractionModel::Status HandleReadColorControlAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint);

Protocols::InteractionModel::Status HandleWriteOnOffAttribute(chip::AttributeId attributeId, uint8_t * buffer, EndpointId endpoint);
Protocols::InteractionModel::Status HandleWriteLevelControlAttribute(chip::AttributeId attributeId, uint8_t * buffer, EndpointId endpoint);
Protocols::InteractionModel::Status HandleWriteColorControlAttribute(chip::AttributeId attributeId, uint8_t * buffer, EndpointId endpoint);