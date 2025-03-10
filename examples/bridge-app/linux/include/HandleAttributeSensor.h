#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Events.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include "IoTDeviceManager.h"
#include <app/ConcreteAttributePath.h>
#include <app/EventLogging.h>

using namespace chip;
using namespace chip::app::Clusters;

void HandleSensorInit(IoTDeviceManager &manager);

Protocols::InteractionModel::Status HandleReadBooleanStateAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint);
Protocols::InteractionModel::Status HandleReadTemperatureMeasurementAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint);
Protocols::InteractionModel::Status HandleReadRelativeHumidityMeasurementAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint);
Protocols::InteractionModel::Status HandleReadIlluminanceMeasurementAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength, EndpointId endpoint);
