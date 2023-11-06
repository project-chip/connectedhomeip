#include <air-quality-instance.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AirQuality;

Instance * gAirQualityCluster = nullptr;

Instance * AirQuality::GetInstance()
{
    return gAirQualityCluster;
}

void AirQuality::Shutdown()
{
    if (gAirQualityCluster != nullptr)
    {
        delete gAirQualityCluster;
        gAirQualityCluster = nullptr;
    }
}

void emberAfAirQualityClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gAirQualityCluster == nullptr);
    chip::BitMask<Feature, uint32_t> airQualityFeatures(Feature::kModerate, Feature::kFair, Feature::kVeryPoor,
                                                        Feature::kExtremelyPoor);
    gAirQualityCluster = new Instance(1, airQualityFeatures);
    gAirQualityCluster->Init();
}
