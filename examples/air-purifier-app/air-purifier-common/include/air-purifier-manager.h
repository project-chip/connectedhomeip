#include <air-quality-sensor-manager.h>
#include <filter-instances.h>

class AirPurifierManager
{
private:
    static const EndpointId mEndpointId = 1;
    HepaFilterMonitoringInstance mHepaFilterMonitoringInstance;
    ActivatedCarbonFilterMonitoringInstance mActivatedCarbonFilterMonitoringInstance;
    AirQualitySensorManager mAirQualitySensorManager;
};