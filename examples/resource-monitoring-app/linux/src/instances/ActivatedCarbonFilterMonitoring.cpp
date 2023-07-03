#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/config.h>
#include <instances/ActivatedCarbonFilterMonitoring.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::HepaFilterMonitoring;
using chip::Protocols::InteractionModel::Status;

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
using chip::Protocols::InteractionModel::Status;

//-- Hepa filter Monitoring Instance functions
CHIP_ERROR ActivatedCarbonFilterMonitoringInstance::AppInit()
{
    ChipLogDetail(Zcl, "ActivatedCarbonFilterMonitoringDelegate::Init()");
    return CHIP_NO_ERROR;
}

Status ActivatedCarbonFilterMonitoringInstance::OnResetCondition()
{
    ChipLogError(Zcl, "ActivatedCarbonFilterMonitoringDelegate::OnResetCondition()");
    if ( GetDegradationDirection() == DegradationDirectionEnum::kDown)
    {
        UpdateCondition(100);
    }
    else if ( GetDegradationDirection() == DegradationDirectionEnum::kUp)
    {
        UpdateCondition(0);
    }

    return Status::Success;
}
